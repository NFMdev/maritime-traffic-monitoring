#pragma once

#include "domain/position_report.hpp"
#include <optional>
#include <vector>

class PositionRepository {
public:
    virtual ~PositionRepository() =  default;
    virtual void save(const PositionReport& report) = 0;
    virtual std::optional<PositionReport> findLatestByMmsi(long long mmsi) = 0;
    virtual std::vector<PositionReport> findByMmsiAndTimeRange(
        long long mmsi, 
        std::chrono::system_clock::time_point from, 
        std::chrono::system_clock::time_point to
    ) = 0;
};