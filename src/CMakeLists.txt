include_directories(
        ${PROJECT_SOURCE_DIR}/lib/ae/includes
        ${PROJECT_SOURCE_DIR}/lib/zmalloc/includes
)
set(APP_SRC yxorp.c yxorp_net.c)
add_executable(yxorp ${APP_SRC})
target_link_libraries(yxorp ${APP_LIBRARIES} zmalloc ae)
