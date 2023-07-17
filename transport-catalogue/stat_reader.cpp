#include "stat_reader.h"

using namespace std;

void PrintBusInfo(ostream& out, const string& name, int total_size, int unique_size, double real_distance, double curv) {
    //ostringstream out;
    out << "Bus " << name << ": " << total_size << " stops on route, " << unique_size << " unique stops, " << real_distance << " route length, " << curv << " curvature" << endl;
    //string b = move(out).str();
    //cout << b;
}

void PrintStopInfo(ostream& out, const string& name, const set<string>& buses) {
    //ostringstream out;
    if (buses.size() == 0) {
        out << "Stop " << name << ": no buses" << endl;
        //string b = move(out).str();
        //cout << b;
        return;
    }

    out << "Stop " << name << ": buses";
    for (auto& bus : buses) {
        out << " " << bus;
    }
    out << endl;

    //string b = move(out).str();
    //cout << b;
}

void NotFoundBus(ostream& out, const string& name) {
    //ostringstream out;
    out << "Bus " << name << ": not found" << endl;
    //string b = move(out).str();
    //cout << b;
}

void NotFoundStop(ostream& out, const string& name) {
    //ostringstream out;
    out << "Stop " << name << ": not found" << endl;
    //string b = move(out).str();
    //cout << b;
}
