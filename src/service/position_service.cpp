#include "service/position_service.hpp"
#include "util/app_exceptions.hpp"

#include <chrono>

// CRUD operations for position reports
PositionService::PositionService(PositionRepository& repository) : repository_(repository) {}

void PositionService::ingest(const PositionReport& report) {
    validateReport(report);
    repository_.save(report);
}

std::optional<PositionReport> PositionService::latest(long long mmsi) {
    validateMmsi(mmsi);
    return repository_.findLatestByMmsi(mmsi);
}

std::vector<PositionReport> PositionService::history(
    long long mmsi, 
    std::chrono::system_clock::time_point from, 
    std::chrono::system_clock::time_point to
) {
    validateMmsi(mmsi);
    validateTimeRange(from, to);
    return repository_.findByMmsiAndTimeRange(mmsi, from, to);
}

void PositionService::validateMmsi(long long mmsi) {
    if (mmsi <= 0) {
        throw ValidationException("Invalid MMSI: must be greater than 0.");
    }
}

// Validates the position report fields according to AIS standards
void PositionService::validateReport(const PositionReport& report) {
    validateMmsi(report.mmsi);

    if (report.latitude < -90.0 || report.latitude > 90.0) {
        throw ValidationException("Invalid latitude: must be between -90 and 90.");
    }

    if (report.longitude < -180.0 || report.longitude > 180.0) {
        throw ValidationException("Invalid longitude: must be between -180 and 180.");
    }

    if (report.sogKnots < 0.0) {
        throw ValidationException("Invalid SOG: must be positive.");
    }

    if (report.cogDegrees < 0.0 || report.cogDegrees >= 360.0) {
        throw ValidationException("Invalid COG: must be between 0 and 360 degrees.");
    }

    if (report.trueHeading < 0 || report.trueHeading >= 360) {
        throw ValidationException("Invalid true heading: must be between 0 and 360 degrees.");
    }

    auto maxFutureTime = std::chrono::system_clock::now() + std::chrono::minutes(5);

    if (report.aisTimestamp > maxFutureTime) {
        throw ValidationException("Invalid timestamp: cannot be more than 5 minutes in the future.");
    }
}

void PositionService::validateTimeRange(
    std::chrono::system_clock::time_point from, 
    std::chrono::system_clock::time_point to
) {
    if (from > to) {
        throw ValidationException("Invalid time range: 'from' time must be before 'to' time.");
    }
}