#include "transport_catalogue.h"

namespace catalogue {
	size_t StopPairHasher::operator()(const std::pair<const Stop*, const Stop*>& stops) const {
		size_t hash_first = std::hash<const void*>{}(stops.first);
		size_t hash_second = std::hash<const void*>{}(stops.second);
		return hash_first + hash_second * salt;
	}

	void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
		stops_.push_back({ name, coordinates, {} });
		stops_as_catalogue_.insert({ stops_.back().name, &stops_.back() });
	}

	void TransportCatalogue::AddRoute(const std::string& number, const std::vector<std::string>& stops) {
		buses_.push_back({ number, stops });
		buses_as_catalogue_.insert({ buses_.back().number, &buses_.back() });

		for (const auto& stop : stops) {
			stops_as_catalogue_.at(stop)->buses.insert(number);
		}
	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop) const {
		return stops_as_catalogue_.count(stop) ? stops_as_catalogue_.at(stop) : nullptr;
	}

	const Bus* TransportCatalogue::FindBus(std::string_view bus) const {
		return buses_as_catalogue_.count(bus) ? buses_as_catalogue_.at(bus) : nullptr;
	}


	void TransportCatalogue::AddDistance(const std::pair<const Stop*, const Stop*>& stops,
		int distance) {
		distances_[stops] = distance;
	}

	double TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
		auto it = distances_.find({ from, to });

		if (it == distances_.end()) {
			it = distances_.find({ to, from });
			if (it == distances_.end()) {
				return 0;
			}
		}

		return it->second;
	}

	BusInfo TransportCatalogue::GetBusInfo(std::string_view bus) const {
		BusInfo bus_info = {};

		const Bus* bus_ptr = FindBus(bus);
		if (bus_ptr == nullptr) {
			throw std::invalid_argument("Bus " + std::string(bus) + ": not found");
		}

		{ //calculate total length
			double length = 0.0;
			for (auto it = bus_ptr->stops.begin(); it + 1 != bus_ptr->stops.end(); ++it) {
				length += ComputeDistance(
					stops_as_catalogue_.at(*it)->coordinates,
					stops_as_catalogue_.at(*(it + 1))->coordinates
				);
			}
			bus_info.geo_route_length = length;
		}

		{	//calculate fact length
			size_t fact_length = 0;
			for (size_t i = 0; i < bus_ptr->stops.size() - 1; ++i) {
				fact_length += GetDistance(FindStop(bus_ptr->stops[i]), FindStop(bus_ptr->stops[i + 1]));
			}
			bus_info.route_length = fact_length;
		}

		bus_info.stops_count = bus_ptr->stops.size();
		bus_info.unique_stops_count = UniqueStopsCount(std::string(bus));
		bus_info.curvature = static_cast<double>(bus_info.route_length) / bus_info.geo_route_length;

		return bus_info;
	}

	std::set<std::string> TransportCatalogue::GetBusesByStop(std::string_view stop) const {
		if (stops_as_catalogue_.count(stop) == 0) {
			throw std::invalid_argument("Stop " + std::string(stop) + ": not found");
		}

		return stops_as_catalogue_.at(stop)->buses;
	}

	size_t TransportCatalogue::UniqueStopsCount(const std::string& bus) const {
		std::unordered_set<std::string_view> unique_stops;
		for (const auto& stop : buses_as_catalogue_.at(bus)->stops) {
			unique_stops.insert(stop);
		}
		return unique_stops.size();
	}

} //namespace catalogue