#pragma once 

#include "json.h"
#include "json_reader.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include <optional>
#include <sstream>

namespace request_handler {
    class RequestHandler {
    public:
        explicit RequestHandler(
            json_reader::JsonReader& requests,
            const transport_catalogue::TransportCatalogue& catalogue,
            const transport_router::TransportRouter& router,
            const renderer::MapRenderer& renderer
        ) :
            requests_(requests),
            catalogue_(catalogue),
            router_(router),
            renderer_(renderer)
        {}

        void ProcessRequests() const;
        svg::Document CreateMap() const;

        const json::Node PrintBus(const json::Dict& request_map) const;
        const json::Node PrintStop(const json::Dict& request_map) const;
        const json::Node PrintMap(const json::Dict& request_map) const;
        const json::Node PrintRoute(const json::Dict& request_map) const;

        std::optional<transport_catalogue::BusInfo> GetBusInfo(std::string_view bus_number) const;
        const std::set<std::string> GetBusesByStop(std::string_view stop_name) const;

    private:
        const json_reader::JsonReader& requests_;
        const transport_catalogue::TransportCatalogue& catalogue_;
        const transport_router::TransportRouter& router_;
        const renderer::MapRenderer& renderer_;
    };
} // namespace request_handler