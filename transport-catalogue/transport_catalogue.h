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

namespace catalogue {

	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
		std::set<std::string> buses;
	};

	struct Bus {
		std::string number;
		std::vector<std::string> stops;
	};

	struct BusInfo {
		size_t stops_count = 0;
		size_t unique_stops_count = 0;
		double route_length = 0.0;
	};

	class TransportCatalogue {
	public:
		void AddStop(const std::string& name, geo::Coordinates coordinates);
		void AddBus(const std::string& number, const std::vector<std::string>& stops);

		const Stop* FindStop(std::string_view stop) const;
		const Bus* FindBus(std::string_view bus) const;

		BusInfo GetBusInfo(std::string_view bus) const;
		std::set<std::string> GetBusesByStop(std::string_view stop) const;
	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		std::unordered_map<std::string_view, Stop*> stops_as_catalogue_;
		std::unordered_map<std::string_view, Bus*> buses_as_catalogue_;

		size_t UniqueStopsCount(const std::string& bus) const;
	};

} //namespace catalogue