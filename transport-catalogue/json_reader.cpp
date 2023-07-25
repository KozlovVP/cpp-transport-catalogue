#include "json_reader.h"

// Замечания исправлены

using namespace request_handler;
namespace transport_catalogue::detail::json {


            JSONReader::JSONReader(Document doc) : document_(std::move(doc)) {}

            JSONReader::JSONReader(std::istream &input) : document_(json::Load(input)) {}

            Stop JSONReader::ParseNodeStop(Node &node) {
                Stop stop;
                Dict stop_node;

                if (node.IsMap()) {
                    stop_node = node.AsMap();
                    stop.name_ = stop_node.at("name").AsString();
                    stop.coordinates_.latitude = stop_node.at("latitude").AsDouble();
                    stop.coordinates_.longitude = stop_node.at("longitude").AsDouble();
                }

                return stop;
            }

    std::vector<JSONReader::DistanceTuple> JSONReader::ParseNodeDistances(Node& node, TransportCatalogue& catalogue) {
        std::vector<DistanceTuple> distances;

        if (node.IsMap()) {
            const Dict& stop_node = node.AsMap();
            const std::string& begin_name = stop_node.at("name").AsString();

            try {
                const Dict& stop_road_map = stop_node.at("road_distances").AsMap();

                for (const auto& [last_name, distanceNode] : stop_road_map) {
                    if (distanceNode.IsInt()) {
                        int distance = distanceNode.AsInt();
                        distances.emplace_back(catalogue.GetStop(begin_name), catalogue.GetStop(last_name), distance);
                    }
                }
            } catch (...) {
                throw std::runtime_error("Invalid road");
            }
        }

        return distances;
    }

            Bus JSONReader::ParseNodeBus(Node &node, TransportCatalogue &catalogue) {
                Bus bus;
                Dict bus_node;
                Array bus_stops;

                if (node.IsMap()) {
                    bus_node = node.AsMap();
                    bus.name_ = bus_node.at("name").AsString();
                    bus.circular_route = bus_node.at("is_roundtrip").AsBool();

                    try {
                        bus_stops = bus_node.at("stops").AsArray();

                        for (const Node &stop: bus_stops) {
                            bus.stops_.push_back(catalogue.GetStop(stop.AsString()));
                        }

                        if (!bus.circular_route) {
                            size_t size = bus.stops_.size() - 1;

                            for (size_t i = size; i > 0; i--) {
                                bus.stops_.push_back(bus.stops_[i - 1]);
                            }

                        }

                    } catch (...) {
                        throw std::runtime_error("base_requests: bus: stops is empty");
                    }
                }

                return bus;
            }

            void JSONReader::ParseNodeBase(const Node &root, TransportCatalogue &catalogue) {
                Array base_requests;
                Dict req_map;
                Node req_node;

                std::vector<Node> buses;
                std::vector<Node> stops;

                if (root.IsArray()) {
                    base_requests = root.AsArray();

                    for (Node &node: base_requests) {
                        if (node.IsMap()) {
                            req_map = node.AsMap();

                            try {
                                req_node = req_map.at("type");

                                if (req_node.IsString()) {

                                    if (req_node.AsString() == "Bus") {
                                        buses.emplace_back(req_map);
                                    } else if (req_node.AsString() == "Stop") {
                                        stops.emplace_back(req_map);
                                    } else {
                                        throw std::runtime_error("base_requests are invalid");
                                    }
                                }

                            } catch (...) {
                                throw std::runtime_error("base_requests does not have type Value");
                            }
                        }
                    }

                    for (auto stop: stops) {
                        catalogue.AddStop(ParseNodeStop(stop));
                    }

                    for (auto stop: stops) {
                        auto distances = ParseNodeDistances(stop, catalogue);
                        for (const auto &distance: distances) {
                            catalogue.SetDistance(*std::get<0>(distance),
                                                  *std::get<1>(distance),
                                                  std::get<2>(distance));
                        }
                    }

                    for (auto bus: buses) {
                        catalogue.AddBus(ParseNodeBus(bus, catalogue));
                    }

                } else {
                    throw std::runtime_error("base_requests is not an array");
                }
            }

