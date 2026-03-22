#pragma once

#include <string>
#include <chrono>
#include <optional>

struct Vessel {
    long long mmsi;
    std::optional<long long> imo;
    std::optional<std::string> name;
    std::optional<std::string> callsign;
    std::optional<int> vesselType;
    std::optional<double> lengthM;
    std::optional<double> widthM;
    std::optional<std::string> destination;
    std::optional<std::chrono::system_clock::time_point> eta;
    std::optional<double> draughtM;
    std::chrono::system_clock::time_point updatedAt;
};