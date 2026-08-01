add_library(can STATIC
    can.cpp
    can_interface.cpp
)

target_include_directories(can PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(test_can_loopback test_can_loopback.cpp)
target_link_libraries(test_can_loopback PRIVATE can)

add_executable(test_can_filter test_can_filter.cpp)
target_link_libraries(test_can_filter PRIVATE can)
