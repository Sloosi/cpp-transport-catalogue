#pragma once

#include <iostream>
#include <string_view>

#include "transport_catalogue.h"

namespace catalogue {
    namespace output {

        struct CommandDescription {
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;      // Название команды
            std::string description;  // Параметры команды
        };

        CommandDescription ParseCommandDescription(std::string_view request);

        class RequestsHandler {
        public:
            RequestsHandler(const TransportCatalogue& catalogue, std::istream& input, std::ostream& output);

            void HandelStopRequest(const std::string& description);
            void HandelBusRequest(const std::string& description);

            void ParseAndHandleRequest(std::string_view request);

            void HandleAllRequests();
        private:
            const TransportCatalogue& catalogue_;
            std::istream& input_;
            std::ostream& output_;
        };
    } //namespace output
} //namespace catalogue