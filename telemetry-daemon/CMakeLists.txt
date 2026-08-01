cmake_minimum_required(VERSION 3.20)
project(telemetry-daemon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGPIOD REQUIRED IMPORTED_TARGET libgpiod)

add_subdirectory(Drivers)          

add_executable(daemon Src/main.cpp)
target_link_libraries(daemon PRIVATE i2c bme280 can PkgConfig::LIBGPIOD pthread)
target_include_directories(daemon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Inc)

install(TARGETS daemon RUNTIME DESTINATION bin)
