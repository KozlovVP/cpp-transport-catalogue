#pragma once

#include <iterator>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "geo.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace catalogue {
    using namespace graph;
    using namespace catalogue;

    struct RoutingSettings {
        double bus_velocity;
        int bus_wait_time;
    };

    struct BusRouteInfo {
        std::string_view bus_name;
        int span_count;
        double time;
    };

    struct WaitRouteInfo {
        std::string_view stop_name;
        int time;
    };

    class TransportRouter {
    public:
        TransportRouter(const TransportCatalogue& catalogue, RoutingSettings& routing_settings);
        std::optional<std::pair<double, std::vector<std::variant<WaitRouteInfo, BusRouteInfo>>>>
                FindRoute(std::string_view from, std::string_view to);

    private:
        const RoutingSettings& routing_settings_;
        const TransportCatalogue& catalogue_;
        std::unique_ptr<DirectedWeightedGraph<double>> graph_;
        std::unique_ptr<Router<double>> router_;
        std::unordered_map<std::string_view, VertexId> stops_vertex_;
        std::unordered_map<EdgeId, std::variant<WaitRouteInfo, BusRouteInfo>> edge_info_;

        void InitGraphVertex(const TransportCatalogue& catalogue);
        void SetWaitEdge();

        template <typename It>
        void SetStopsEdge(const std::string_view bus_name, It begin, It end) {
            for (auto it_ext = begin; it_ext != end - 1; ++it_ext) {
                double route_length = 0;
                for (auto it_int = it_ext+1; it_int != end; ++it_int) {
                    route_length += catalogue_.GetRouteLength(*(it_int-1), *it_int);
                    double weight = route_length / (routing_settings_.bus_velocity * 1000 / 60);
                    VertexId from = stops_vertex_[(*it_ext)->name] + 1; // пассажир садится в подъехавший автобус в вершине "остановка-автобус"
                    VertexId to = stops_vertex_[(*it_int)->name]; // пассажир, выходя из автобуса должен попасть в вершину "остановка-ожидание"
                    Edge<double> edge {from, to, weight};
                    EdgeId edge_id = graph_->AddEdge(edge);
                    int span_count = std::distance(it_ext, it_int);
                    edge_info_[edge_id] = BusRouteInfo{bus_name, span_count, weight};
                }
            }
        }
    };

} // namespace catalogue
