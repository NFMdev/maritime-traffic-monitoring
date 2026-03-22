#include "position_repository_pg.hpp"
#include "db/postgres_pool.hpp"
#include "util/time_utils.hpp"
#include "util/app_exceptions.hpp"

#include <pqxx/pqxx>
#include <memory>
#include <sstream>
#include <iomanip>
#include <stdexcept>

PositionRepositoryPg::PositionRepositoryPg(const PostgresPool& pool) : pool_(pool) {}

void PositionRepositoryPg::save(const PositionReport& report) {
    try {
        auto conn = pool_.acquire();
        pqxx::work tx(*conn);

        tx.exec(
            R"(
                INSERT INTO vessels (mmsi, updated_at)
                VALUES ($1, NOW())
                ON CONFLICT (mmsi)
                DO UPDATE SET updated_at = NOW()
            )",
            report.mmsi);

        tx.exec(
            R"(
                INSERT INTO position_reports (mmsi, latitude, longitude, sog_knots, cog_degrees,
                true_heading, navigational_status, ais_timestamp)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
            )",
            pqxx::params{
                report.mmsi,
                report.latitude,
                report.longitude,
                report.sogKnots,
                report.cogDegrees,
                report.trueHeading,
                report.navStatus,
                time_utils::to_pg_timestamp(report.aisTimestamp)
            }
        );

        tx.commit();
    } catch (const std::exception &e) {
        throw DatabaseException(std::string("Unexpected database error while saving position report: ") + e.what());
    } catch (const pqxx::sql_error &e) {
        throw DatabaseException(std::string("Database SQL error while saving position report: ") + e.what());
    } catch (const pqxx::broken_connection &e) {
        throw DatabaseException(std::string("Database connection error while saving position report: ") + e.what());
    } catch (const pqxx::failure &e) {
        throw DatabaseException(std::string("Database failure while saving position report: ") + e.what());
    }
}

std::optional<PositionReport> PositionRepositoryPg::findLatestByMmsi(long long mmsi) {
    try {
        auto conn = pool_.acquire();
        pqxx::read_transaction tx(*conn);

        const pqxx::result result = tx.exec(
            R"(
                SELECT mmsi, latitude, longitude, sog_knots, cog_degrees,
                true_heading, navigational_status, ais_timestamp
                FROM position_reports
                WHERE mmsi = $1
                ORDER BY ais_timestamp DESC
                LIMIT 1
            )",
            mmsi);

        if (result.empty())
        {
            return std::nullopt;
        }

        return mapToPositionReport(result[0]);
    } catch (const std::exception &e) {
        throw DatabaseException(std::string("Unexpected database error while finding latest position report: ") + e.what());
    } catch (const pqxx::sql_error &e) {
        throw DatabaseException(std::string("Database SQL error while retrieving latest vessel position: ") + e.what());
    } catch (const pqxx::broken_connection &e) {
        throw DatabaseException(std::string("Database connection error while retrieving latest vessel position: ") + e.what());
    } catch (const pqxx::failure &e) {
        throw DatabaseException(std::string("Database failure while retrieving latest vessel position: ") + e.what());
    }
}

std::vector<PositionReport> PositionRepositoryPg::findByMmsiAndTimeRange(
    long long mmsi,
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) {
    try {
        auto conn = pool_.acquire();
        pqxx::read_transaction tx(*conn);

        const pqxx::result result = tx.exec(
            R"(
                    SELECT mmsi, latitude, longitude, sog_knots, cog_degrees,
                    true_heading, navigational_status, ais_timestamp
                    FROM position_reports
                    WHERE mmsi = $1 AND ais_timestamp BETWEEN $2 AND $3
                    ORDER BY ais_timestamp DESC
                )",
            pqxx::params{
                mmsi,
                time_utils::to_pg_timestamp(from),
                time_utils::to_pg_timestamp(to)
            }
        );

        std::vector<PositionReport> reports;
        for (const auto &row : result) {
            reports.push_back(mapToPositionReport(row));
        }
        return reports;
    } catch (const std::exception &e) {
        throw DatabaseException(std::string("Unexpected database error while retrieving vessel position history: ") + e.what());
    } catch (const pqxx::sql_error &e) {
        throw DatabaseException(std::string("Database SQL error while retrieving vessel position history: ") + e.what());
    } catch (const pqxx::broken_connection &e) {
        throw DatabaseException(std::string("Database connection error while retrieving vessel position history: ") + e.what());
    } catch (const pqxx::failure &e) {
        throw DatabaseException(std::string("Database failure while retrieving vessel position history: ") + e.what());
    }
}

PositionReport PositionRepositoryPg::mapToPositionReport(const pqxx::row &row)
{
    return PositionReport{
        .mmsi = row["mmsi"].as<long long>(),
        .latitude = row["latitude"].as<double>(),
        .longitude = row["longitude"].as<double>(),
        .sogKnots = row["sog_knots"].as<double>(),
        .cogDegrees = row["cog_degrees"].as<double>(),
        .trueHeading = row["true_heading"].as<double>(),
        .navStatus = readOptional<int>(row, "navigational_status"),
        .aisTimestamp = time_utils::from_pg_timestamp(row["ais_timestamp"].c_str())
    };
}

template <typename T>
std::optional<T> PositionRepositoryPg::readOptional(const pqxx::row& row, const char* column) {
    if (row[column].is_null()) {
        return std::nullopt;
    }
    return row[column].as<T>();
}
template std::optional<double> PositionRepositoryPg::readOptional<double>(const pqxx::row&, const char*);
template std::optional<int> PositionRepositoryPg::readOptional<int>(const pqxx::row&, const char*);