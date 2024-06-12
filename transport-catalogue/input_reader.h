#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace catalogue {
    namespace input {

        struct CommandDescription {
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;      // �������� �������
            std::string id;           // id �������� ��� ���������
            std::string description;  // ��������� �������
        };

        class InputReader {
        public:
            void ParseLine(std::string_view line);
            void ApplyCommands(TransportCatalogue& catalogue) const;

        private:
            std::vector<CommandDescription> stop_commands_;
            std::vector<CommandDescription> bus_commands_;
        };

    } //namespace input
} //namespace catalogue