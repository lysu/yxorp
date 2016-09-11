include_directories(${PROJECT_SOURCE_DIR}/lib/zmalloc/includes)
set(ZM_SRC zmalloc.c)
add_library(zmalloc ${ZM_SRC})