cmake_minimum_required(VERSION 3.16)

# Project name and language
project(HTTPServerFromScratch LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Force static linking of C++ runtime on Linux/Windows
if(MSVC)
    # Windows: static runtime
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    # Linux: static linking for libstdc++ and libgcc
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Add executable
add_executable(server
    main.cpp
    http_parser.cpp
    util.cpp
    thread_pool.cpp
    vendor/logging/AsciiColor.cpp
    vendor/logging/Logging.cpp
    http_response_builder.cpp
    vendor/nlohmann/json.hpp
    server.cpp
)

# Benchmark target
add_subdirectory(vendor/benchmark)
add_executable(bench_single_client_processing
        benchmarks/benchmark_single_client_processing.cpp
        server.cpp
        http_parser.cpp
        util.cpp
        vendor/logging/AsciiColor.cpp
        vendor/logging/Logging.cpp
        http_response_builder.cpp
        vendor/nlohmann/json.hpp
)
target_link_libraries(bench_single_client_processing PRIVATE benchmark::benchmark pthread)