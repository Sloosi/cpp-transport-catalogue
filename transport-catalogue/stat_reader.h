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

            std::string command;      // �������� �������
            std::string description;  // ��������� �������
        };

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

    } //namespace output
} //namespace catalogue