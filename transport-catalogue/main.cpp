#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace std;
using namespace transport_catalogue;
using namespace map_renderer;
using namespace request_handler;
using namespace transport_catalogue::detail::json;
using namespace domain;

int main() {
    vector<StatRequest> stat_request;
    RenderSettings render_settings;
    TransportCatalogue catalogue;

    JSONReader json_reader;
    RequestHandler request_handler;

    json_reader = JSONReader(cin);
    json_reader.Parse(catalogue, stat_request, render_settings);

    request_handler = RequestHandler();
    request_handler.ExecuteQueries(catalogue, stat_request, render_settings);
    transport_catalogue::detail::json::Print(request_handler.GetDocument(), cout);
}
