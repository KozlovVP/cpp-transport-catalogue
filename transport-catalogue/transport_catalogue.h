#pragma once
#include <deque>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

#include "geo.h"

namespace transport_catalogue {

    struct Stop {
        std::string name_;
        geo::Coordinates coordinates_;

        bool operator==(const Stop& other) const {
            return name_ == other.name_ && coordinates_ == other.coordinates_;
        }
    };

    struct Bus {
        std::string name_;
        std::vector<const Stop *> stops_;
        bool circular_route{};

        bool operator==(const Bus& other) const {
            return name_ == other.name_ && stops_ == other.stops_;
        }
        [[maybe_unused]]  bool CircularRoute() const {
            Bus bus;
            if (stops_.size() < 2) {
                return false;
            }
            const Stop *first_stop = stops_.front();
            const Stop *last_stop = stops_.back();
            return first_stop == last_stop;
        }
    };

    struct BusInfo {
        size_t stops_on_route;
        size_t unique_stops;
        double route_length;
        double curvature;
    };

    struct PairHash {
        size_t operator()(const std::pair<const Stop*, const Stop*>& key) const {
            size_t h1 = std::hash<const Stop*>()(key.first);
            size_t h2 = std::hash<const Stop*>()(key.second);
            return h1 ^ (h2 << 1);
        }
    };

    struct StopHash {
        size_t operator()(const Stop& stop) const {
            return std::hash<std::string>()(stop.name_);
        }
    };

    struct BusHash {
        std::size_t operator()(const Bus& bus) const {
            std::size_t result = std::hash<std::string>{}(bus.name_);
            for (const auto* stop : bus.stops_) {
                result ^= std::hash<std::string>{}(stop->name_);
            }
            return result;
        }
    };

    using StopMap = std::unordered_map<std::string_view, Stop *>;
    using StopToBusesMap = std::unordered_map<const Stop*, std::vector<const Bus*>>;
    using BusMap = std::unordered_map<std::string_view, Bus *>;
    using DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairHash>;

    class TransportCatalogue {
    public:
        void AddBus(const Bus& bus);
        void AddStop(const Stop& stop);
        void SetDistance(const Stop& stop_from, const Stop& stop_to, int distance);

        BusInfo GetBusInfo(const Bus& bus) const;

        const Bus *GetBus(std::string_view _bus_name);
        const Stop *GetStop(std::string_view _stop_name);

        BusMap ReturnBusMap() const;
        StopToBusesMap ReturnStopToBuses () const;

        std::unordered_set<Bus, BusHash> StopGetUniqueBuses(Stop *stop) const;
        size_t GetDistanceStop(const Stop *_start, const Stop *_finish) const;


    private:
        void AddBusToStop(const Bus& bus, const Stop& stop);
        const std::vector<const Bus*>& GetBusesForStop(const Stop* stop) const;
        static std::unordered_set<Stop, StopHash> GetUniqueStops(const Bus *bus);
        size_t GetDistanceToBus(const Bus *_bus) const;
        double GetLength(const Bus *bus) const;

        std::deque<Bus> buses_;
        std::deque<Stop> stops_;
        DistanceMap distances_;
        StopMap stop_name_;
        BusMap bus_name_;
        StopToBusesMap stop_buses_;
    };
}
