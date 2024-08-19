#include "json_reader.h"

using namespace transport_catalogue;
using namespace transport_router;
using namespace json_reader;

namespace {
    svg::Point SetOffset(const json::Array& offset_data) {
        svg::Point result;
        result.x = offset_data.at(0).AsDouble();
        result.y = offset_data.at(1).AsDouble();
        return result;
    }

    svg::Color SetColor(const json::Node& color_data) {
        if (color_data.IsString()) {
            return color_data.AsString();
        }

        if (color_data.IsArray() && color_data.AsArray().size() == 3) {
            auto result_color = svg::Rgb(
                static_cast<uint8_t>(color_data.AsArray().at(0).AsInt()),
                static_cast<uint8_t>(color_data.AsArray().at(1).AsInt()),
                static_cast<uint8_t>(color_data.AsArray().at(2).AsInt())
            );
            return result_color;
        }

        if (color_data.IsArray() && color_data.AsArray().size() == 4) {
            auto result_color = svg::Rgba(
                static_cast<uint8_t>(color_data.AsArray().at(0).AsInt()),
                static_cast<uint8_t>(color_data.AsArray().at(1).AsInt()),
                static_cast<uint8_t>(color_data.AsArray().at(2).AsInt()),
                color_data.AsArray().at(3).AsDouble()
            );
            return result_color;
        }
        
        return svg::NoneColor;
        
    }

} // namespace

const json::Node& JsonReader::GetBaseRequests() const {
    if (!input_.GetRoot().AsDict().count("base_requests")) {
        return dummy_;
    }
    return input_.GetRoot().AsDict().at("base_requests");
}

const json::Node& JsonReader::GetStatRequests() const {
    if (!input_.GetRoot().AsDict().count("stat_requests")) {
        return dummy_;
    }
    return input_.GetRoot().AsDict().at("stat_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    if (!input_.GetRoot().AsDict().count("render_settings")) {
        return dummy_;
    }
    return input_.GetRoot().AsDict().at("render_settings");
}

const json::Node& json_reader::JsonReader::GetRouterSettings() const {
    if (!input_.GetRoot().AsDict().count("routing_settings")) {
        return dummy_;
    }
    return input_.GetRoot().AsDict().at("routing_settings");
}

void JsonReader::FillCatalogue(TransportCatalogue& catalogue) {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsDict();
        const auto& type = request_stops_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = GetStopData(request_stops_map);
            catalogue.AddStop(std::string(stop_name), coordinates);
        }
    }

    FillStopDistances(catalogue);

    for (auto& request_bus : arr) {
        const auto& request_bus_map = request_bus.AsDict();
        const auto& type = request_bus_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_number, stops, circular_route] = GetRouteData(request_bus_map, catalogue);
            catalogue.AddRoute(std::string(bus_number), stops, circular_route);
        }
    }
}

void JsonReader::FillRenderSettings(renderer::MapRenderer& map_renderer) const {
    auto& request = GetRenderSettings();
    if (request.IsNull()) {
        return;
    }
    auto& request_map = request.AsDict();
    
    renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width").AsDouble();
    render_settings.height = request_map.at("height").AsDouble();
    render_settings.padding = request_map.at("padding").AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius").AsDouble();
    render_settings.line_width = request_map.at("line_width").AsDouble();
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size").AsInt();
    render_settings.stop_label_font_size = request_map.at("stop_label_font_size").AsInt();
    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    render_settings.bus_label_offset = SetOffset(request_map.at("bus_label_offset").AsArray());
    render_settings.stop_label_offset = SetOffset(request_map.at("stop_label_offset").AsArray());
    render_settings.underlayer_color = SetColor(request_map.at("underlayer_color"));

    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        render_settings.color_palette.push_back(SetColor(color_element));
    }

    map_renderer.SetRendererSettings(render_settings);
}

RouterSettings json_reader::JsonReader::ParseRouterSettings() const {
    auto& request = GetRouterSettings();
    if (request.IsNull()) {
        return {0, 0};
    }
    auto& request_map = request.AsDict();

    RouterSettings settings;
    settings.bus_wait_time = request_map.at("bus_wait_time").AsDouble();
    settings.bus_velocity = request_map.at("bus_velocity").AsDouble();

    return settings;
}



JsonReader::StopData JsonReader::GetStopData(const json::Dict& request_map) const {
    std::string_view stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = { request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble() };
    std::map<std::string_view, int> stop_distances;
    auto& distances = request_map.at("road_distances").AsDict();
    for (auto& [stop_name, dist] : distances) {
        stop_distances.emplace(stop_name, dist.AsInt());
    }
    return std::make_tuple(stop_name, coordinates, stop_distances);
}

void JsonReader::FillStopDistances(TransportCatalogue& catalogue) const {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsDict();
        const auto& type = request_stops_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = GetStopData(request_stops_map);
            for (auto& [to_name, dist] : stop_distances) {
                auto from = catalogue.FindStop(stop_name);
                auto to = catalogue.FindStop(to_name);
                catalogue.SetDistance({ from, to }, dist);
            }
        }
    }
}

JsonReader::RouteData JsonReader::GetRouteData(const json::Dict& request_map, TransportCatalogue& catalogue) const {
    std::string_view bus_number = request_map.at("name").AsString();
    std::vector<const Stop*> stops;
    for (auto& stop : request_map.at("stops").AsArray()) {
        stops.push_back(catalogue.FindStop(stop.AsString()));
    }

    bool is_roundtrip = request_map.at("is_roundtrip").AsBool();

    return std::make_tuple(bus_number, stops, is_roundtrip);
}