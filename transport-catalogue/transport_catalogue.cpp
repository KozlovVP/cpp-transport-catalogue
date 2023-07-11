#include "transport_catalogue.h"

using namespace std;

void TransportCatalogue::AddStop(std::string& name, double lt, double lg) {
    Stop stop{ name, lt, lg };
    stops_.push_back(move(stop));
    stopname_to_stop_[name] = &stops_.back();
}

void TransportCatalogue::AddBus(std::string& name, std::vector<std::string>& stops) {
    Bus bus;
    bus.name = name;
    vector<Stop*> bus_stops;
    for (const auto& stop : stops) {
        bus_stops.push_back(stopname_to_stop_[stop]);
        buses_to_stop_[stopname_to_stop_[stop]->name].insert(name);
    }
    bus.stops = move(bus_stops);

    buses_.push_back(move(bus));
    busname_to_route_[name] = &buses_.back();
}

void TransportCatalogue::GetBusInfo(std::string& name) {
    if (busname_to_route_.find(name) == busname_to_route_.end()) {
        NotFoundBus(name);
        return;
    }

    unordered_map<string_view, int> unique;
    for (const auto stop : busname_to_route_[name]->stops) {
        unique[stop->name] += 1;
    }

    double ideal_distance = 0;
    auto prev_coor = (*busname_to_route_[name]->stops.begin())->coor;
    for (auto it = busname_to_route_[name]->stops.begin() + 1; it != busname_to_route_[name]->stops.end(); it++) {
        ideal_distance += ComputeDistance(prev_coor, (*it)->coor);
        prev_coor = (*it)->coor;
    }

    set<Stop*> repeated_three_times;

    double real_distance = 0;
    auto prev_stop = *busname_to_route_[name]->stops.begin();
    real_distance += stop_distance_[pair(prev_stop, prev_stop)];
    for (auto it = next(busname_to_route_[name]->stops.begin()); it != busname_to_route_[name]->stops.end(); it++) {
        if (prev_stop->name == (*it)->name) { repeated_three_times.insert(*it); }
        
        if (stop_distance_.count(pair(prev_stop, *it)) == 0) {
            real_distance += stop_distance_[pair(*it, prev_stop)];
        }
        else { real_distance += stop_distance_[pair(prev_stop, *it)]; }
        real_distance += stop_distance_[pair(*it, * it)];
        prev_stop = *it;

    }
    real_distance -= stop_distance_[pair(*busname_to_route_[name]->stops.rbegin(), *busname_to_route_[name]->stops.rbegin())];

   
    for (auto stop : repeated_three_times) {
        real_distance -= 4 * stop_distance_[pair(stop, stop)];
    }

    double temp_curv = real_distance / ideal_distance;

    PrintBusInfo(name, busname_to_route_[name]->stops.size(), unique.size(), real_distance, temp_curv);
}

void TransportCatalogue::GetStopInfo(std::string& name) {
    if (stopname_to_stop_.find(name) == stopname_to_stop_.end()) {
        NotFoundStop(name);
        return;
    }
    PrintStopInfo(name, buses_to_stop_[name]);
}

void TransportCatalogue::AddDistance(string name, double distance, string name2) {
    auto stop_to_stop = pair(stopname_to_stop_[name], stopname_to_stop_[name2]);
    stop_distance_[stop_to_stop] = distance;
}




