#include "db/postgres_pool.hpp"

PostgresPool::PostgresPool(const std::string& connStr) : connStr_(connStr) {}

std::unique_ptr<pqxx::connection> PostgresPool::acquire() const {
    return std::make_unique<pqxx::connection>(connStr_);
}