#include "map_renderer.h"

// Замечания исправлены

namespace map_renderer {
    bool SphereProjector::IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    MapRenderer::MapRenderer(RenderSettings& render_settings) : render_settings_(render_settings) {}

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {(coords.longitude - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.latitude) * zoom_coeff_ + padding_};
    }

    SphereProjector MapRenderer::GetSphereProjector(const std::vector<geo::Coordinates>& points) const {
        return {points.begin(),
                               points.end(),
                               render_settings_.width_,
                               render_settings_.height_,
                               render_settings_.padding_};
    }

    void MapRenderer::InitSphereProjector(std::vector<geo::Coordinates> points){
        sphere_projector_ = SphereProjector(points.begin(),
                                            points.end(),
                                            render_settings_.width_,
                                            render_settings_.height_,
                                            render_settings_.padding_);
    }

    RenderSettings MapRenderer::GetRenderSettings() const{
        return render_settings_;
    }

    int MapRenderer::GetPaletteSize() const{
        return render_settings_.color_palette_.size();
    }

    svg::Color MapRenderer::GetColor(int line_number) const {
        return render_settings_.color_palette_[line_number];
    }

    void MapRenderer::VisualizeLineProperties(svg::Polyline& polyline,
                                        [[maybe_unused]] int line_number) const {
        using namespace std::literals;

        polyline.SetStrokeColor(GetColor(line_number));
        polyline.SetFillColor("none"s);
        polyline.SetStrokeWidth(render_settings_.line_width_);
        polyline.SetStrokeLinecap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLinejoin(svg::StrokeLineJoin::ROUND);
    }

    void MapRenderer::VisualizeRouteTextCommonProperties(svg::Text& text,
                                                         const std::string& name,
                                                         svg::Point position) const {
        using namespace std::literals;

        text.SetPosition(position);
        text.SetOffset({render_settings_.bus_label_offset_.first,
                        render_settings_.bus_label_offset_.second});
        text.SetFontSize(render_settings_.bus_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(name);
    }

    void MapRenderer::VisualizeRouteTextAdditionalProperties(svg::Text& text,
                                                             const std::string& name,
                                                             svg::Point position) const {
        VisualizeRouteTextCommonProperties(text,
                                           name,
                                           position);

        text.SetFillColor(render_settings_.underlayer_color_);
        text.SetStrokeColor(render_settings_.underlayer_color_);
        text.SetStrokeWidth(render_settings_.underlayer_width_);
        text.SetStrokeLinejoin(svg::StrokeLineJoin::ROUND);
        text.SetStrokeLinecap(svg::StrokeLineCap::ROUND);
    }

    void MapRenderer::VisualizeRouteTextColorProperties(svg::Text& text,
                                                        const std::string& name,
                                                        int palette,
                                                        svg::Point position) const {
        VisualizeRouteTextCommonProperties(text,
                                           name,
                                           position);

        text.SetFillColor(GetColor(palette));
    }

    void MapRenderer::VisualizeStopsCirclesProperties(svg::Circle& circle,
                                                      svg::Point position) const {
        using namespace std::literals;

        circle.SetCenter(position);
        circle.SetRadius(render_settings_.stop_radius_);
        circle.SetFillColor("white");
    }

    void MapRenderer::VisualizeStopsTextCommonProperties(svg::Text& text,
                                                         const std::string& name,
                                                         svg::Point position) const {
        using namespace std::literals;

        text.SetPosition(position);
        text.SetOffset({render_settings_.stop_label_offset_.first,
                        render_settings_.stop_label_offset_.second});
        text.SetFontSize(render_settings_.stop_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetData(name);
    }

    void MapRenderer::VisualizeStopsTextAdditionalProperties(svg::Text& text,
                                                             const std::string& name,
                                                             svg::Point position) const {
        using namespace std::literals;
        VisualizeStopsTextCommonProperties(text,
                                           name,
                                           position);

        text.SetFillColor(render_settings_.underlayer_color_);
        text.SetStrokeColor(render_settings_.underlayer_color_);
        text.SetStrokeWidth(render_settings_.underlayer_width_);
        text.SetStrokeLinejoin(svg::StrokeLineJoin::ROUND);
        text.SetStrokeLinecap(svg::StrokeLineCap::ROUND);
    }

    void MapRenderer::VisualizeStopsTextColorProperties(svg::Text& text,
                                                        const std::string& name,
                                                        svg::Point position) const {
        using namespace std::literals;

        VisualizeStopsTextCommonProperties(text,
                                           name,
                                           position);
        text.SetFillColor("black");
    }

    void MapRenderer::AddLine(std::vector<std::pair<const Bus *, int>>& buses_palette) {
        std::vector<geo::Coordinates> stops_geo_coords;

        for (auto [bus, palette] : buses_palette) {
            for (auto& stop : bus->stops_) {
                geo::Coordinates coordinates{};
                coordinates.latitude = stop->coordinates_.latitude;
                coordinates.longitude = stop->coordinates_.longitude;

                stops_geo_coords.push_back(coordinates);
            }
            svg::Polyline bus_line;
            bool bus_empty = true;

            for (auto& coord : stops_geo_coords) {
                bus_empty = false;
                bus_line.AddPoint(sphere_projector_(coord));
            }

            if (!bus_empty) {
                VisualizeLineProperties(bus_line, palette);
                map_svg_.Add(bus_line);
            }
            stops_geo_coords.clear();
        }
    }

    void MapRenderer::AddBusesName(std::vector<std::pair<const Bus *, int>>& buses_palette){
        std::vector<geo::Coordinates> stops_geo_coords;
        bool bus_empty = true;

        for (auto [bus, palette] : buses_palette) {

            for (auto& stop : bus->stops_) {
                geo::Coordinates coordinates{};
                coordinates.latitude = stop->coordinates_.latitude;
                coordinates.longitude = stop->coordinates_.longitude;

                stops_geo_coords.push_back(coordinates);

                if(bus_empty) bus_empty = false;
            }

            svg::Text route_name_roundtrip;
            svg::Text route_title_roundtrip;
            svg::Text route_name_notroundtrip;
            svg::Text route_title_notroundtrip;

            if (!bus_empty) {
                if (bus->circular_route) {
                    VisualizeRouteTextAdditionalProperties(route_name_roundtrip,
                                                           std::string(bus->name_),
                                                           sphere_projector_(stops_geo_coords[0]));
                    map_svg_.Add(route_name_roundtrip);

                    VisualizeRouteTextColorProperties(route_title_roundtrip,
                                                      std::string(bus->name_),
                                                      palette,
                                                      sphere_projector_(stops_geo_coords[0]));
                    map_svg_.Add(route_title_roundtrip);

                } else {

                    VisualizeRouteTextAdditionalProperties(route_name_roundtrip,
                                                           std::string(bus->name_),
                                                           sphere_projector_(stops_geo_coords[0]));
                    map_svg_.Add(route_name_roundtrip);

                    VisualizeRouteTextColorProperties(route_title_roundtrip,
                                                      std::string(bus->name_),
                                                      palette,
                                                      sphere_projector_(stops_geo_coords[0]));
                    map_svg_.Add(route_title_roundtrip);

                    if (stops_geo_coords[0] != stops_geo_coords[stops_geo_coords.size()/2]) {

                        VisualizeRouteTextAdditionalProperties(route_name_notroundtrip,
                                                               std::string(bus->name_),
                                                               sphere_projector_(
                                                                       stops_geo_coords[stops_geo_coords.size() / 2]));
                        map_svg_.Add(route_name_notroundtrip);

                        VisualizeRouteTextColorProperties(route_title_notroundtrip,
                                                          std::string(bus->name_),
                                                          palette,
                                                          sphere_projector_(
                                                                  stops_geo_coords[stops_geo_coords.size() / 2]));
                        map_svg_.Add(route_title_notroundtrip);
                    }
                }
            }

            bus_empty = false;
            stops_geo_coords.clear();
        }
    }

    void MapRenderer::AddStopsCircle(const std::vector<const Stop *>& stops_name) {
        svg::Circle icon;
        std::vector<geo::Coordinates> stops_geo_coords;

        for (const Stop* stop_info : stops_name) {

            if (stop_info ) {
                geo::Coordinates coordinates{};
                coordinates.latitude = stop_info->coordinates_.latitude;
                coordinates.longitude = stop_info->coordinates_.longitude;

                VisualizeStopsCirclesProperties(icon,
                                                sphere_projector_(coordinates));
                map_svg_.Add(icon);
            }
        }
    }

    void MapRenderer::AddStopsName(const std::vector<const Stop *>& stops_name) {
        std::vector<geo::Coordinates> stops_geo_coords;

        svg::Text svg_stop_name;
        svg::Text svg_stop_name_title;

        for (const Stop *stop_info: stops_name) {

                if (stop_info) {
                    geo::Coordinates coordinates{};
                    coordinates.latitude = stop_info->coordinates_.latitude;
                    coordinates.longitude = stop_info->coordinates_.longitude;

                    VisualizeStopsTextAdditionalProperties(svg_stop_name,
                                                           stop_info->name_,
                                                           sphere_projector_(coordinates));
                    map_svg_.Add(svg_stop_name);

                    VisualizeStopsTextColorProperties(svg_stop_name_title,
                                                      stop_info->name_,
                                                      sphere_projector_(coordinates));
                    map_svg_.Add(svg_stop_name_title);
                }
            }
        }

    void MapRenderer::GetStreamMap(std::ostream& stream_) {
        map_svg_.Render(stream_);
    }

}
