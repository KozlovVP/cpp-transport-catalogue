#include "transport_catalogue.h"
#include <execution>

namespace transport_catalogue {
    void TransportCatalogue::AddStop(const Stop &stop_) {
        stops_.push_back(stop_);
        Stop *stop_buf = &stops_.back();
        stop_name_.insert(StopMap::value_type(stop_buf->name_, stop_buf));
    }

    void TransportCatalogue::AddBus(const Bus &bus) {
        if (bus.stops_.empty()) {
            return;
        }

        buses_.push_back(bus);
        Bus &bus_buf = buses_.back();
        bus_name_.insert(BusMap::value_type(bus_buf.name_, &bus_buf));

        for (const Stop *_stop: bus_buf.stops_) {
            AddBusToStop(bus_buf, *_stop);
        }
    }

    BusInfo TransportCatalogue::GetBusInfo(const Bus& bus) const {
        BusInfo info{};
        info.stops_on_route = bus.stops_.size();
        info.unique_stops = GetUniqueStops(&bus).size();
        info.route_length = GetDistanceToBus(&bus);
        info.curvature = double (info.route_length / GetLength(&bus));
        return info;
    }

    BusMap TransportCatalogue::ReturnBusMap() const {
        return bus_name_;
    }

    StopToBusesMap TransportCatalogue:: ReturnStopToBuses () const {
        return stop_buses_;
    }

    void TransportCatalogue::AddBusToStop(const Bus &bus, const Stop &stop) {
        auto it = std::find(stops_.begin(), stops_.end(), stop);
        if (it != stops_.end()) {
            stop_buses_[&stop].push_back(&bus);
        }
    }

    void TransportCatalogue::SetDistance( const Stop& stop_from, const Stop& stop_to, int distance) {
        distances_[std::make_pair(&stop_from, &stop_to)] = distance;

    }

    const Bus *TransportCatalogue::GetBus(std::string_view _bus_name) {
        if (bus_name_.empty()) {
            return nullptr;
        }
        try {
            return bus_name_.at(_bus_name);
        } catch (const std::out_of_range &e) {
            return nullptr;
        }
    }

    const std::vector<const Bus *> &TransportCatalogue::GetBusesForStop(const Stop *stop) const {
        static const std::vector<const Bus *> empty_result;
        auto it = stop_buses_.find(stop);
        if (it != stop_buses_.end()) {
            return it->second;
        } else {
            return empty_result;
        }
    }

    const Stop *TransportCatalogue::GetStop(std::string_view _stop_name) {
        if (stop_name_.empty()) {
            return nullptr;
        }
        try {
            return stop_name_.at(_stop_name);
        } catch (const std::out_of_range &e) {
            return nullptr;
        }
    }

    std::unordered_set<Stop, StopHash> TransportCatalogue::GetUniqueStops(const Bus *bus) {
        std::unordered_set<Stop, StopHash> unique_stops;

        for (const auto* stop : bus->stops_) {
            unique_stops.insert(*stop);
        }
        return unique_stops;
    }

    std::unordered_set<Bus, BusHash> TransportCatalogue::StopGetUniqueBuses(Stop *stop) const {
        std::unordered_set<Bus, BusHash> unique_buses;

        const auto &buses = GetBusesForStop(stop);
        for (const auto *bus: buses) {
            unique_buses.insert(*bus);
        }
        return unique_buses;
    }

    struct ComputeStopDistance {
        mutable std::unordered_map<std::pair<const Stop *, const Stop *>, double, PairHash> cache;

        double operator()(const Stop *lhs, const Stop *rhs) const {
            auto key = std::minmax(lhs, rhs);
            if (cache.count(key)) {
                return cache[key];
            }
            double distance = geo::ComputeDistance({lhs->coordinates_},
                                                   {rhs->coordinates_});
            cache[key] = distance;
            return distance;
        }
    };

    double TransportCatalogue::GetLength(const Bus *bus) const {
        double length = 0.0;
        ComputeStopDistance compute_distance;
        if (!bus->stops_.empty()) {
            for (auto it = next(bus->stops_.begin()); it != bus->stops_.end(); ++it) {
                if (*prev(it) && *it) {
                    length += compute_distance(*prev(it), *it);
                }
            }
        }
        return length;
    }

    size_t TransportCatalogue::GetDistanceStop(const Stop *_begin, const Stop *_finish) const {
        if (distances_.empty()) {
            return 0;
        }
        try {
            auto dist_pair = std::make_pair(_begin, _finish);
            return distances_.at(dist_pair);
        } catch (const std::out_of_range &e) {
            try {
                auto dist_pair = std::make_pair(_finish, _begin);
                return distances_.at(dist_pair);
            } catch (const std::out_of_range &e) {
                return 0;
            }
        }
    }

    size_t TransportCatalogue::GetDistanceToBus(const Bus *_bus) const {
        size_t distance = 0;
        auto stops_size = _bus->stops_.size() - 1;
        for (int i = 0; i < static_cast<int>(stops_size); i++) {
            distance += GetDistanceStop(_bus->stops_[i], _bus->stops_[i + 1]);
        }
        return distance;
    }
}
