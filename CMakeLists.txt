cmake_minimum_required(VERSION 3.25)
project(maritime-traffic-monitoring LANGUAGES CXX)


find_package(PostgreSQL REQUIRED)
find_package(libpqxx REQUIRED)

add_executable(maritime_traffic_monitoring
    src/main.cpp
    src/config/app_config.cpp
    src/api/position_controller.cpp
    src/service/position_service.cpp
    src/repository/position_repository_pg.cpp
    src/util/time_utils.cpp
)

target_include_directories(maritime_traffic_monitoring PRIVATE src)

target_link_libraries(maritime_traffic_monitoring PRIVATE
    libpqxx::pqxx
)