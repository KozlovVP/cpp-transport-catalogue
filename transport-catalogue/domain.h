#pragma once

#include "geo.h"

#include <set>
#include <string>
#include <vector>

namespace catalogue {
    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
        Stop(const std::string &name, const geo::Coordinates &coordinates) : name(name), coordinates(coordinates) {};
    };
    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        bool is_roundtrip;
        Bus(const std::string &name, const std::vector<Stop*> &stops, bool is_roundtrip) :
                name(name), stops(stops), is_roundtrip(is_roundtrip) {};
    };

    struct RoadInfo {
        RoadInfo() = default;

        RoadInfo(double distance, int length)
            : distance(distance)
            , length(length) {
        }

        double distance = 0.0;
        int length = 0;
    };

    struct BusInfo {
        BusInfo() = default;

        BusInfo(const std::string &bus, int stops, int unique_stops, const RoadInfo &road_info)
            : bus(bus)
            , stops(stops)
            , unique_stops(unique_stops)
            , road_info(road_info) {
        }

        std::string bus;
        int stops = 0;
        int unique_stops = 0;
        RoadInfo road_info = {0.0, 0};
    };

    struct StopInfo {
        StopInfo() = default;

        StopInfo(const std::string &stop, const std::set<std::string_view>* &buses, bool stop_exist, bool buses_exist)
            : stop(stop)
            , buses(buses)
            , stop_exist(stop_exist)
            , buses_exist(buses_exist) {
        }

        std::string stop;
        const std::set<std::string_view>* buses = nullptr;
        bool stop_exist = false;
        bool buses_exist = false;
    };
}

