#pragma once

#include "repository/vessel_repository.hpp"
#include "db/postgres_pool.hpp"

#include <pqxx/pqxx>
#include <optional>

class VesselRepositoryPg final : public VesselRepository {
public:
    explicit VesselRepositoryPg(const PostgresPool& pool);

    void save(const Vessel& vessel);

    std::optional<Vessel> findByMmsi(long long mmsi) override;

private:
    const PostgresPool& pool_;

    static Vessel mapToVessel(const pqxx::row& row);

    template <typename T>
    static std::optional<T> readOptional(const pqxx::row& row, const char* column);
    template <typename T>
    static void appendParam(pqxx::params& params, const std::optional<T>& value);
};