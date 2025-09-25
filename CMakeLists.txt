cmake_minimum_required(VERSION 3.16)

# Project name and language
project(HTTPServerFromScratch LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(server
    main.cpp
    http_parser.cpp
    util.cpp
    vendor/logging/AsciiColor.cpp
    vendor/logging/Logging.cpp
    http_response_builder.cpp
    vendor/nlohmann/json.hpp
)
