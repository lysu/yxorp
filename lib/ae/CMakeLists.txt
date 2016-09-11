include_directories(
        ${PROJECT_SOURCE_DIR}/lib/zmalloc/includes
        ${PROJECT_SOURCE_DIR}/lib/ae/includes)
set(AE_SRC ae.c)
add_library(ae ${AE_SRC})