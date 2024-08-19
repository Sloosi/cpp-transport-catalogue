#pragma once

#include "transport_catalogue.h"
#include "router.h"

#include <memory>

namespace transport_router {

	// коэффициент перевода км/ч в м/мин
	constexpr static double KPH_TO_MPM = 1000.0 / 60.0;

	struct RouteWeight {
		std::string_view bus_name;
		double total_time = 0;
		int span_count = 0;
	};

	struct RouterSettings {
		double bus_wait_time = 0;
		double bus_velocity = 0;
	};

	class TransportRouter {
	public:
		TransportRouter() = default;
		TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, RouterSettings settings);

		std::optional <graph::Router<RouteWeight>::RouteInfo> BuildRouter(std::string_view from, std::string_view to) const;

		const graph::DirectedWeightedGraph<RouteWeight>& GetGraph() const;
		const std::unique_ptr<graph::Router<RouteWeight>>& GetRouter() const;

		const RouterSettings& GetRouterSettings() const;
		const std::string_view GetStopNameFromID(size_t id) const;

		void SetRouterSetting(RouterSettings settings);

	private:
		size_t CountStops(const transport_catalogue::TransportCatalogue& catalogue);

		double ComputeRouteTime(
			const transport_catalogue::TransportCatalogue& catalogue,
			const transport_catalogue::Stop* stop_from_index,
			const transport_catalogue::Stop* stop_to_index
		) const;

		void BuildGraph(
			graph::DirectedWeightedGraph<RouteWeight>& graph,
			const transport_catalogue::TransportCatalogue& catalogue,
			const std::vector<const transport_catalogue::Stop*>& stops,
			std::string_view bus_id
		);

		RouterSettings settings_;
		std::unordered_map<std::string_view, uint32_t> stopname_to_id_;
		std::unordered_map<uint32_t, std::string_view> id_to_stopname_;
		std::unique_ptr<graph::Router<RouteWeight>> router_ = nullptr;
		graph::DirectedWeightedGraph<RouteWeight> graph_;
	};

	bool operator<(const RouteWeight& left, const RouteWeight& right);
	bool operator>(const RouteWeight& left, const RouteWeight& right);
	RouteWeight operator+(const RouteWeight& left, const RouteWeight& right);
}