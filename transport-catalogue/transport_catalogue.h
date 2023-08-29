#pragma once

#include "domain.h"

#include <deque>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

namespace catalogue {
    class TransportCatalogue {
    public:
        TransportCatalogue() {};

        void AddStop(const std::string &name, const geo::Coordinates& coordinates);
        void AddDistance(const std::string& from, const std::string& to, int distance);
        void AddBus(const std::string& bus_name, const std::vector<std::string>& stops_name, bool is_roundtrip);
        BusInfo GetBusInfo(std::string_view bus_name) const;
        StopInfo GetStopInfo(std::string_view stop_name) const;
        const std::unordered_map<std::string_view, Bus*> GetBusesMap() const;
        const std::unordered_map<std::string_view, Stop*> GetStopsMap() const;
        int GetRouteLength(const Stop* from, const Stop* to) const;
    private:
        class DistanceHasher {
        public:
            size_t operator()(std::pair<Stop*, Stop*> stops_pair) const {
                size_t first = hasher(stops_pair.first);
                size_t second = hasher(stops_pair.second);
                return first + 37 * second;
            }
            std::hash<const void*> hasher;
        };

        int ComputeStopCount(std::string_view bus_name) const;
        int ComputeUniqueStopCount(std::string_view bus_name) const;
        RoadInfo ComputeRoadInfo(std::string_view bus_name) const;
        //int GetRouteLength(const Stop* from, const Stop* to) const;

        std::unordered_map<std::string_view, Bus*> buses_map_;
        std::unordered_map<std::string_view, Stop*> stops_map_;
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, DistanceHasher> stops_to_distance_;
    };
} // namespace catalogue
