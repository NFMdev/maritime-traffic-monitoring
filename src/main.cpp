#include <crow.h>

#include "api/health_controller.hpp"
#include "api/position_controller.hpp"
#include "repository/position_repository_pg.hpp"
#include "service/position_service.hpp"
#include "postgres_pool.hpp"

int main() {
    crow::SimpleApp app;

    // Initialize PostgreSQL connection pool
    auto pgPool = std::make_shared<PostgresPool>("postgresql://user:password@localhost:5432/maritime_db");
    PositionRepositoryPg positionRepository(*pgPool);
    PositionService positionService(positionRepository);

    registerHealthRoutes(app);
    registerPositionRoutes(app, positionService);

    app.port(8080).multithreaded().run();
    return 0;
}