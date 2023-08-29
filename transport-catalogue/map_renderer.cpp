#include "map_renderer.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "domain.h"

using namespace std;

namespace catalogue {

    void MapRenderer::AddLines(
            svg::Document& doc,
            geo::SphereProjector& sphere_projector,
            vector<Bus*>& buses
         ) {
        int color_idx = 0;
        int colors_count = render_settings_.color_palette.size();
        for (const auto& bus : buses) {
            svg::Polyline polyline;
            polyline.SetFillColor("none");
            polyline.SetStrokeWidth(render_settings_.line_width);
            polyline.SetStrokeColor(render_settings_.color_palette.at(color_idx % colors_count));
            polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            const auto stops = bus->stops;

            if (stops.empty()) {
                continue;
            }

            for (const auto& stop : stops) {
                polyline.AddPoint(sphere_projector(stop->coordinates));
            }

            if (!bus->is_roundtrip && (stops.size() > 1)) {
                for (int i = stops.size()-2; i >= 0; --i) {
                    polyline.AddPoint(sphere_projector(stops[i]->coordinates));
                }
            }

            doc.Add(polyline);
            ++color_idx;
        }
    }

    void MapRenderer::AddRoute(
            svg::Document& doc,
            geo::SphereProjector& sphere_projector,
            vector<Bus*>& buses
         ) {
        int color_idx = 0;
        int colors_count = render_settings_.color_palette.size();
        for (const auto& bus : buses) {
            const auto stops = bus->stops;

            if (stops.empty()) {
                continue;
            }

            svg::Text text;
            text.SetPosition(sphere_projector(stops[0]->coordinates));
            text.SetOffset(
                    {
                        render_settings_.bus_label_offset.first,
                        render_settings_.bus_label_offset.second
                    }
            );
            text.SetFillColor(render_settings_.color_palette.at(color_idx % colors_count));
            text.SetFontSize(render_settings_.bus_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetFontWeight("bold");
            text.SetData(bus->name);

            svg::Text substrate = text;
            substrate.SetFillColor(render_settings_.underlayer_color);
            substrate.SetStrokeColor(render_settings_.underlayer_color);
            substrate.SetStrokeWidth(render_settings_.underlayer_width);
            substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            doc.Add(substrate);
            doc.Add(text);

            size_t stops_count = bus->stops.size();
            if (!bus->is_roundtrip &&
                    (bus->stops[0]->name != bus->stops[stops_count-1]->name)) {
                svg::Point coordinate = sphere_projector(stops[stops_count-1]->coordinates);
                text.SetPosition(coordinate);
                substrate.SetPosition(coordinate);

                doc.Add(substrate);
                doc.Add(text);
            }
            ++color_idx;
        }
    }

    void MapRenderer::AddStops(
            svg::Document& doc,
            geo::SphereProjector& sphere_projector,
            std::vector<Stop*>& stops
         ) {
        vector<svg::Text> texts;
        for (const auto& stop : stops) {
            const StopInfo info = catalogue_.GetStopInfo(stop->name);
            if (!info.buses_exist) {
                continue;
            }

            svg::Circle circle;
            svg::Point coordinate = sphere_projector(stop->coordinates);
            circle.SetCenter(coordinate);
            circle.SetRadius(render_settings_.stop_radius);
            circle.SetFillColor("white");

            doc.Add(circle);

            svg::Text text;
            text.SetPosition(coordinate);
            text.SetOffset(
                    {
                        render_settings_.stop_label_offset.first,
                        render_settings_.stop_label_offset.second
                    }
            );
            text.SetFontSize(render_settings_.stop_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetData(stop->name);

            svg::Text substrate = text;
            substrate.SetFillColor(render_settings_.underlayer_color);
            substrate.SetStrokeColor(render_settings_.underlayer_color);
            substrate.SetStrokeWidth(render_settings_.underlayer_width);
            substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            text.SetFillColor("black");

            texts.push_back(substrate);
            texts.push_back(text);
        }

        for (const auto& text_elt : texts) {
            doc.Add(text_elt);
        }
    }

    void MapRenderer::DrawMap(svg::Document& doc) {
        vector<geo::Coordinates> geo_coords;
        const unordered_map<string_view, Stop*> stops_map = catalogue_.GetStopsMap();
        vector<Stop*> stops;
        stops.reserve(stops_map.size());
        for (const auto& stop_pair : stops_map) {
            const StopInfo stop_info = catalogue_.GetStopInfo(stop_pair.first);
            if (stop_info.buses_exist) {
                geo_coords.push_back(stop_pair.second->coordinates);
            }
            stops.push_back(stop_pair.second);
        }
        std::sort(stops.begin(), stops.end(), [](const auto& a, const auto&b) {return a->name < b->name;});

        geo::SphereProjector sphere_projector(
                geo_coords.begin(), geo_coords.end(), render_settings_.width,
                render_settings_.height, render_settings_.padding
        );

        const unordered_map<string_view, Bus*> buses_map = catalogue_.GetBusesMap();
        vector<Bus*> buses;
        buses.reserve(buses_map.size());
        for (const auto& bus_pair : buses_map) {
            buses.push_back(bus_pair.second);
        }
        std::sort(buses.begin(), buses.end(), [](const auto& a, const auto&b) {return a->name < b->name;});

        this->AddLines(doc, sphere_projector, buses);
        this->AddRoute(doc, sphere_projector, buses);
        this->AddStops(doc, sphere_projector, stops);
    }

} // namespace catalogue
