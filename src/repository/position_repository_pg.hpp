#pragma once

#include "repository/position_repository.hpp"
#include "db/postgres_pool.hpp"

#include <pqxx/pqxx>
#include <optional>
#include <vector>

class PositionRepositoryPg final : public PositionRepository {
public:
    explicit PositionRepositoryPg(const PostgresPool& pool);

    void save(const PositionReport& report) override;

    std::optional<PositionReport> findLatestByMmsi(long long mmsi) override;

    std::vector<PositionReport> findByMmsiAndTimeRange(
        long long mmsi,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to
    ) override;

private:
    const PostgresPool& pool_;

    static PositionReport mapToPositionReport(const pqxx::row& row);

    template <typename T>
    static std::optional<T> readOptional(const pqxx::row& row, const char* column);
};