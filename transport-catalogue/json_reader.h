#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

#include "geo.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace catalogue {
    class JsonReader {
    public:
        JsonReader(TransportCatalogue& catalogue, RenderSettings& render_settings, std::istream& input) :
                catalogue_(catalogue), render_settings_(render_settings), doc_(json::Load(input)) {};
        void Parse();
        const json::Node& GetRequestNode() const;
        const TransportCatalogue& GetCatalogue() const;
        const RenderSettings& GetRenderSettings() const;
        void Handle(std::ostream &out);
    private:
        TransportCatalogue& catalogue_;
        RenderSettings& render_settings_;
        json::Document doc_;
    };

} // namespace catalogue
