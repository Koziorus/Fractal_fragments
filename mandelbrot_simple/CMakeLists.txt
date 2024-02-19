cmake_minimum_required(VERSION 3.25)

project(mandelbrot_simple VERSION 0.1)

add_subdirectory(libs)

link_directories("C:/mingw_libs/lib/")

add_executable(Mandelbrot_simple main.c)

target_include_directories(Mandelbrot_simple PUBLIC "${PROJECT_SOURCE_DIR}/libs"
													"C:/mingw_libs/include/SDL2")
target_link_libraries(Mandelbrot_simple PUBLIC Complex)

target_link_libraries(Mandelbrot_simple PUBLIC mingw32) 
target_link_libraries(Mandelbrot_simple PUBLIC SDL2main) 
target_link_libraries(Mandelbrot_simple PUBLIC SDL2)

