#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

using namespace std;
using namespace catalogue;

int main() {
    RenderSettings render_settings;
    TransportCatalogue catalogue;
    JsonReader json_reader(catalogue, render_settings, cin);
    json_reader.Parse();
    MapRenderer map_renderer(catalogue, render_settings);
    json_reader.Handle(cout);
    return 0;
}
