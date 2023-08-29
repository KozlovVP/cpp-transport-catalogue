#include "transport_router.h"

namespace catalogue {
    TransportRouter::TransportRouter(const TransportCatalogue& catalogue, RoutingSettings& routing_settings)
            : routing_settings_(routing_settings), catalogue_(catalogue) {
        InitGraphVertex(catalogue);
        SetWaitEdge();

        const auto buses_map = catalogue.GetBusesMap();
        for (const auto& bus_map : buses_map) {
            SetStopsEdge(bus_map.first, bus_map.second->stops.begin(), bus_map.second->stops.end());
            if (!bus_map.second->is_roundtrip) {
                SetStopsEdge(bus_map.first, bus_map.second->stops.rbegin(), bus_map.second->stops.rend());
            }
        }

        router_ = std::make_unique<Router<double>>(*graph_);
    }

    std::optional<std::pair<double, std::vector<std::variant<WaitRouteInfo, BusRouteInfo>>>>
            TransportRouter::FindRoute(std::string_view from, std::string_view to) {
        VertexId from_id = stops_vertex_[from]; // пассажир, начинает путь из вершины "остановка-ожидание"
        VertexId to_id = stops_vertex_[to]; // пассажир, заканчивает путь на вершине "остановка-ожидание"
        auto info = router_->BuildRoute(from_id, to_id);
        if (!info.has_value()) {
            return std::nullopt;
        }

        std::vector<std::variant<WaitRouteInfo, BusRouteInfo>> route_items;
        for (const auto& edge_id : info.value().edges) {
            route_items.push_back(edge_info_[edge_id]);
        }

        std::pair<double, std::vector<std::variant<WaitRouteInfo, BusRouteInfo>>> p = {info.value().weight, route_items};
        std::optional<std::pair<double, std::vector<std::variant<WaitRouteInfo, BusRouteInfo>>>> o_p = p;
        return o_p;
        //return {info.value().weight, route_items};
    }

    void TransportRouter::InitGraphVertex(const TransportCatalogue& catalogue) {
        const auto stops_map = catalogue.GetStopsMap();
        // Четные вершины - вершины "остановка-ожидание", нечетные - "остановка-автобус"
        VertexId id = 0;
        for (const auto& stop_map : stops_map) {
            stops_vertex_[stop_map.first] = id;
            id += 2; // "остановка-автобус"
        }

        graph_ = std::make_unique<DirectedWeightedGraph<double>>(2*stops_map.size());
    }

    void TransportRouter::SetWaitEdge() {
        for (const auto stop_vertex : stops_vertex_) {
            VertexId from = stop_vertex.second;
            VertexId to = stop_vertex.second + 1;
            Edge<double> edge { from, to, static_cast<double>(routing_settings_.bus_wait_time) };
            EdgeId edge_id = graph_->AddEdge(edge);
            edge_info_[edge_id] = WaitRouteInfo{stop_vertex.first, routing_settings_.bus_wait_time};
        }
    }
}
