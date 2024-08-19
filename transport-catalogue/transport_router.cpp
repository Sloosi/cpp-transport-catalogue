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

	std::optional <graph::Router<RouteWeight>::RouteInfo> TransportRouter::BuildRouter(std::string_view from, std::string_view to) const {
		if (!router_) {
			return std::nullopt;
		}
		return router_->BuildRoute(stopname_to_id_.at(from), stopname_to_id_.at(to));
	}

	const graph::DirectedWeightedGraph<RouteWeight>& TransportRouter::GetGraph() const {
		return graph_;
	}

	const std::unique_ptr<graph::Router<RouteWeight>>& TransportRouter::GetRouter() const {
		return router_;
	}

	const RouterSettings& TransportRouter::GetRouterSettings() const {
		return settings_;
	}

	const std::string_view TransportRouter::GetStopNameFromID(size_t id) const {
		return id_to_stopname_.at(id);
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