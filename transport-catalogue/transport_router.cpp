#include "transport_router.h"
#include <stdexcept>

namespace transport_router {

	using namespace std::literals;
	using namespace transport_catalogue;

	TransportRouter::TransportRouter(const TransportCatalogue& catalogue, RouterSettings settings) {
		SetRouterSetting(settings);

		graph::DirectedWeightedGraph<RouteWeight> graph(CountStops(catalogue));
		for (const auto& [bus_id, route] : catalogue.GetBuses()) {
			std::vector<const Stop*> stops = route->stops;
			BuildGraph(graph, catalogue, stops, bus_id);
			if (!route->is_roundtrip) {
				std::vector<const Stop*> rstops{ route->stops.rbegin(), route->stops.rend() };
				BuildGraph(graph, catalogue, rstops, bus_id);
			}
		}
		graph_ = std::move(graph);
		router_ = std::make_unique<graph::Router<RouteWeight>>(graph::Router(graph_));
	}

	std::optional<RouteData> TransportRouter::BuildRouteData(std::string_view from, std::string_view to) const {
		if (!router_) {
			return std::nullopt;
		}

		const auto& route = router_->BuildRoute(stopname_to_id_.at(from), stopname_to_id_.at(to));
		if (!route) {
			return std::nullopt;
		}

		json::Array items;
		items.reserve(route.value().edges.size());

		for (const auto& edge : route->edges) {
			const auto& edge_info = graph_.GetEdge(edge);
			auto wait_time = settings_.bus_wait_time;

			items.emplace_back(json::Node(json::Builder{}
				.StartDict()
				.Key("stop_name"s).Value(std::string(id_to_stopname_.at(edge_info.from)))
				.Key("time"s).Value(wait_time)
				.Key("type"s).Value("Wait"s)
				.EndDict()
				.Build()
			));
			items.emplace_back(json::Node(json::Builder{}
				.StartDict()
				.Key("bus"s).Value(std::string(edge_info.weight.bus_name))
				.Key("span_count"s).Value(edge_info.weight.span_count)
				.Key("time"s).Value(edge_info.weight.total_time - wait_time)
				.Key("type"s).Value("Bus"s)
				.EndDict()
				.Build()
			));
		}
		return RouteData{ items, route->weight.total_time};
	}

	void TransportRouter::SetRouterSetting(RouterSettings settings) {
		settings_ = settings;
	}

	size_t TransportRouter::CountStops(const TransportCatalogue& catalogue) {
		size_t stops_counter = 0;
		const auto& stops = catalogue.GetStops();
		stopname_to_id_.reserve(stops.size());
		id_to_stopname_.reserve(stops.size());
		for (const auto& stop : stops) {
			stopname_to_id_.insert({ stop.first, stops_counter });
			id_to_stopname_.insert({ stops_counter++, stop.first });
		}
		return stops_counter;
	}

	double TransportRouter::ComputeRouteTime(const TransportCatalogue& catalogue, const Stop* from, const Stop* to) const {
		auto split_distance = catalogue.GetDistance(from, to);
		return split_distance / (settings_.bus_velocity * KPH_TO_MPM);
	}

	void TransportRouter::BuildGraph(
		graph::DirectedWeightedGraph<RouteWeight>& graph,
		const TransportCatalogue& catalogue,
		const std::vector<const Stop*>& stops,
		std::string_view bus_id
	) {
		for (size_t i = 0; i < stops.size() - 1; ++i) {
			double route_time = settings_.bus_wait_time;
			auto from = stops[i];
			int span_count = 1;
			for (size_t j = i + 1; j < stops.size(); ++j) {
				auto to = stops[j];
				route_time += ComputeRouteTime(catalogue, stops[j - 1], to);
				graph.AddEdge({ stopname_to_id_[from->name], stopname_to_id_[to->name], {bus_id, route_time, span_count++ } });
			}
		}
	}
	
	bool operator<(const RouteWeight& left, const RouteWeight& right) {
		return left.total_time < right.total_time;
	}

	bool operator>(const RouteWeight& left, const RouteWeight& right) {
		return left.total_time > right.total_time;
	}

	RouteWeight operator+(const RouteWeight& left, const RouteWeight& right) {
		RouteWeight result;
		result.total_time = left.total_time + right.total_time;
		return result;
	}

} //namespace transport_router