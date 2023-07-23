#pragma once

#include <vector>

#include "transport_catalogue.h"
#include "svg.h"

namespace catalogue {
    struct RenderSettings {
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        int bus_label_font_size;
        std::pair<double, double> bus_label_offset;
        int stop_label_font_size;
        std::pair<double, double> stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };

    class MapRenderer {
    public:
        MapRenderer(const TransportCatalogue& catalogue, const RenderSettings& render_settings)
                : catalogue_(catalogue)
                , render_settings_(render_settings) {};
        void DrawMap(svg::Document& doc);
    private:
        void AddLines(svg::Document& doc, geo::SphereProjector& sphere_projector, std::vector<Bus*>& buses);
        void AddRoute(svg::Document& doc, geo::SphereProjector& sphere_projector, std::vector<Bus*>& buses);
        void AddStops(svg::Document& doc, geo::SphereProjector& sphere_projector, std::vector<Stop*>& stops);

        const TransportCatalogue& catalogue_;
        const RenderSettings& render_settings_;
    };
} // namespace catalogue

