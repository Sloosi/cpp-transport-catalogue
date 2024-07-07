#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>

namespace json_reader {

    class JsonReader {
    public:
        using StopData = std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>>;
        using RouteData = std::tuple<std::string_view, std::vector<const transport_catalogue::Stop*>, bool>;

        JsonReader(std::istream& input)
            : input_(json::Load(input))
        {}
        
        const json::Node& GetBaseRequests() const;
        const json::Node& GetStatRequests() const;
        const json::Node& GetRenderSettings() const;

        void FillCatalogue(transport_catalogue::TransportCatalogue& catalogue);
        void FillRenderSettings(renderer::MapRenderer& map_renderer) const;

    private:
        json::Document input_;
        json::Node dummy_ = nullptr;

        void FillStopDistances(transport_catalogue::TransportCatalogue& catalogue) const;
        StopData GetStopData(const json::Dict& request_map) const;
        RouteData GetRouteData(const json::Dict& request_map, transport_catalogue::TransportCatalogue& catalogue) const;
    };

} // namespace json_reader