            void JSONReader::ParseNodeStat(const Node &root, std::vector<StatRequest> &stat_request) {
                Array stat_requests;
                Dict req_map;
                StatRequest req;

                if (root.IsArray()) {
                    stat_requests = root.AsArray();

                    for (const Node &req_node: stat_requests) {

                        if (req_node.IsMap()) {
                            req_map = req_node.AsMap();
                            req.id = req_map.at("id").AsInt();
                            req.type_ = req_map.at("type").AsString();

                            if (req.type_ != "Map") {
                                req.name_ = req_map.at("name").AsString();
                            } else {
                                req.name_ = "";
                            }

                            stat_request.push_back(req);

                        }
                    }

                } else {
                    throw std::runtime_error("base_requests is not array");
                }
            }

    void JSONReader::ParseNodeRender(const Node& node, map_renderer::RenderSettings& render_settings) {
        if (!node.IsMap()) {
            throw std::runtime_error("render_settings is not a map");
        }

        const Dict& rend_map = node.AsMap();

        try {
            render_settings.width_ = rend_map.at("width").AsDouble();
            render_settings.height_ = rend_map.at("height").AsDouble();
            render_settings.padding_ = rend_map.at("padding").AsDouble();
            render_settings.line_width_ = rend_map.at("line_width").AsDouble();
            render_settings.stop_radius_ = rend_map.at("stop_radius").AsDouble();

            render_settings.bus_label_font_size_ = rend_map.at("bus_label_font_size").AsInt();

            if (const auto& bus_label_offset = rend_map.at("bus_label_offset").AsArray(); bus_label_offset.size() == 2) {
                render_settings.bus_label_offset_ = {
                        bus_label_offset[0].AsDouble(),
                        bus_label_offset[1].AsDouble()
                };
            }

            render_settings.stop_label_font_size_ = rend_map.at("stop_label_font_size").AsInt();

            if (const auto& stop_label_offset = rend_map.at("stop_label_offset").AsArray(); stop_label_offset.size() == 2) {
                render_settings.stop_label_offset_ = {
                        stop_label_offset[0].AsDouble(),
                        stop_label_offset[1].AsDouble()
                };
            }

            render_settings.underlayer_color_ = ParseColor(rend_map.at("underlayer_color"));
            render_settings.underlayer_width_ = rend_map.at("underlayer_width").AsDouble();

            const auto& color_palette = rend_map.at("color_palette");
            if (color_palette.IsArray()) {
                for (const auto& palette_entry : color_palette.AsArray()) {
                    render_settings.color_palette_.emplace_back(ParseColor(palette_entry));
                }
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Unable to parse init settings: " + std::string(e.what()));
        }
    }

    svg::Color JSONReader::ParseColor(const Node& node) {
        if (node.IsString()) {
            return svg::Color(node.AsString());
        } else if (node.IsArray()) {
            const auto& arr_color = node.AsArray();
            if (arr_color.size() >= 3) {
                uint8_t red = arr_color[0].AsInt();
                uint8_t green = arr_color[1].AsInt();
                uint8_t blue = arr_color[2].AsInt();

                if (arr_color.size() == 4) {
                    double opacity = arr_color[3].AsDouble();
                    return svg::Color(svg::Rgba(red, green, blue, opacity));
                } else {
                    return svg::Color(svg::Rgb(red, green, blue));
                }
            }
        }

        throw std::runtime_error("Invalid color format");
    }

            void JSONReader::ParseNode(const Node &root, TransportCatalogue &catalogue,
                                       [[maybe_unused]] std::vector<StatRequest> &stat_request,
                                       map_renderer::RenderSettings &render_settings) {
                Dict root_dictionary;

                if (root.IsMap()) {
                    root_dictionary = root.AsMap();

                    try {
                        ParseNodeBase(root_dictionary.at("base_requests"), catalogue);
                    } catch (...) {
                        throw std::runtime_error("base_requests is empty");
                    }

                    try {
                        ParseNodeStat(root_dictionary.at("stat_requests"), stat_request);
                    } catch (...) {
                        throw std::runtime_error("stat_requests is empty");
                    }

                    try {
                        ParseNodeRender(root_dictionary.at("render_settings"), render_settings);
                    } catch (...) {
                        throw std::runtime_error("render_settings is empty");
                    }

                } else {
                    throw std::runtime_error("root is not map");
                }
            }

            void JSONReader::Parse(TransportCatalogue &catalogue, std::vector<StatRequest> &stat_request,
                                   map_renderer::RenderSettings &render_settings) {
                ParseNode(document_.GetRoot(),
                          catalogue,
                          stat_request,
                          render_settings);
            }

            [[maybe_unused]] const Document &JSONReader::GetDocument() const {
                return document_;
            }

    Node JSONReader::ExecuteMakeNodeStop(int id_request, const StopInfoAfterQuery& query_result){
        Node result;
        Array buses;
        Builder builder;

        std::string str_not_found = "not found";

        if (query_result.not_found_) {
            builder.StartDict()
                    .Key("request_id").Value(id_request)
                    .Key("error_message").Value(str_not_found)
                    .EndDict();

            result = builder.Build();

        } else {
            builder.StartDict()
                    .Key("request_id").Value(id_request)
                    .Key("buses").StartArray();

            for (std::string bus_name : query_result.buses_name_) {
                builder.Value(bus_name);
            }

            builder.EndArray().EndDict();

            result = builder.Build();
        }

        return result;
    }

    Node JSONReader::ExecuteMakeNodeBus(int id_request, BusInfoAfterQuery query_result){
        Node result;
        std::string str_not_found = "not found";

        if (query_result.not_found_) {
            result = Builder{}.StartDict()
                    .Key("request_id").Value(id_request)
                    .Key("error_message").Value(str_not_found)
                    .EndDict()
                    .Build();
        } else {
            result = Builder{}.StartDict()
                    .Key("request_id").Value(id_request)
                    .Key("curvature").Value(query_result.curvature_)
                    .Key("route_length").Value(query_result.route_length_)
                    .Key("stop_count").Value(query_result.stops_on_route_)
                    .Key("unique_stop_count").Value(query_result.unique_stops_)
                    .EndDict()
                    .Build();
        }

        return result;
    }

    Node JSONReader::ExecuteMakeNodeMap(int id_request, TransportCatalogue& catalogue, RenderSettings render_settings) {
        Node result;
        std::ostringstream map_stream;
        std::string map_str;

        MapRenderer map_catalogue(render_settings);
        map_catalogue.InitSphereProjector(RequestHandler::GetStopsCoordinates(catalogue));
        RequestHandler::ExecuteRenderMap(map_catalogue, catalogue);
        map_catalogue.GetStreamMap(map_stream);
        map_str = map_stream.str();

        result = Builder{}.StartDict()
                .Key("request_id").Value(id_request)
                .Key("map").Value(map_str)
                .EndDict()
                .Build();

        return result;
    }

    BusInfoAfterQuery JSONReader::BusQuery(TransportCatalogue& catalogue, std::string_view bus_name) {
        BusInfoAfterQuery bus_info;
        const Bus *bus = catalogue.GetBus(bus_name);
        if (bus != nullptr) {
            bus_info.name_ = bus->name_;
            bus_info.not_found_ = false;
            auto info = catalogue.GetBusInfo(*bus);
            bus_info.stops_on_route_ = info.stops_on_route;
            bus_info.unique_stops_ = info.unique_stops;
            bus_info.route_length_ = info.route_length;
            bus_info.curvature_ = info.curvature;
        } else {
            bus_info.name_ = bus_name;
            bus_info.not_found_ = true;
        }

        return bus_info;
    }

    StopInfoAfterQuery JSONReader::StopQuery(TransportCatalogue& catalogue, std::string_view stop_name) {
        std::unordered_set<Bus, BusHash> unique_buses;
        std::vector<std::string> bus_name_v;

        StopInfoAfterQuery stop_info;

        Stop *stop = const_cast<Stop *>(catalogue.GetStop(stop_name));

        if (stop != nullptr) {

            stop_info.name_ = stop->name_;
            stop_info.not_found_ = false;
            unique_buses = catalogue.StopGetUniqueBuses(stop);

            if (!unique_buses.empty()) {

                for (const Bus &bus: unique_buses) {
                    stop_info.buses_name_.push_back(bus.name_);
                }

                std::sort(stop_info.buses_name_.begin(), stop_info.buses_name_.end());
            }

        } else {
            stop_info.name_ = stop_name;
            stop_info.not_found_ = true;
        }

        return stop_info;
    }

        }
