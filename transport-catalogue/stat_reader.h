#pragma once

// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include <string>
#include <iostream>
#include <set>
#include <sstream>

void PrintBusInfo(const std::string& name, int total_size, int unique_size, double real_distance, double curv);

void PrintStopInfo(const std::string& name, const std::set<std::string>& buses);

void NotFoundBus(const std::string& name);

void NotFoundStop(const std::string& name);

