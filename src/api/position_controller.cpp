#include "api/position_controller.hpp"

#include "util/app_exceptions.hpp"
#include "util/time_utils.hpp"

#include <initializer_list>
#include <optional>
#include <string>

namespace {
    crow::response jsonError(int status, const std::string& message) {
        crow::json::wvalue response;
        response["error"] = message;
        return crow::response{status, response};
    }

    std::optional<double> readOptionalDouble(const crow::json::rvalue& body, const char* key) {
        auto it = body[key];
        if (!it) {
            return std::nullopt;
        }
        try {
            return it.d();
        } catch (const std::exception&) {
            throw BadRequestException("Invalid value for " + std::string(key) + ": expected a number");
        }
    }

    std::optional<int> readOptionalInt(const crow::json::rvalue& body, const char* key) {
        auto it = body[key];
        if (!it) {
            return std::nullopt;
        }
        try {
            return it.i();
        } catch (const std::exception&) {
            throw BadRequestException("Invalid value for " + std::string(key) + ": expected an integer");
        }
    }

    void requireFields(
        const crow::json::rvalue& body,
        std::initializer_list<const char*> fields
    ) {
        for (const char* field : fields) {
            if (!body.has(field)) {
                throw BadRequestException("Missing required field: " + std::string(field));
            }
        }
    }

    PositionReport parsePositionReport(const crow::json::rvalue& body) {
        requireFields(body, {
            "mmsi",
            "latitude",
            "longitude",
            "sogKnots",
            "cogDegrees",
            "trueHeading",
            "aisTimestamp"
        });

        return PositionReport {
            .mmsi = body["mmsi"].i(),
            .latitude = body["latitude"].d(),
            .longitude = body["longitude"].d(),
            .sogKnots = body["sogKnots"].d(),
            .cogDegrees = body["cogDegrees"].d(),
            .trueHeading = body["trueHeading"].d(),
            .navStatus = readOptionalInt(body, "navStatus"),
            .aisTimestamp = time_utils::from_iso_utc(body["aisTimestamp"].s())
        };
    }

    crow::json::wvalue positionReportToJson(const PositionReport& report) {
        crow::json::wvalue json;
        json["mmsi"] = report.mmsi;
        json["latitude"] = report.latitude;
        json["longitude"] = report.longitude;
        json["sogKnots"] = report.sogKnots;
        json["cogDegrees"] = report.cogDegrees;
        json["trueHeading"] = report.trueHeading;
        if (report.navStatus.has_value()) {
            json["navStatus"] = *report.navStatus;
        } else {
            json["navStatus"] = nullptr;
        }
        json["aisTimestamp"] = time_utils::to_iso_utc(report.aisTimestamp);
        return json;
    }
} // namespace

void registerPositionRoutes(crow::SimpleApp& app, PositionService& positionService) {
    CROW_ROUTE(app, "/api/v1/positions")
    .methods(crow::HTTPMethod::POST)
    ([&positionService](const crow::request& req) {
        try {
            const auto body = crow::json::load(req.body);
            if (!body) {
                return jsonError(400, "Invalid JSON");
            }

            PositionReport report = parsePositionReport(body);
            positionService.ingest(report);

            return crow::response{200};
        } catch (const ValidationException& ex) {
            return jsonError(400, ex.what());
        } catch (const DatabaseException& ex) {
            return jsonError(500, "Internal database error");
        } catch (const std::exception& ex) {
            return jsonError(500, "Internal server error");
        }
    });

    CROW_ROUTE(app, "/api/v1/positions/<string>/latest")
    .methods(crow::HTTPMethod::GET)
    ([&positionService](const std::string& mmsi) {
        try {
            const auto reportOpt = positionService.latest(static_cast<long long>(std::stoll(mmsi)));
            if (!reportOpt.has_value()) {
                return jsonError(404, "Position report not found for MMSI: " + mmsi);
            }
            return crow::response{200, positionReportToJson(*reportOpt)};
        } catch (const ValidationException& ex) {
            return jsonError(400, ex.what());
        } catch (const DatabaseException& ex) {
            return jsonError(500, "Internal database error");
        } catch (const std::exception& ex) {
            return jsonError(500, "Internal server error");
        }
    });

    CROW_ROUTE(app, "/api/v1/positions/<string>/history?from=<string>&to=<string>")
    .methods(crow::HTTPMethod::GET)
    ([&positionService](const std::string& mmsi, const std::string& fromStr, const std::string& toStr) {
        try {
            const auto from = time_utils::from_iso_utc(fromStr);
            const auto to = time_utils::from_iso_utc(toStr);
            const auto reports = positionService.history(static_cast<long long>(std::stoll(mmsi)), from, to);
            crow::json::wvalue json;
            for (size_t i = 0; i < reports.size(); ++i) {
                json[std::to_string(i)] = positionReportToJson(reports[i]);
            }
            return crow::response{200, json};
        } catch (const ValidationException& ex) {
            return jsonError(400, ex.what());
        } catch (const DatabaseException& ex) {
            return jsonError(500, "Internal database error");
        } catch (const std::exception& ex) {
            return jsonError(500, "Internal server error");
        }
    });
}