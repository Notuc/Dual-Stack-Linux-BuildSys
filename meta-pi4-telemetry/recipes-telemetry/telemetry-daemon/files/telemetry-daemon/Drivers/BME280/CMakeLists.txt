add_library(bme280 STATIC bme280.cpp)
target_include_directories(bme280 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(bme280 PUBLIC i2c)
