#include <crow.h>

#include "api/health_controller.hpp"
#include "api/position_controller.hpp"
#include "repository/position_repository_pg.hpp"
#include "service/position_service.hpp"
#include "db/postgres_pool.hpp"
#include "config/app_config.hpp"

int main() {
    const AppConfig config = AppConfig::fromEnvironment();

    PostgresPool PostgresPool(config.database.toConnectionString());
    PositionRepositoryPg PositionRepository(PostgresPool);
    PositionService positionService(PositionRepository);

    crow::SimpleApp app;

    registerHealthRoutes(app);
    registerPositionRoutes(app, positionService);

    if (config.server.multithreaded) {
        app.port(config.server.port).multithreaded().run();
    } else {
        app.port(config.server.port).run();
    }
    
    return 0;
}