#pragma once
#include "transport_catalogue.h"
#include <algorithm>
#include <vector>
#include <ostream>

    namespace domain {

        struct StatRequest {
            int id;
            std::string type_;
            std::string name_;
        };

        struct BusInfoAfterQuery{
            std::string_view name_;
            bool not_found_;
            int stops_on_route_;
            int unique_stops_;
            int route_length_;
            double curvature_;
        };

        struct StopInfoAfterQuery{
            std::string_view name_;
            bool not_found_;
            std::vector <std::string> buses_name_;
        };
    }
