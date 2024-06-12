#include "stat_reader.h"

namespace catalogue {
    namespace output {

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {
            if (request.empty()) {
                return;
            }

            auto command_start = request.find_first_not_of(' ');
            auto command_end = request.find_first_of(' ', command_start);
            auto description_start = request.find_first_not_of(' ', command_end);
            auto description_end = request.find_last_not_of(' ');

            CommandDescription command_description = {
                std::string(request.substr(command_start, command_end - command_start)),
                std::string(request.substr(description_start, description_end - description_start + 1))
            };

            if (command_description.command == "Bus") {
                try {
                    BusInfo bus_info = tansport_catalogue.GetBusInfo(command_description.description);

                    output << command_description.command << " " << command_description.description << ": "
                        << bus_info.stops_count << " stops on route, "
                        << bus_info.unique_stops_count << " unique stops, "
                        << bus_info.route_length << " route length" << std::endl;
                }
                catch (const std::invalid_argument& e) {
                    output << e.what() << std::endl;
                }
            }
            else if (command_description.command == "Stop") {
                try
                {
                    auto buses = tansport_catalogue.GetBusesByStop(command_description.description);

                    if (buses.empty()) {
                        output << command_description.command << " " << command_description.description << ": " << "no buses" << std::endl;
                        return;
                    }

                    output << command_description.command << " " << command_description.description << ": buses ";

                    for (const auto& bus : buses) {
                        output << bus << " ";
                    }
                    output << std::endl;
                }
                catch (const std::invalid_argument& e)
                {
                    output << e.what() << std::endl;
                }
            }
        }

    } //namespace output
} //namespace catalogue