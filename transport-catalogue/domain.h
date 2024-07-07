#pragma once

#include "geo.h"

#include <string>
#include <set>
#include <vector>

namespace transport_catalogue {
	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
		std::set<std::string> buses;
	};

	struct Bus {
		std::string number;
		std::vector<const Stop*> stops;
		bool is_roundtrip = false;
	};

	struct BusInfo {
		size_t stops_count = 0;
		size_t unique_stops_count = 0;
		double route_length = 0.0;
		double geo_route_length = 0.0;
		double curvature = 0.0;
	};

	struct StopPairHasher {
		static const size_t salt = 37;

		size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const;
	};
} // namespace transport_catalogue