cmake_minimum_required(VERSION 3.25)

project(mandelbrot_simple VERSION 0.1)

add_subdirectory(libs)

add_executable(Mandelbrot_simple main.c)

target_include_directories(Mandelbrot_simple PUBLIC "${PROJECT_SOURCE_DIR}/libs")

target_link_libraries(Mandelbrot_simple PUBLIC Complex)