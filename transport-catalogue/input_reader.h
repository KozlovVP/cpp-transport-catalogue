#pragma once
// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include <string>
#include <deque>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>
#include <utility>

#include "transport_catalogue.h"

using dis_container = std::map<std::string, std::deque<std::pair<double, std::string>>>;

void OrderAddBusLess(TransportCatalogue& catalogue, std::string& command);
void OrderAddBusDash(TransportCatalogue& catalogue, std::string& command);
void OrderAddStop(TransportCatalogue& catalogue, std::string& command, dis_container& d_deque);

void OrderGetBusInfo(TransportCatalogue& catalogue, std::string& command);
void OrderGetStopInfo(TransportCatalogue& catalogue, std::string& command);

void Read(std::istream& input);
