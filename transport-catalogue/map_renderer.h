#pragma once
#include <iostream>
#include <optional>
#include <algorithm>
#include <cstdlib>

#include "domain.h"
#include "geo.h"
#include "svg.h"

using namespace transport_catalogue;
using namespace domain;

namespace map_renderer {

    inline const double EPSILON = 1e-6;

    class SphereProjector {
    public:
        SphereProjector() = default;

        template <typename InputIt>
        SphereProjector(InputIt points_begin,
                        InputIt points_end,
                        double max_width,
                        double max_height,
                        double padding);

        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;

        static bool IsZero(double value);
    };

    struct RenderSettings {
        double width_;
        double height_;
        double padding_;
        double line_width_;
        double stop_radius_;
        int bus_label_font_size_;
        std::pair<double, double> bus_label_offset_;
        int stop_label_font_size_;
        std::pair<double, double> stop_label_offset_;
        svg::Color underlayer_color_;
        double underlayer_width_;
        std::vector<svg::Color> color_palette_;
    };

    class MapRenderer {

    public:
        explicit MapRenderer(RenderSettings& render_settings);

        [[nodiscard]] SphereProjector GetSphereProjector(const std::vector<geo::Coordinates>& points) const;
        void InitSphereProjector(std::vector<geo::Coordinates> points);

        [[nodiscard]] RenderSettings GetRenderSettings() const;
        [[nodiscard]] int GetPaletteSize() const;
        [[nodiscard]] svg::Color GetColor(int line_number) const;

        void VisualizeLineProperties(svg::Polyline& polyline, int line_number) const;

        void VisualizeRouteTextCommonProperties(svg::Text &text, const std::string& name, svg::Point position) const;
        void VisualizeRouteTextAdditionalProperties(svg::Text& text, const std::string& name, svg::Point position) const;
        void VisualizeRouteTextColorProperties(svg::Text& text, const std::string& name, int palette, svg::Point position) const;

        void VisualizeStopsCirclesProperties(svg::Circle& circle, svg::Point position) const;

        void VisualizeStopsTextCommonProperties(svg::Text& text, const std::string& name, svg::Point position) const;
        void VisualizeStopsTextAdditionalProperties(svg::Text& text, const std::string& name, svg::Point position) const;
        void VisualizeStopsTextColorProperties(svg::Text& text, const std::string& name, svg::Point position) const;

        void AddLine(std::vector<std::pair<const Bus *, int>>& buses_palette);
        void AddBusesName(std::vector<std::pair<const Bus *, int>>& buses_palette);
        void AddStopsCircle(const std::vector<const Stop *>& stops_name);
        void AddStopsName(const std::vector<const Stop *>& stops_name);

        void GetStreamMap(std::ostream& stream_);

    private:
        SphereProjector sphere_projector_;
        RenderSettings& render_settings_;
        svg::Document map_svg_;
    };

    template <typename InputIt>
    SphereProjector::SphereProjector(InputIt points_begin,
                                     InputIt points_end,
                                     double max_width,
                                     double max_height,
                                     double padding) : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it,
                right_it] = std::minmax_element(points_begin,
                                                points_end,
                                                [](auto lhs, auto rhs) {
                                                    return lhs.longitude < rhs.longitude;
                                                });

        min_lon_ = left_it->longitude;
        const double max_lon = right_it->longitude;

        const auto [bottom_it,
                top_it] = std::minmax_element(points_begin,
                                              points_end,
                                              [](auto lhs, auto rhs) {
                                                  return lhs.latitude < rhs.latitude;
                                              });

        const double min_lat = bottom_it->latitude;
        max_lat_ = top_it->latitude;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding)
                         / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding)
                          / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom,
                                   *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;

        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;

        }
    }

}
