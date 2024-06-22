cmake_minimum_required(VERSION 3.10)

get_filename_component(LIB_TITLE ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_library(${LIB_TITLE} STATIC 
    ${LIB_TITLE}.cpp
    ${LIB_TITLE}.h
)
target_include_directories(${LIB_TITLE}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

add_library(Libs::Logger ALIAS ${LIB_TITLE})
