#include "transport_catalogue.h"

namespace transport_catalogue {
	void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
		stops_.push_back({ name, coordinates, {} });
		stops_as_catalogue_.insert({ stops_.back().name, &stops_.back() });
	}

	void TransportCatalogue::AddRoute(const std::string& number, const std::vector<const Stop*>& stops, bool is_roundtrip) {
		buses_.push_back({ number, stops, is_roundtrip });
		buses_as_catalogue_.insert({ buses_.back().number, &buses_.back() });

		for (const auto& stop : stops) {
			stops_as_catalogue_.at(stop->name)->buses.insert(number);
		}
	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop) const {
		return stops_as_catalogue_.count(stop) ? stops_as_catalogue_.at(stop) : nullptr;
	}

	const Bus* TransportCatalogue::FindRoute(std::string_view bus) const {
		return buses_as_catalogue_.count(bus) ? buses_as_catalogue_.at(bus) : nullptr;
	}


	void TransportCatalogue::SetDistance(const std::pair<const Stop*, const Stop*>& stops,
		int distance) {
		distances_[stops] = distance;
	}

	double TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
		if (distances_.count({ from, to })) {
			return distances_.at({ from, to });
		}

		if (distances_.count({ to, from })) {
			return distances_.at({ to, from });
		}

		return 0;
	}

	const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetBuses() const {
		return buses_as_catalogue_;
	}

	const std::unordered_map<std::string_view, Stop*>& TransportCatalogue::GetStops() const {
		return stops_as_catalogue_;
	}

	size_t TransportCatalogue::UniqueStopsCount(const std::string& bus) const {
		std::unordered_set<std::string_view> unique_stops;
		for (const auto& stop : buses_as_catalogue_.at(bus)->stops) {
			unique_stops.insert(stop->name);
		}
		return unique_stops.size();
	}

} //namespace catalogue