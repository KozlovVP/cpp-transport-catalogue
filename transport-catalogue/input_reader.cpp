#include "input_reader.h"

using namespace std;

using dis_container = map<string, deque<pair<double, string>>>;

bool IsNumber(const string& s)
{
    for (char const& ch : s) {
        if (std::isdigit(ch) == 0)
            return false;
    }
    return true;
}

string DeleteExtraSpaces(string& segment) { // удаляет лишние пробелы рядом с "-", ">"
    if (*segment.begin() == ' ') {
        segment = segment.substr(1, segment.size());
    }
    if (*segment.rbegin() == ' ') {
        segment = segment.substr(0, segment.size() - 1);
    }
    return segment;
}

pair<double, string> ParseDistance(string& command) { // обработка строки вида "100m to Marushkino"
    auto it = command.find('m');
    auto distance = stod(command.substr(0, it));

    it = command.find('o');
    command = command.substr(it + 1, command.size());
    auto name = DeleteExtraSpaces(command);

    //cout << distance << ' ' << name << endl;

    return { distance, name };
}

void OrderAddStop(TransportCatalogue& catalogue, string& command, dis_container& d_deque) {
    auto it = command.find(':');
    string name = command.substr(5, it - 5);
    name = DeleteExtraSpaces(name);

    auto m = command.find_first_not_of(' ', it + 1);
    command = command.substr(m, command.size());
    auto g = command.find(',');
    double lt = stod(command.substr(0, g));

    auto h = command.find_first_not_of(' ', g + 1);
    double lg = stod(command.substr(h, command.size()));

    it = command.find(',');
    command = command.substr(it + 2, command.size());
    it = command.find(',');
    command = command.substr(it + 2, command.size());

    if (command.find('m') == std::string::npos) {
        catalogue.AddStop(name, lt, lg);
        return;
    }

    stringstream test(command);
    string segment;

    while (std::getline(test, segment, ','))
    {
        string temp = DeleteExtraSpaces(segment);
        auto para = ParseDistance(temp);

        d_deque[name].push_back(para);
        //cout << temp << endl;
    }

    catalogue.AddStop(name, lt, lg);


}

void OrderAddBusLess(TransportCatalogue& catalogue, string& command) {
    auto it = command.find(':');
    string* name = new string(command.substr(4, it - 4));
    *name = DeleteExtraSpaces(*name);

    auto m = command.find_first_not_of(' ', it + 1);
    command = command.substr(m, command.size());

    stringstream test(command);
    string segment;
    vector<std::string> seglist;

    while (std::getline(test, segment, '>'))
    {
        string temp = DeleteExtraSpaces(segment);
        seglist.push_back(temp);
    }

    catalogue.AddBus(*name, seglist);
}

void OrderAddBusDash(TransportCatalogue& catalogue, string& command) {
    auto it = command.find(':');
    string* name = new string(command.substr(4, it - 4));
    *name = DeleteExtraSpaces(*name);

    auto m = command.find_first_not_of(' ', it + 1);
    command = command.substr(m, command.size());

    stringstream test(command);
    string segment;
    vector<std::string> seglist;

    while (std::getline(test, segment, '-'))
    {
        string temp = DeleteExtraSpaces(segment);
        seglist.push_back(temp);
    }

    int const size = seglist.size() - 2;

    for (auto it = size; it >= 0; it--) {
        seglist.push_back(seglist[it]);
    }

    catalogue.AddBus(*name, seglist);
}

void OrderGetBusInfo(TransportCatalogue& catalogue, string& command) {
    command = command.substr(3, command.size());
    command = DeleteExtraSpaces(command);
    catalogue.GetBusInfo(command);
}

void OrderGetStopInfo(TransportCatalogue& catalogue, string& command) {
    command = command.substr(4, command.size());
    command = DeleteExtraSpaces(command);
    catalogue.GetStopInfo(command);
}

void Read(std::istream& input) {
    TransportCatalogue catalogue;
    string command;

    dis_container add_distance_deque;
    
    while (getline(input, command)) {
        if (IsNumber(command)) {

            deque<string> add_bus_deque;

            int c = stoi(command);
            for (int i = 0; i < c; i++) {
                getline(input, command);
                auto it = command.find_first_not_of(' ');
                command.erase(0, it);

                //AddStop
                if (command.find(':') != std::string::npos && command.substr(0, 4) == "Stop") {
                    OrderAddStop(catalogue, command, add_distance_deque);
                }

                //AddBus
                else if (command.find(':') != std::string::npos && command.substr(0, 3) == "Bus") {
                    add_bus_deque.push_back(command);
                }

                else if (command.substr(0, 3) == "Bus") {
                    OrderGetBusInfo(catalogue, command);
                }

                else if (command.substr(0, 4) == "Stop") {
                    OrderGetStopInfo(catalogue, command);
                }
            }

            for (auto& [name, para_deque] : add_distance_deque) {
                for (auto& [distance, name2] : para_deque) {
                    catalogue.AddDistance(name, distance, name2);
                }
            }

            for (auto& command : add_bus_deque) {
                if (command.find('-') != std::string::npos) {
                    OrderAddBusDash(catalogue, command);
                }
                else { OrderAddBusLess(catalogue, command); }
            }
        }
    }

    
}
