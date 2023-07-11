#include "stat_reader.h"

using namespace std;

void PrintBusInfo(string& name, int total_size, int unique_size, double real_distance, double curv) {
    cout << "Bus " << name << ": " << total_size << " stops on route, " << unique_size << " unique stops, " << real_distance << " route length, " << curv << " curvature" << endl;
}

void PrintStopInfo(string& name, const set<string>& buses) {
    if (buses.size() == 0) {
        cout << "Stop " << name << ": no buses" << endl;
        return;
    }

    cout << "Stop " << name << ": buses";
    for (auto& bus : buses) {
        cout << " " << bus;
    }
    cout << endl;
}

void NotFoundBus(string name) {
    cout << "Bus " << name << ": not found" << endl;
}

void NotFoundStop(string name) {
    cout << "Stop " << name << ": not found" << endl;
}
