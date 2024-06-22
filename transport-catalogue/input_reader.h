#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace catalogue::input {

    struct CommandDescription {
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // Название команды
        std::string id;           // id маршрута или остановки
        std::string description;  // Параметры команды
    };

    class InputReader {
    public:
        void Read(std::istream& input);
        void ParseLine(std::string_view line);

        void ApplyCommands(TransportCatalogue& catalogue) const;


    private:
        std::vector<CommandDescription> stop_commands_;
        std::vector<CommandDescription> bus_commands_;

        void SetStops(TransportCatalogue& catalogue) const;
        void SetStopsDistances(TransportCatalogue& catalogue) const;
        void SetRoutes(TransportCatalogue& catalogue) const;
    };

} //namespace catalogue::input