#pragma once

#include <pqxx/pqxx>
#include <memory>
#include <string>

class PostgresPool {
public:
    explicit PostgresPool(const std::string& connStr) : connStr_(connStr) {}

    std::unique_ptr<pqxx::connection> acquire() const {
        return std::make_unique<pqxx::connection>(connStr_);
    }

    private:
        std::string connStr_;
};