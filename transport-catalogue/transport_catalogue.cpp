#include "transport_catalogue.h"

#include <algorithm>
#include <unordered_set>

#include <iostream>

using namespace std;

namespace catalogue {
    void TransportCatalogue::AddStop(const string &name, const geo::Coordinates& coordinates) {
        stops_.push_back(move(Stop(name, move(coordinates))));
        stops_map_[stops_.back().name] = &(stops_.back());
    }

    void TransportCatalogue::AddDistance(const string& from, const string& to, int distance) {
        Stop* stop_from = stops_map_.at(from);
        Stop* stop_to = stops_map_.at(to);
        stops_to_distance_[make_pair(stop_from, stop_to)] = distance;
    }

    void TransportCatalogue::AddBus(const string& name, const std::vector<string>& stops_name, bool is_roundtrip) {
        vector<Stop*> stops;
        for (const auto &stop_name : stops_name) {
            stops.push_back(stops_map_.at(stop_name));
        }
        buses_.push_back(move(Bus(name, move(stops), is_roundtrip)));
        buses_map_[buses_.back().name] = &(buses_.back());

        for (const auto& stop : buses_.back().stops) {
            stop_to_buses_[stop->name].insert(buses_.back().name);
        }
    }
    BusInfo TransportCatalogue::GetBusInfo(string_view bus_name) const {
        BusInfo info;
        info.bus = bus_name;
        info.stops = ComputeStopCount(bus_name);
        info.unique_stops = ComputeUniqueStopCount(bus_name);
        info.road_info = ComputeRoadInfo(bus_name);
        return info;
    }

    StopInfo TransportCatalogue::GetStopInfo(string_view stop_name) const {
        StopInfo info;
        if (stops_map_.count(stop_name) == 0) {
            info.stop = stop_name;
            info.stop_exist = false;
            return info;
        }

        if (stop_to_buses_.count(stop_name) == 0) {
            info.stop = stop_name;
            info.stop_exist = true;
            info.buses_exist = false;
            return info;
    }

        info.stop = stop_name;
        info.buses = &stop_to_buses_.at(stop_name);
        info.stop_exist = true;
        info.buses_exist = true;
        return info;
    }

    int TransportCatalogue::ComputeStopCount(string_view bus_name) const {
        if (buses_map_.count(bus_name) == 0) {
            return 0;
        };

        int result = buses_map_.at(bus_name)->stops.size();
        if (!buses_map_.at(bus_name)->is_roundtrip) {
            result = 2.0 * result - 1;
        }
        return result;
    }

    int TransportCatalogue::ComputeUniqueStopCount(string_view bus_name) const {
        if (buses_map_.count(bus_name) == 0) {
            return 0;
        }

        unordered_set<string> unique_stops;
        for (auto stop_ptr : buses_map_.at(bus_name)->stops) {
            unique_stops.insert(stop_ptr->name);
        }
        return unique_stops.size();
    }

    RoadInfo TransportCatalogue::ComputeRoadInfo(string_view bus_name) const {
        if (buses_map_.count(bus_name) == 0) {
            return {0.0, 0};
        }

        double distance = 0.0;
        int length = 0;
        int n_stops = buses_map_.at(bus_name)->stops.size();
        for (int i = 0; i <= n_stops-2; ++i) {
            distance += ComputeDistance(
                    buses_map_.at(bus_name)->stops[i]->coordinates,
                    buses_map_.at(bus_name)->stops[i+1]->coordinates
            );
            length += GetRouteLength(
                    buses_map_.at(bus_name)->stops[i],
                    buses_map_.at(bus_name)->stops[i+1]
            );
            if (!buses_map_.at(bus_name)->is_roundtrip) {
                length += GetRouteLength(
                        buses_map_.at(bus_name)->stops[i+1],
                        buses_map_.at(bus_name)->stops[i]
                );
            }
        }

        if (!buses_map_.at(bus_name)->is_roundtrip) {
            distance *= 2.0;
        }
        return {distance, length};
    }

    int TransportCatalogue::GetRouteLength(const Stop* from, const Stop* to) const {
        pair<Stop*, Stop*> key = make_pair(const_cast<Stop*>(from), const_cast<Stop*>(to));
        if (stops_to_distance_.count(key) > 0) {
            return stops_to_distance_.at(key);
        }

        key = make_pair(const_cast<Stop*>(to), const_cast<Stop*>(from));
        if (stops_to_distance_.count(key) > 0) {
            return stops_to_distance_.at(key);
        }

        return 0;
    }

    const std::unordered_map<std::string_view, Bus*> TransportCatalogue::GetBusesMap() const {
        return buses_map_;
    }

    const std::unordered_map<std::string_view, Stop*> TransportCatalogue::GetStopsMap() const {
        return stops_map_;
    }

} // namespace catalogue
