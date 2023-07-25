#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "json_builder.h"

using namespace map_renderer;
using namespace transport_catalogue;
using namespace transport_catalogue::detail::json::builder;
namespace transport_catalogue::detail::json {

            class JSONReader {
            public:
                JSONReader() = default;

                JSONReader(Document doc);

                JSONReader(std::istream &input);

                void ParseNodeBase(const Node &root, TransportCatalogue &catalogue);

                static void ParseNodeStat(const Node &root, std::vector<StatRequest> &stat_request);

                static void ParseNodeRender(const Node &node, map_renderer::RenderSettings &render_settings);

                void ParseNode(const Node &root, TransportCatalogue &catalogue, std::vector<StatRequest> &stat_request,
                               map_renderer::RenderSettings &render_settings);

                void Parse(TransportCatalogue &catalogue, std::vector<StatRequest> &stat_request,
                           map_renderer::RenderSettings &render_settings);

                static Stop ParseNodeStop(Node &node);

                static Bus ParseNodeBus(Node &node, TransportCatalogue &catalogue);

                using DistanceTuple = std::tuple<const Stop *, const Stop *, int>;

                static std::vector<DistanceTuple> ParseNodeDistances(Node &node, TransportCatalogue &catalogue);

                static Node ExecuteMakeNodeMap(int id_request, TransportCatalogue& catalogue, RenderSettings render_settings);

                static Node ExecuteMakeNodeStop(int id_request, const StopInfoAfterQuery &query_result);

                static Node ExecuteMakeNodeBus(int id_request, BusInfoAfterQuery query_result);

                static BusInfoAfterQuery BusQuery(TransportCatalogue& catalogue, std::string_view bus_name);
                static StopInfoAfterQuery StopQuery(TransportCatalogue& catalogue, std::string_view stop_name);
                

                [[maybe_unused]] const Document &GetDocument() const;

            private:
                Document document_;

                static svg::Color ParseColor(const Node &node);
            };

        }
