#include "config/app_config.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {
    std::string requireEnv(const char* key) {
        const char* value = std::getenv(key);
        if (!value) {
            throw std::runtime_error("Environment variable not set: " + std::string(key));
        }
        return std::string(value);
    }

    std::string getEnvOrDefault(const char* key, const std::string& defaultValue) {
        const char* value = std::getenv(key);
        return value ? std::string(value) : defaultValue;
    }

    int getEnvIntOrDefault(const char* key, int defaultValue) {
        const char* value = std::getenv(key);
        if (value == nullptr || std::string(value).empty()) {
            return defaultValue;
        }

        try {
            return std::stoi(value);
        } catch(...) {
            throw std::runtime_error(std::string("Invalid integer value for environment variable: ") + key);
        }
    }

    bool getEnvBoolOrDefault(const char* key, bool defaultValue) {
        const char* value = std::getenv(key);
        if (value == nullptr || std::string(value).empty()) {
            return defaultValue;
        }

        const std::string str(value);
        
        if (str == "true" || str == "1" || str == "TRUE") {
            return true;
        }

        if (str == "false" || str == "0" || str == "FALSE") {
            return false;
        }

        throw std::runtime_error(std::string("Invalid boolean value for environment variable: ") + key);
    }
}

std::string DatabaseConfig::toConnectionString() const {
    return "host=" + host +
           " port=" + std::to_string(port) +
           " dbname=" + dbName +
           " user=" + user +
           " password=" + password +
           " sslmode=" + sslMode;
}

AppConfig AppConfig::fromEnvironment() {
    AppConfig config {
        .server = ServerConfig {
            .port = getEnvIntOrDefault("APP_PORT", 8080),
            .multithreaded = getEnvBoolOrDefault("APP_MULTITHREADED", true)
        },
        .database = DatabaseConfig {
            .host = requireEnv("DB_HOST"),
            .port = getEnvIntOrDefault("DB_PORT", 5432),
            .dbName = requireEnv("DB_NAME"),
            .user = requireEnv("DB_USER"),
            .password = requireEnv("DB_PASSWORD"),
            .sslMode = getEnvOrDefault("DB_SSLMODE", "disable")
        }
    };

    return config;
}