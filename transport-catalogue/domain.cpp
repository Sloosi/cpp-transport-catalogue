#include "domain.h"

namespace transport_catalogue {
	size_t StopPairHasher::operator()(const std::pair<const Stop*, const Stop*>& stops) const {
		size_t hash_first = std::hash<const void*>{}(stops.first);
		size_t hash_second = std::hash<const void*>{}(stops.second);
		return hash_first + hash_second * salt;
	}
} // namespace transport_catalogue