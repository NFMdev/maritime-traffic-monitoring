#pragma once

#include <optional>
#include "repository/position_repository.hpp"
#include "domain/position_report.hpp"


class PositionService {
public:
    explicit PositionService(PositionRepository& repository) : repository_(repository) {}

    void ingest(const PositionReport& report);

    std::optional<PositionReport> latest(long long mmsi);

    std::vector<PositionReport> history(
        long long mmsi, 
        std::chrono::system_clock::time_point from, 
        std::chrono::system_clock::time_point to
    );

private:
    PositionRepository& repository_;

    static void validateMmsi(long long mmsi);
    static void validateReport(const PositionReport& report);
    static void validateTimeRange(
        std::chrono::system_clock::time_point from, 
        std::chrono::system_clock::time_point to
    );
};