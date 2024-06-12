#include "transport_catalogue.h"

namespace catalogue {

	void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
		stops_.push_back({ name, coordinates, {} });
		stops_as_catalogue_.insert({ stops_.back().name, &stops_.back() });
	}

	void TransportCatalogue::AddBus(const std::string& number, std::vector<std::string> stops) {
		buses_.push_back({ number, stops });
		buses_as_catalogue_.insert({ buses_.back().number, &buses_.back() });

		for (const auto& stop : stops) {
			stops_as_catalogue_.at(stop)->buses.insert(number);
		}
	}

	const Stop* TransportCatalogue::FindStop(const std::string& stop) const {
		return stops_as_catalogue_.count(stop) ? stops_as_catalogue_.at(stop) : nullptr;
	}

	const Bus* TransportCatalogue::FindBus(const std::string& bus) const {
		return buses_as_catalogue_.count(bus) ? buses_as_catalogue_.at(bus) : nullptr;
	}

	std::set<std::string> TransportCatalogue::GetBusesByStop(const std::string& stop) const {
		if (stops_as_catalogue_.count(stop) == 0) {
			throw std::invalid_argument("Stop " + stop + ": not found");
		}

		return stops_as_catalogue_.at(stop)->buses;
	}

	BusInfo TransportCatalogue::GetBusInfo(const std::string& bus) const {
		BusInfo bus_info = {};

		const Bus* bus_ptr = FindBus(bus);
		if (bus_ptr == nullptr) {
			throw std::invalid_argument("Bus " + bus + ": not found");
		}

		{ //calculate total length
			double length = 0.0;
			for (auto it = bus_ptr->stops.begin(); it + 1 != bus_ptr->stops.end(); ++it) {
				length += ComputeDistance(
					stops_as_catalogue_.at(*it)->coordinates,
					stops_as_catalogue_.at(*(it + 1))->coordinates
				);
			}
			bus_info.route_length = length;
		}

		bus_info.stops_count = bus_ptr->stops.size();
		bus_info.unique_stops_count = UniqueStopsCount(bus);

		return bus_info;
	}

	size_t TransportCatalogue::UniqueStopsCount(const std::string& bus) const {
		std::unordered_set<std::string_view> unique_stops;
		for (const auto& stop : buses_as_catalogue_.at(bus)->stops) {
			unique_stops.insert(stop);
		}
		return unique_stops.size();
	}

} //namespace catalogue