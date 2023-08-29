#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "geo.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace catalogue {
    class JsonReader {
    public:
        JsonReader(TransportCatalogue& catalogue, RenderSettings& render_settings, RoutingSettings& routing_settings, std::istream& input) :
                catalogue_(catalogue), render_settings_(render_settings), routing_settings_(routing_settings), doc_(json::Load(input)) {};
        void Parse();
        const json::Node& GetRequestNode() const;
        const TransportCatalogue& GetCatalogue() const;
        const RenderSettings& GetRenderSettings() const;
        void Handle(std::ostream &out);
    private:
        TransportCatalogue& catalogue_;
        RenderSettings& render_settings_;
        RoutingSettings& routing_settings_;
        std::unique_ptr<TransportRouter> router_;
        json::Document doc_;
    };

} // namespace catalogue
