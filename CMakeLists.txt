cmake_minimum_required(VERSION 3.10)
project(MetricsDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(metrics_lib metrics_lib.cpp)

add_executable(main_http_cpu main_http_cpu.cpp)
target_link_libraries(main_http_cpu metrics_lib)

add_executable(main_temperature main_temperature.cpp)
target_link_libraries(main_temperature metrics_lib)

add_executable(main_errors main_errors.cpp)
target_link_libraries(main_errors metrics_lib)
