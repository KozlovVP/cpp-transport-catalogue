#include <cstdlib>
#include <iostream>
#include <utility>
#include <sstream>

#include "json_reader.h"

using namespace std;
using namespace json;

namespace catalogue {
    using namespace geo;


    Coordinates GetCoordinates(const Node* node) {
        double lat = node->AsMap().at("latitude").AsDouble();
        double lng = node->AsMap().at("longitude").AsDouble();
        return {lat, lng};
    }

    vector<string> GetRoute(const Node* node) {
        vector<string> route;
        const auto stops = node->AsMap().at("stops").AsArray();
        for (const auto& stop : stops) {
            route.push_back(stop.AsString());
        }
        return route;
    }

    void ParseStopCommandForCoordinates(TransportCatalogue& catalogue, const Node* node) {
        const std::string name = node->AsMap().at("name").AsString();
        const Coordinates coordinates = GetCoordinates(node);
        catalogue.AddStop(name, coordinates);
    }

    void ParseStopCommandForDistances(TransportCatalogue& catalogue, const Node* node) {
        const std::string name = node->AsMap().at("name").AsString();
        const auto& stop_to_distances = node->AsMap().at("road_distances").AsMap();
        for (const auto& stop_to_distance : stop_to_distances) {
            catalogue.AddDistance(name, stop_to_distance.first, stop_to_distance.second.AsInt());
        }
    }

    svg::Color ParseColor(const Node& node) {
        if (node.IsString()) {
            return node.AsString();
        }

        svg::Color color;
        if (node.IsArray() && (node.AsArray().size() == 3)) {
            color = svg::Rgb{
                (uint8_t)node.AsArray()[0].AsInt(),
                (uint8_t)node.AsArray()[1].AsInt(),
                (uint8_t)node.AsArray()[2].AsInt()
            };
            return color;
        }

        if (node.IsArray() && (node.AsArray().size() == 4)) {
            color = svg::Rgba{
                (uint8_t)node.AsArray()[0].AsInt(),
                (uint8_t)node.AsArray()[1].AsInt(),
                (uint8_t)node.AsArray()[2].AsInt(),
                node.AsArray()[3].AsDouble()
            };
            return color;
        }

        return color;
    }

    void ParseBusCommand(TransportCatalogue& catalogue, const Node* node) {
        bool is_roundtrip = node->AsMap().at("is_roundtrip").AsBool();
        const std::string name = node->AsMap().at("name").AsString();
        const std::vector<std::string> route = GetRoute(node);
        catalogue.AddBus(name, route, is_roundtrip);
    }

    const Node& JsonReader::GetRequestNode() const {
        return doc_.GetRoot().AsMap().at("stat_requests");
    }

    const TransportCatalogue& JsonReader::GetCatalogue() const {
        return catalogue_;
    }

    const RenderSettings& JsonReader::GetRenderSettings() const {
        return render_settings_;
    }

    void JsonReader::Parse() {
        vector<const Node*> bus_commands;
        vector<const Node*> stop_commands;
        const auto& base_requests = doc_.GetRoot().AsMap().at("base_requests").AsArray();
        for (const auto& base_request : base_requests) {
            if (base_request.AsMap().at("type").AsString() == "Bus") {
                bus_commands.push_back(&base_request);
            } else {
                stop_commands.push_back(&base_request);
                ParseStopCommandForCoordinates(catalogue_, &base_request);
            }
        }

        for (const auto& stop_command : stop_commands) {
            ParseStopCommandForDistances(catalogue_, stop_command);
        }

        for (const auto& bus_command : bus_commands) {
            ParseBusCommand(catalogue_, bus_command);
        }

        const auto& render_settings = doc_.GetRoot().AsMap().at("render_settings").AsMap();
        render_settings_.width = render_settings.at("width").AsDouble();
        render_settings_.height = render_settings.at("height").AsDouble();
    render_settings_.padding = render_settings.at("padding").AsDouble();
        render_settings_.stop_radius = render_settings.at("stop_radius").AsDouble();
        render_settings_.line_width = render_settings.at("line_width").AsDouble();
        render_settings_.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
        const auto& bus_label_offset = render_settings.at("bus_label_offset").AsArray();
        render_settings_.bus_label_offset =
                make_pair(bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble());
        render_settings_.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
        const auto& stop_label_offset = render_settings.at("stop_label_offset").AsArray();
        render_settings_.stop_label_offset =
                make_pair(stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble());

        const auto& underlayer_color_node = render_settings.at("underlayer_color");
        render_settings_.underlayer_color = ParseColor(underlayer_color_node);

        render_settings_.underlayer_width = render_settings.at("underlayer_width").AsDouble();

        const auto& colors = render_settings.at("color_palette").AsArray();
        for (const auto& color : colors) {
            render_settings_.color_palette.push_back(ParseColor(color));
        }
    }

    void JsonReader::Handle(ostream &out) {
        const Node& requests = GetRequestNode();
        Array out_node;
        for (const auto& request : requests.AsArray()) {
            Dict dict_node;
            const string type = request.AsMap().at("type").AsString();
            const int id = request.AsMap().at("id").AsInt();
            if (type == "Bus"s) {
                const string name = request.AsMap().at("name").AsString();
                auto info = GetCatalogue().GetBusInfo(name);
                dict_node = (info.stops) ?
                        Dict{
                                {"curvature"s, info.road_info.length / info.road_info.distance},
                                {"request_id"s, id},
                                {"route_length"s, info.road_info.length},
                                {"stop_count"s, info.stops},
                                {"unique_stop_count"s, info.unique_stops}
                            } :
                        Dict{
                                {"error_message"s, "not found"s},
                                {"request_id"s, id}
                        };

            } else if (type == "Stop"s) {
                const string name = request.AsMap().at("name").AsString();
                auto info = GetCatalogue().GetStopInfo(name);
                if (info.stop_exist) {
                    Array arr_node;
                    if (info.buses_exist) {
                        for (const auto& bus : *info.buses) {
                            arr_node.push_back(string(bus));
                        }
                    }
                    dict_node = Dict{
                            {"buses"s, arr_node},
                            {"request_id"s, id}
                    };
                } else {
                    dict_node = Dict{
                            {"error_message"s, "not found"s},
                            {"request_id"s, id}
                    };
                }
            } else {
                const TransportCatalogue& catalogue = GetCatalogue();
                const RenderSettings& render_settings = GetRenderSettings();
                MapRenderer map_render(catalogue, render_settings);
                svg::Document doc;
                map_render.DrawMap(doc);
                std::ostringstream map_stream;
                doc.Render(map_stream);
                string map_str = map_stream.str();

                dict_node = Dict{
                        {"map"s, map_str},
                        {"request_id"s, id}
                };
            }
            out_node.push_back(dict_node);
        }
        Print(Document(out_node), out);
    }
}
