cmake_minimum_required(VERSION 3.16)
project(JustPlay C RC)

# justplay: console program. justplayw: no window, for Explorer file associations.
add_executable(justplay justplay.c res/justplay.rc)
add_executable(justplayw WIN32 justplay.c res/justplay.rc)
target_compile_definitions(justplayw PRIVATE JUSTPLAY_GUI)

foreach(target justplay justplayw)
    target_compile_definitions(${target} PRIVATE UNICODE _UNICODE)
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -municode>)
    target_link_options(${target} PRIVATE -municode -static)
    target_link_libraries(${target} PRIVATE winmm shlwapi)
endforeach()
