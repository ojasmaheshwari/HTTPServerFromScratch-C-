cmake_minimum_required(VERSION 3.16)

# Project name and language
project(HTTPServerFromScratch LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Linux: static linking for libstdc++ and libgcc
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# Add executable
add_executable(server
        src/main.cpp
        src/http_parser.cpp
        src/util.cpp
        src/thread_pool.cpp
        src/vendor/logging/AsciiColor.cpp
        src/vendor/logging/Logging.cpp
        src/http_response_builder.cpp
        src/vendor/nlohmann/json.hpp
        src/server.cpp
)

target_include_directories(server PUBLIC
        src/include
        src/vendor/logging/include
        src/vendor
)

# Copy sample resources to the build directory
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})