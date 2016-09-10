cmake_minimum_required(VERSION 3.0)
project(yxorp VERSION 0.0.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
IF (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
ENDIF ()

set(EXECUTABLE_OUTPUT_PATH ${yxorp_SOURCE_DIR}/build/bin/)

set(APP_LIBRARIES)
set(CMAKE_THREAD_PREFER_PTHREAD 1)
find_package(Threads)
if(CMAKE_USE_PTHREADS_INIT)
    list(APPEND APP_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

add_subdirectory(src)


