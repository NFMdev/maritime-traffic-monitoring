#pragma once

#include <crow.h>
#include "service/position_service.hpp"

void registerPositionRoutes(crow::SimpleApp& app, PositionService& positionService);