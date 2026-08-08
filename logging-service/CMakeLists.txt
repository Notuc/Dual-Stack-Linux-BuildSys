cmake_minimum_required(VERSION 3.20)
project(logging-service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(logging-service Src/main.cpp)

install(TARGETS logging-service RUNTIME DESTINATION bin)
