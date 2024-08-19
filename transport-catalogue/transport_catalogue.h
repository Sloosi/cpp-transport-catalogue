#pragma once

#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

	class TransportCatalogue {
	public:
		void AddStop(const std::string& name, geo::Coordinates coordinates);
		void AddRoute(const std::string& number, const std::vector<const Stop*>& stops, bool is_roundtrip);

		const Stop* FindStop(std::string_view stop) const;
		const Bus* FindRoute(std::string_view bus) const;

		double GetDistance(const Stop* a, const Stop* b) const;
		void SetDistance(const std::pair<const Stop*, const Stop*>& stops, int distance);

		const std::unordered_map<std::string_view, Bus*>& GetBuses() const;
		const std::unordered_map<std::string_view, Stop*>& GetStops() const;

		size_t UniqueStopsCount(const std::string& bus) const;
	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		std::unordered_map<std::string_view, Stop*> stops_as_catalogue_;
		std::unordered_map<std::string_view, Bus*> buses_as_catalogue_;

		std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> distances_;

	};

} //namespace catalogue