#pragma once

// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include <string>
#include <iostream>
#include <set>
#include <sstream>

void PrintBusInfo(std::ostream& out, const std::string& name, int total_size, int unique_size, double real_distance, double curv);

void PrintStopInfo(std::ostream& out, const std::string& name, const std::set<std::string>& buses);

void NotFoundBus(std::ostream& out, const std::string& name);

void NotFoundStop(std::ostream& out, const std::string& name);

