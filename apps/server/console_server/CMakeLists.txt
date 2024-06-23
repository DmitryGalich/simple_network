cmake_minimum_required(VERSION 3.10)

get_filename_component(APP_TITLE ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${APP_TITLE} main.cpp)
target_link_libraries(${APP_TITLE}
    PRIVATE
        Libs::Logger
        Libs::Network
)