#include "request_handler.h"

namespace request_handler {

    void RequestHandler::ExecuteQueries(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_requests, RenderSettings& render_settings){
        std::vector<Node> result_request;

        for (const StatRequest& req : stat_requests) {

            if (req.type_ == "Stop") {
                result_request.push_back(transport_catalogue::detail::json::JSONReader::ExecuteMakeNodeStop(req.id, transport_catalogue::detail::json::JSONReader::StopQuery(catalogue, req.name_)));
            } else if (req.type_ == "Bus") {
                result_request.push_back(transport_catalogue::detail::json::JSONReader::ExecuteMakeNodeBus(req.id, transport_catalogue::detail::json::JSONReader::BusQuery(catalogue, req.name_)));
            } else if(req.type_ == "Map") {
                result_request.push_back(transport_catalogue::detail::json::JSONReader::ExecuteMakeNodeMap(req.id, catalogue, render_settings));
            }

        }

        doc_out = Document{Node(result_request)};
    }

    void RequestHandler::ExecuteRenderMap(MapRenderer& map_catalogue, TransportCatalogue& catalogue_) {
        std::vector<std::pair<const Bus*, int>> buses_palette;
        std::vector<const Stop*> stops_sort;

        const int palette_size = map_catalogue.GetPaletteSize();
        if (palette_size == 0) {
            return;
        }

        const auto& buses = catalogue_.ReturnBusMap();
        if (buses.empty()) {
            return;
        }

        for (const auto& bus_name : GetSortBusesNames(catalogue_)) {
            const Bus* bus_info = catalogue_.GetBus(bus_name);
            if (bus_info && !bus_info->stops_.empty()) {
                buses_palette.emplace_back(bus_info, buses_palette.size() % palette_size);
            }
        }

        if (!buses_palette.empty()) {
            map_catalogue.AddLine(buses_palette);
            map_catalogue.AddBusesName(buses_palette);
        }

        const auto& stops = catalogue_.ReturnStopToBuses();
        if (stops.empty()) {
            return;
        }

        std::vector<const Stop*> stops_name;
        for (const auto& [stop, buses_] : stops) {
            stops_name.push_back(stop);
        }
        std::sort(stops_name.begin(), stops_name.end(),
                  [](const Stop* a, const Stop* b) { return a->name_ < b->name_; });

        for (const auto& stop : stops_name) {
            stops_sort.push_back(catalogue_.GetStop(stop->name_));
        }

        if (!stops_sort.empty()) {
            map_catalogue.AddStopsCircle(stops_sort);
            map_catalogue.AddStopsName(stops_sort);
        }
    }

    std::vector<geo::Coordinates> RequestHandler::GetStopsCoordinates(TransportCatalogue& catalogue_) {

        std::vector<geo::Coordinates> stops_coordinates;
        auto buses = catalogue_.ReturnBusMap();

        for (auto& [busname, bus] : buses) {
            for (auto& stop : bus->stops_) {
                geo::Coordinates coordinates{};
                coordinates.latitude = stop->coordinates_.latitude;
                coordinates.longitude = stop->coordinates_.longitude;

                stops_coordinates.push_back(coordinates);
            }
        }
        return stops_coordinates;
    }

    std::vector<std::string_view> RequestHandler::GetSortBusesNames(TransportCatalogue& catalogue_) {
        std::vector<std::string_view> buses_names;

        auto buses = catalogue_.ReturnBusMap();
        if (!buses.empty()) {

            for (auto& [busname, bus] : buses) {
                buses_names.emplace_back(busname);
            }

            std::sort(buses_names.begin(), buses_names.end());

            return buses_names;

        } else {
            return {};
        }
    }


    const Document& RequestHandler::GetDocument(){
        return doc_out;
    }

}
