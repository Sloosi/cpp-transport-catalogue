#include "request_handler.h"

using namespace request_handler;
using namespace transport_catalogue;
using namespace json;
using namespace std::literals;

namespace {
    std::pair<double, double> CalculateRouteAndGeoLengths(const TransportCatalogue& catalogue, const Bus* bus_ptr) {
        double route_length = 0;
        double geo_length = 0.0;

        for (size_t i = 0; i < bus_ptr->stops.size() - 1; ++i) {
            auto from = bus_ptr->stops[i];
            auto to = bus_ptr->stops[i + 1];
            if (bus_ptr->is_roundtrip) {
                route_length += catalogue.GetDistance(from, to);
                geo_length += geo::ComputeDistance(from->coordinates, to->coordinates);
            }
            else {
                route_length += catalogue.GetDistance(from, to) + catalogue.GetDistance(to, from);
                geo_length += geo::ComputeDistance(from->coordinates, to->coordinates) * 2;
            }
        }

        return { route_length, geo_length };
    }
}

void RequestHandler::ProcessRequests() const {
    json::Array result;
    const json::Array& arr = requests_.GetStatRequests().AsArray();
    for (auto& request : arr) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            result.push_back(PrintStop(request_map).AsDict());
        }
        if (type == "Bus") {
            result.push_back(PrintBus(request_map).AsDict());
        }
        if (type == "Map") {
            result.push_back(PrintMap(request_map).AsDict());
        }
        if (type == "Route") {
            result.push_back(PrintRoute(request_map).AsDict());
        }
    }
    json::Print(json::Document{ result }, std::cout);
}

const json::Node RequestHandler::PrintBus(const json::Dict& request_map) const {
    json::Node result;
    const std::string& route_number = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();

    if (!catalogue_.FindRoute(route_number)) {
        result = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("error_message").Value("not found")
                .EndDict()
            .Build();
    }
    else {
        const auto& route_info = GetBusInfo(route_number);
        result = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("curvature").Value(route_info->curvature)
                    .Key("route_length").Value(route_info->route_length)
                    .Key("stop_count").Value(static_cast<int>(route_info->stops_count))
                    .Key("unique_stop_count").Value(static_cast<int>(route_info->unique_stops_count))
                .EndDict()
            .Build();
    }
    return result;

}

const json::Node RequestHandler::PrintStop(const json::Dict& request_map) const {
    json::Node result;
    const std::string& stop_name = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();

    if (!catalogue_.FindStop(stop_name)) {
        result = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("error_message").Value("not found")
                .EndDict()
            .Build();
    }
    else {
        json::Array buses;
        for (auto& bus : GetBusesByStop(stop_name)) {
            buses.push_back(bus);
        }
        result = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("buses").Value(buses)
                .EndDict()
            .Build();
    }

    return result;
}

const json::Node RequestHandler::PrintMap(const json::Dict& request_map) const {
    json::Node result;
    const int id = request_map.at("id").AsInt();

    svg::Document map = CreateMap();
    std::ostringstream out;
    map.Render(out);

    result = json::Builder{}
            .StartDict()
                .Key("request_id").Value(id)
                .Key("map").Value(out.str())
            .EndDict()
        .Build();

    return result;
}

const json::Node request_handler::RequestHandler::PrintRoute(const json::Dict& request_map) const {
    json::Node result;
    const int id = request_map.at("id"s).AsInt();
    const std::string_view from = request_map.at("from"s).AsString();
    const std::string_view to = request_map.at("to"s).AsString();
    const auto& route = router_.BuildRouteData(from, to);

    if (!route) {
        result = json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(id)
                    .Key("error_message"s).Value("not found"s)
                .EndDict()
            .Build();
    }
    else {
        result = json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(id)
                    .Key("total_time"s).Value(route->total_time)
                    .Key("items"s).Value(route->data)
                .EndDict()
            .Build();
    }

    return result;
}

std::optional<transport_catalogue::BusInfo> RequestHandler::GetBusInfo(std::string_view bus) const {
    BusInfo bus_info = {};

    const Bus* bus_ptr = catalogue_.FindRoute(bus);
    if (bus_ptr == nullptr) {
        return std::nullopt;
    }
    auto [route_length, geo_length] = CalculateRouteAndGeoLengths(catalogue_, bus_ptr);

    bus_info.geo_route_length = geo_length;
    bus_info.route_length = route_length;
    bus_info.curvature = static_cast<double>(bus_info.route_length) / bus_info.geo_route_length;

    bus_info.stops_count = bus_ptr->is_roundtrip ? bus_ptr->stops.size() : bus_ptr->stops.size() * 2 - 1;
    bus_info.unique_stops_count = catalogue_.UniqueStopsCount(std::string(bus));

    return bus_info;
}

const std::set<std::string> RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name)->buses;
}

svg::Document RequestHandler::CreateMap() const {
    renderer::MapRenderer::Buses buses;
    for (const auto& bus : catalogue_.GetBuses()) {
        buses.insert(bus);
    }

    return renderer_.GetSVG(buses);
}