cmake_minimum_required(VERSION 3.10.0)

project(tson VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "-Og -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(nlohmann_json 3.10.5 REQUIRED)
find_package(pybind11_json)
find_package(fmt)

add_subdirectory(tson)