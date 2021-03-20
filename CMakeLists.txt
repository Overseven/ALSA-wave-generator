cmake_minimum_required(VERSION 3.17)
project(ALSA-wave-generator)

set(CMAKE_CXX_STANDARD 14)

add_executable(generator
        generator.cpp)

target_link_libraries (generator -lasound -lm)