#include "input_reader.h"

#include <cassert>
#include <iterator>

namespace catalogue::input {
    /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
    geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == std::string_view::npos) {
            return { nan, nan };
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        const double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        const double lng = std::stod(std::string(str.substr(not_space2)));

        return { lat, lng };
    }

    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == std::string_view::npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == std::string_view::npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит строку вида "1000m to stop1, 536m to stop2, ..." и возвращает словарь дистанций (остановка, расстояние до остановки)
     */
    std::unordered_map<std::string, double> ParseDistances(std::string_view str) {
        std::unordered_map<std::string, double> distances;

        str = Trim(str);

        auto distances_to_parse = Split(str, ',');

        for (auto distance : distances_to_parse) {
            distance = Trim(distance);

            auto first_space = distance.find(' ');
            auto stop_begin = distance.find_first_not_of(' ', first_space);
            stop_begin = distance.find_first_not_of(' ', stop_begin + 2);

            auto stop = distance.substr(stop_begin);

            distances[std::string(stop)] = std::stod(std::string(distance.substr(0, first_space - 1)));
        }

        return distances;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {        
        if (route.find('>') != std::string_view::npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == std::string_view::npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return { std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1)) };
    }

    void InputReader::Read(std::istream& input) {
        int base_request_count = 0;
        input >> base_request_count >> std::ws;

        for (int i = 0; i < base_request_count; ++i) {
            std::string line;
            std::getline(input, line);
            ParseLine(line);
        }
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            if (command_description.command == "Stop") {
                stop_commands_.push_back(std::move(command_description));
            }
            else if (command_description.command == "Bus") {
                bus_commands_.push_back(std::move(command_description));
            }
        }
    }

    void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
        SetStops(catalogue);
        SetStopsDistances(catalogue);
        SetRoutes(catalogue);
    }

    void InputReader::SetStops(TransportCatalogue& catalogue) const {
        for (const auto& command : stop_commands_) {
            auto comma2 = command.description.find(',', command.description.find(',') + 1);
            auto coord_end = comma2 == std::string::npos ? command.description.size() : comma2;
            
            catalogue.AddStop(command.id, ParseCoordinates(command.description.substr(0, coord_end)));
        }
    }

    void InputReader::SetStopsDistances(TransportCatalogue& catalogue) const {
        for (const auto& command : stop_commands_) {
            auto comma2 = command.description.find(',', command.description.find(',') + 1);

            if (comma2 == std::string::npos) {
                continue;
            }

            auto distances = ParseDistances(command.description.substr(comma2 + 1));
            for (const auto& [stop, distance] : distances) {
                auto from = catalogue.FindStop(command.id);
                auto to = catalogue.FindStop(stop);

                catalogue.AddDistance({ from, to }, distance);
            }
        }
    }

    void InputReader::SetRoutes(TransportCatalogue& catalogue) const {
        for (const auto& command_description : bus_commands_) {
            std::vector<std::string> route;
            for (auto stop : ParseRoute(command_description.description)) {
                route.emplace_back(std::string(stop));
            }
            catalogue.AddRoute(command_description.id, route);
        }
    }

} //namespace catalogue::input