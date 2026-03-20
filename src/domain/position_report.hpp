#pragma once

#include <optional>
#include <chrono>

struct PositionReport {
    long long mmsi;
    double latitude;
    double longitude;
    double sogKnots;
    double cogDegrees;
    double trueHeading;
    std::optional<int> navStatus;
    std::chrono::system_clock::time_point aisTimestamp;
};