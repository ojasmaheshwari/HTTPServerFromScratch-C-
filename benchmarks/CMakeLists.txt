cmake_minimum_required(VERSION 3.16)

# Project name and language
project(ServerBenchmark LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Linux: static linking for libstdc++ and libgcc
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

add_subdirectory(google_benchmark)

add_executable(bench_single_client_processing
        benchmark_single_client_processing.cpp
        ../server/src/server.cpp
        ../server/src/http_parser.cpp
        ../server/src/util.cpp
        ../server/src/vendor/logging/AsciiColor.cpp
        ../server/src/vendor/logging/Logging.cpp
        ../server/src/http_response_builder.cpp
        ../server/src/vendor/nlohmann/json.hpp
)
target_include_directories(bench_single_client_processing PUBLIC
        ../server/src/include
        ../server/src/vendor/logging/include
        ../server/src/vendor
)
target_link_libraries(bench_single_client_processing PRIVATE benchmark::benchmark pthread)

# Copy sample resources to the build directory
file(COPY ../server/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})