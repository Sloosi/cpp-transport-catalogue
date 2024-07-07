#include "map_renderer.h"

namespace renderer {

    namespace {
        inline void Iterate(size_t& iterator, size_t upper_bound) {
            (iterator < upper_bound - 1) ? ++iterator : iterator = 0;
        }
    } // namespace

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    void MapRenderer::SetRendererSettings(const RenderSettings& settings) {
        render_settings_ = settings;
    }

    std::vector<svg::Polyline> MapRenderer::GetRouteLines(const Buses& buses, const SphereProjector& sp) const {
        std::vector<svg::Polyline> result;

        size_t color_it = 0;
        size_t color_end = render_settings_.color_palette.size();

        for (const auto& [bus_number, bus] : buses) {
            if (bus->stops.empty()) {
                continue;
            }

            svg::Polyline line;
            line.SetStrokeColor(render_settings_.color_palette[color_it]);
            line.SetFillColor("none");
            line.SetStrokeWidth(render_settings_.line_width);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            std::vector<const transport_catalogue::Stop*> route_stops{ bus->stops.begin(), bus->stops.end() };
            if (!bus->is_roundtrip) {
                route_stops.insert(route_stops.end(), std::next(bus->stops.rbegin()), bus->stops.rend());
            }

            for (const auto& stop : route_stops) {
                line.AddPoint(sp(stop->coordinates));
            }

            result.push_back(line);
            Iterate(color_it, color_end);
        }

        return result;
    }

    std::vector<svg::Text> MapRenderer::GetBusLabel(const Buses& buses, const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        size_t color_it = 0;
        size_t color_end = render_settings_.color_palette.size();

        for (const auto& [bus_number, bus] : buses) {
            if (bus->stops.empty()) {
                continue;
            }

            svg::Text text;
            text.SetPosition(sp(bus->stops[0]->coordinates));
            text.SetOffset(render_settings_.bus_label_offset);
            text.SetFontSize(render_settings_.bus_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetFontWeight("bold");
            text.SetData(bus->number);
            text.SetFillColor(render_settings_.color_palette[color_it]);
            Iterate(color_it, color_end);

            svg::Text underlayer;
            underlayer.SetPosition(sp(bus->stops[0]->coordinates));
            underlayer.SetOffset(render_settings_.bus_label_offset);
            underlayer.SetFontSize(render_settings_.bus_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetFontWeight("bold");
            underlayer.SetData(bus->number);
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            result.push_back(underlayer);
            result.push_back(text);

            if (!bus->is_roundtrip && bus->stops[0] != bus->stops[bus->stops.size() - 1]) {
                svg::Text text2{ text };
                svg::Text underlayer2{ underlayer };
                text2.SetPosition(sp(bus->stops[bus->stops.size() - 1]->coordinates));
                underlayer2.SetPosition(sp(bus->stops[bus->stops.size() - 1]->coordinates));

                result.push_back(underlayer2);
                result.push_back(text2);
            }
        }

        return result;
    }

    std::vector<svg::Circle> MapRenderer::GetStopsSymbols(const Stops& stops, const SphereProjector& sp) const {
        std::vector<svg::Circle> result;

        for (const auto& [stop_name, stop] : stops) {
            svg::Circle symbol;
            symbol.SetCenter(sp(stop->coordinates));
            symbol.SetRadius(render_settings_.stop_radius);
            symbol.SetFillColor("white");

            result.push_back(symbol);
        }

        return result;
    }

    std::vector<svg::Text> MapRenderer::GetStopsLabels(const Stops& stops, const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        for (const auto& [stop_name, stop] : stops) {
            svg::Text text;
            text.SetPosition(sp(stop->coordinates));
            text.SetOffset(render_settings_.stop_label_offset);
            text.SetFontSize(render_settings_.stop_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetData(stop->name);
            text.SetFillColor("black");

            svg::Text underlayer;
            underlayer.SetPosition(sp(stop->coordinates));
            underlayer.SetOffset(render_settings_.stop_label_offset);
            underlayer.SetFontSize(render_settings_.stop_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetData(stop->name);
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            result.push_back(underlayer);
            result.push_back(text);
        }

        return result;
    }

    svg::Document MapRenderer::GetSVG(const Buses& buses) const {
        svg::Document result;
        std::vector<geo::Coordinates> route_stops_coord;
        Stops all_stops;
        for (const auto& [bus_number, bus] : buses) {
            for (const auto& stop : bus->stops) {
                route_stops_coord.push_back(stop->coordinates);
                all_stops[stop->name] = stop;
            }
        }
        SphereProjector sp(route_stops_coord.begin(), route_stops_coord.end(), render_settings_.width, render_settings_.height, render_settings_.padding);

        for (const auto& line : GetRouteLines(buses, sp)) {
            result.Add(line);
        }

        for (const auto& text : GetBusLabel(buses, sp)) {
            result.Add(text);
        }

        for (const auto& circle : GetStopsSymbols(all_stops, sp)) {
            result.Add(circle);
        }

        for (const auto& text : GetStopsLabels(all_stops, sp)) {
            result.Add(text);
        }

        return result;
    }

} // namespace renderer