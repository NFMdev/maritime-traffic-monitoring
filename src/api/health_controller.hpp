#pragma once

#include <crow.h>

inline void registerHealthRoutes(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/health")
    ([]() {
        crow::json::wvalue response;
        response["status"] = "ok";
        return crow::response{200, response};
    });
}