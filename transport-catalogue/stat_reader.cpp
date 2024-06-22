#include "stat_reader.h"

namespace catalogue {
    namespace output {

        CommandDescription ParseCommandDescription(std::string_view request) {
            auto command_start = request.find_first_not_of(' ');
            auto command_end = request.find_first_of(' ', command_start);
            auto description_start = request.find_first_not_of(' ', command_end);
            auto description_end = request.find_last_not_of(' ');
            
            return {
                std::string(request.substr(command_start, command_end - command_start)),
                std::string(request.substr(description_start, description_end - description_start + 1))
            };
        }

        RequestsHandler::RequestsHandler(const TransportCatalogue& catalogue, std::istream& input, std::ostream& output) :
            catalogue_(catalogue),
            input_(input),
            output_(output)
        {}

        void RequestsHandler::HandelStopRequest(const std::string& description) {
            try
            {
                auto buses = catalogue_.GetBusesByStop(description);

                if (buses.empty()) {
                    output_ <<  "Stop " << description << ": " << "no buses" << std::endl;
                    return;
                }

                output_ << "Stop " << description << ": buses ";
                for (const auto& bus : buses) {
                    output_ << bus << " ";
                }
                output_ << std::endl;
            }
            catch (const std::invalid_argument& e)
            {
                output_ << e.what() << std::endl;
            }
        }
        

        void RequestsHandler::HandelBusRequest(const std::string& description) {
            try {
                BusInfo bus_info = catalogue_.GetBusInfo(description);

                output_ << "Bus " << description << ": "
                    << bus_info.stops_count << " stops on route, "
                    << bus_info.unique_stops_count << " unique stops, "
                    << bus_info.route_length << " route length, "
                    << bus_info.curvature << " curvature" << std::endl;
            }
            catch (const std::invalid_argument& e) {
                output_ << e.what() << std::endl;
            }
        }

        void RequestsHandler::ParseAndHandleRequest(std::string_view request) {
            if (request.empty()) {
                return;
            }

            const CommandDescription& command_description = ParseCommandDescription(request);

            if (command_description.command == "Stop") {
                HandelStopRequest(command_description.description);
            }
            else if (command_description.command == "Bus") {
                HandelBusRequest(command_description.description);
            }
        }

        void RequestsHandler::HandleAllRequests() {
            int stat_request_count;
            input_ >> stat_request_count >> std::ws;
            for (int i = 0; i < stat_request_count; ++i) {
                std::string line;
                std::getline(input_, line);
                ParseAndHandleRequest(line);
            }
        }

    } //namespace output
} //namespace catalogue