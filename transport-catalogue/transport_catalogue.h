#pragma once

// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include "geo.h"
#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <string_view>
#include <set>
#include <utility>

#include "stat_reader.h"


struct Stop {
    std::string name;
    Coordinates coor;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};

class TransportCatalogue {
public:
    void AddStop(std::string& name, double lt, double lg);

    void AddBus(std::string& name, std::vector<std::string>& stops);

    void GetBusInfo(std::string& name);

    void GetStopInfo(std::string& name);

    void AddDistance(std::string name, double distance, std::string name2);

private:
    struct StopNameToStopHasher
    {
        size_t operator()(const std::string& name) const {
            std::hash<std::string> hasher;
            return hasher(name);
        }
    };

    struct BusNameToRouteHasher
    {
        size_t operator()(const std::string name) const {
            std::hash<std::string> hasher;
            return hasher(name);
        }
    };

    struct DistanceHasher
    {
        size_t operator()(const std::pair<Stop*, Stop*> stops) const {
            std::hash<std::string> hasher;
            return hasher(stops.first->name) * 37 + hasher(stops.second->name);
        }
    };

    std::deque<Stop> stops_; // все остановки
    std::unordered_map<std::string, Stop*, StopNameToStopHasher> stopname_to_stop_; // поиск остановки по имени

    std::deque<Bus> buses_; // все маршруты
    std::unordered_map<std::string, Bus*, BusNameToRouteHasher> busname_to_route_; // поиск маршрута по имени

    std::unordered_map<std::string, std::set<std::string>, StopNameToStopHasher> buses_to_stop_; // автобусы через Stop

    std::unordered_map<std::pair<Stop*, Stop*>, double, DistanceHasher> stop_distance_; //дистанция между остановками
};
