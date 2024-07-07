#include <iostream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std;
using namespace transport_catalogue;

int main() {
    TransportCatalogue catalogue;
    renderer::MapRenderer map_renderer;
    json_reader::JsonReader requests(std::cin);
    requests.FillCatalogue(catalogue);
    requests.FillRenderSettings(map_renderer);

    request_handler::RequestHandler handler(requests, catalogue, map_renderer);
    handler.ProcessRequests();
}