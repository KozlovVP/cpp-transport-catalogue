#pragma once

// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include <string>
#include <iostream>
#include <set>

void PrintBusInfo(std::string name, int total_size, int unique_size, double real_distance, double curv);

void PrintStopInfo(std::string name, const std::set<std::string>& buses);

void NotFoundBus(std::string name);

void NotFoundStop(std::string name);

