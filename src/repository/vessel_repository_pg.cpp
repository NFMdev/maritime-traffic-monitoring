#include "repository/vessel_repository_pg.hpp"

#include "util/app_exceptions.hpp"
#include "util/time_utils.hpp"

#include <pqxx/pqxx>
#include <string>

VesselRepositoryPg::VesselRepositoryPg(const PostgresPool& pool) : pool_(pool) {}

void VesselRepositoryPg::save(const Vessel& vessel) {
    try {
        auto conn = pool_.acquire();
        pqxx::work tx(*conn);

        pqxx::params params;
        params.append(vessel.mmsi);

        appendParam(params, vessel.imo);
        appendParam(params, vessel.name);
        appendParam(params, vessel.callsign);
        appendParam(params, vessel.vesselType);
        appendParam(params, vessel.lengthM);
        appendParam(params, vessel.widthM);
        appendParam(params, vessel.destination);
        appendParam(params, vessel.draughtM);

        if (vessel.eta) params.append(time_utils::to_pg_timestamp(*vessel.eta));
        else params.append();

        tx.exec(
            R"(
                INSERT INTO vessels (mmsi, imo, vessel_name, callsign, vessel_type, length_m,
                width_m, destination, eta, draught_m, updated_at)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, NOW())
                ON CONFLICT (mmsi)
                DO UPDATE SET
                    imo = EXCLUDED.imo,
                    vessel_name = EXCLUDED.vessel_name,
                    callsign = EXCLUDED.callsign,
                    vessel_type = EXCLUDED.vessel_type,
                    length_m = EXCLUDED.length_m,
                    width_m = EXCLUDED.width_m,
                    destination = EXCLUDED.destination,
                    eta = EXCLUDED.eta,
                    draught_m = EXCLUDED.draught_m,
                    updated_at = NOW()
            )",
            params
        );

        tx.commit();
    } catch (const pqxx::sql_error& e) {
        throw DatabaseException(std::string("Database SQL error while upserting vessel: ") + e.what());
    } catch (const pqxx::broken_connection& e) {
        throw DatabaseException(std::string("Database connection error while upserting vessel: ") + e.what());
    } catch (const pqxx::failure& e) {
        throw DatabaseException(std::string("Database failure while upserting vessel: ") + e.what());
    } catch (const std::exception& e) {
        throw DatabaseException(std::string("Unexpected database error while upserting vessel: ") + e.what());
    }
}

std::optional<Vessel> VesselRepositoryPg::findByMmsi(long long mmsi) {
    try {
        auto conn = pool_.acquire();
        pqxx::read_transaction tx(*conn);

        const pqxx::result result = tx.exec(
            R"(
               SELECT mmsi, imo, vessel_name, callsign, vessel_type, length_m, 
               width_m, destination, eta, draught_m, updated_at
               FROM vessels WHERE mmsi = $1
            )",
            mmsi
        );

        if  (result.empty()) {
            return std::nullopt;
        }

        return mapToVessel(result[0]);
    } catch (const std::exception &e) {
        throw DatabaseException(std::string("Unexpected database error while finding vessel: ") + e.what());
    } catch (const pqxx::sql_error &e) {
        throw DatabaseException(std::string("Database SQL error while retrieving vessel: ") + e.what());
    } catch (const pqxx::broken_connection &e) {
        throw DatabaseException(std::string("Database connection error while retrieving vessel: ") + e.what());
    } catch (const pqxx::failure &e) {
        throw DatabaseException(std::string("Database failure while retrieving vessel: ") + e.what());
    }
}

Vessel VesselRepositoryPg::mapToVessel(const pqxx::row& row) {
    return Vessel {
        .mmsi = row["mmsi"].as<long long>(),
        .imo = readOptional<long long>(row, "imo"),
        .name = readOptional<std::string>(row, "vessel_name"),
        .callsign = readOptional<std::string>(row, "callsign"),
        .vesselType = readOptional<int>(row, "vessel_type"),
        .lengthM = readOptional<double>(row, "length_m"),
        .widthM = readOptional<double>(row, "width_m"),
        .destination = readOptional<std::string>(row, "destination"),
        .eta = row["eta"].is_null() ? std::nullopt : std::make_optional(time_utils::from_pg_timestamp(row["eta"].c_str())),
        .draughtM = readOptional<double>(row, "draught_m"),
        .updatedAt = time_utils::from_pg_timestamp(row["updated_at"].c_str())
    };
}

template <typename T>
std::optional<T> VesselRepositoryPg::readOptional(const pqxx::row& row, const char* column) {
    if (row[column].is_null()) {
        return std::nullopt;
    }
    return row[column].as<T>();
}

template std::optional<long long> VesselRepositoryPg::readOptional<long long>(const pqxx::row&, const char*);
template std::optional<std::string> VesselRepositoryPg::readOptional<std::string>(const pqxx::row&, const char*);
template std::optional<int> VesselRepositoryPg::readOptional<int>(const pqxx::row&, const char*);
template std::optional<double> VesselRepositoryPg::readOptional<double>(const pqxx::row&, const char*);

template <typename T>
void VesselRepositoryPg::appendParam(pqxx::params& parms, const std::optional<T>& value) {
    if (value) parms.append(*value);
    else parms.append();
}