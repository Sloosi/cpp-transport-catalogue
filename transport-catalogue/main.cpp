#include <iostream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std;
using namespace transport_catalogue;
using namespace transport_router;

int main() {
    json_reader::JsonReader requests(std::cin);
    
    TransportCatalogue catalogue;
    requests.FillCatalogue(catalogue);
    
    RouterSettings router_settings = requests.ParseRouterSettings();
    TransportRouter router(catalogue, router_settings);

    renderer::MapRenderer map_renderer;
    requests.FillRenderSettings(map_renderer);

    request_handler::RequestHandler handler(requests, catalogue, router, map_renderer);
    handler.ProcessRequests();
}