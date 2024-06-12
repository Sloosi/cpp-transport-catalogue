#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace catalogue;

int main() {
    TransportCatalogue catalogue;

    input::InputReader reader;
    reader.Read(cin);
    reader.ApplyCommands(catalogue);

    output::RequestsHandler requests_handler(catalogue, cin, cout);
    requests_handler.HandleAllRequests();
}