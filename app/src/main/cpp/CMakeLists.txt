cmake_minimum_required(VERSION 3.22.1)
project("lastsurvivor")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/player
    ${CMAKE_CURRENT_SOURCE_DIR}/weapon
    ${CMAKE_CURRENT_SOURCE_DIR}/enemy
    ${CMAKE_CURRENT_SOURCE_DIR}/wave
    ${CMAKE_CURRENT_SOURCE_DIR}/loot
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/android
)

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/player/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/weapon/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/enemy/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wave/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loot/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/audio/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/android/*.cpp
)

add_library(lastsurvivor SHARED ${SOURCES})

find_library(log-lib log)
find_library(gles3-lib GLESv3)
find_library(android-lib android)

target_link_libraries(lastsurvivor
    ${log-lib}
    ${gles3-lib}
    ${android-lib}
    m
)
