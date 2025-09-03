cmake_minimum_required(VERSION 3.14)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(NeoVSID VERSION "1.3.3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_COMPILER "g++-14")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(DIR_SEPARATOR="\\\\")
else()
    add_compile_definitions(DIR_SEPARATOR="/")
endif()

if(DEV)
    message(STATUS "Development mode enabled")
    add_compile_definitions(DEV=1)
endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_BINARY_DIR}/Version.h
)

if (DEBUG)
    add_compile_definitions(
        DEBUG=1
    )
    message(STATUS "DEBUG mode enabled")
endif()

add_subdirectory(External/NeoRadarSDK)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

set(SOURCES
    src/NeoVSID.cpp
    src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES}  "src/core/DataManager.cpp" "src/utils/Format.h" "src/config/Colors.h")

target_link_libraries(${PROJECT_NAME} PRIVATE 
    NeoRadarSDK::NeoRadarSDK
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/External/httplib)
target_compile_definitions(${PROJECT_NAME} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PREFIX ""
)

if(${CMAKE_HOST_APPLE})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}-${CMAKE_HOST_SYSTEM_PROCESSOR}
    )
endif()

file(GLOB CONFIG_JSON_FILES "${CMAKE_SOURCE_DIR}/src/config/*.json")
file(COPY ${CONFIG_JSON_FILES} DESTINATION ${CMAKE_BINARY_DIR})

