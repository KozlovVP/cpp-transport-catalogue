#pragma once
#include <sstream>
#include "json_reader.h"


using namespace transport_catalogue;
using namespace map_renderer;
using namespace transport_catalogue::detail::json;


namespace request_handler {

    class RequestHandler {
    public:

        RequestHandler() = default;

        static std::vector<geo::Coordinates> GetStopsCoordinates(TransportCatalogue& catalogue_) ;
        static std::vector<std::string_view> GetSortBusesNames(TransportCatalogue& catalogue_) ;

        void ExecuteQueries(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_requests, RenderSettings& render_settings);
        static void ExecuteRenderMap(MapRenderer& map_catalogue, TransportCatalogue& catalogue_) ;

        const Document& GetDocument();

    private:
        Document doc_out;
    };

}
