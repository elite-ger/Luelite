cmake_minimum_required(VERSION 3.10)
project(Luelite C)

set(LUA_SRC_DIR "${CMAKE_SOURCE_DIR}/lua-5.5.0/src")

add_library(lua STATIC IMPORTED)
set_target_properties(lua PROPERTIES
    IMPORTED_LOCATION "${LUA_SRC_DIR}/liblua.a"
)

set(LUELITE_SOURCES
    luelite.c
    luelib/luelib_sched.c
    luelib/luelib_http.c
    luelib/luelib_match.c
)

add_executable(luelite ${LUELITE_SOURCES})

target_include_directories(luelite PRIVATE
    "${LUA_SRC_DIR}"
    "${CMAKE_SOURCE_DIR}"
)

target_link_libraries(luelite PRIVATE lua m)

if(WIN32)
    target_link_libraries(luelite PRIVATE ws2_32)
endif()