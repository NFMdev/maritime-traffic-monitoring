#pragma once

#include <string>

struct DatabaseConfig {
    std::string host;
    int port;
    std::string dbName;
    std::string user;
    std::string password;
    std::string sslMode;

    std::string toConnectionString() const;
};

struct ServerConfig {
    int port;
    bool multithreaded;
};

struct AppConfig {
    ServerConfig server;
    DatabaseConfig database;

    static AppConfig fromEnvironment();
};