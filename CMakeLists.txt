cmake_minimum_required(VERSION 3.13)
project(MotifCommunityDetectionMPI)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)

set(CMAKE_C_COMPILER /usr/local/bin/mpicc)
set(CMAKE_CXX_COMPILER /usr/local/bin/mpic++)

SET(CMAKE_CXX_FLAGS "-O3")
SET(CMAKE_C_FLAGS "-O3")

add_executable(MotifCommunityDetectionMPI src/main.cpp)
