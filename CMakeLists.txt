cmake_minimum_required(VERSION 3.16)
project(JustPlay C)

add_executable(justplay justplay.c)
target_compile_definitions(justplay PRIVATE UNICODE _UNICODE)
target_compile_options(justplay PRIVATE -Wall -Wextra -municode)
target_link_options(justplay PRIVATE -municode -static)
target_link_libraries(justplay PRIVATE winmm shlwapi)
