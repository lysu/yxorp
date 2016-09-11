include_directories(
        ${PROJECT_SOURCE_DIR}/lib/ae/includes
        ${PROJECT_SOURCE_DIR}/lib/zmalloc/includes
)
set(APP_SRC yxorp.c
        yxorp_core.c
        yxorp_thread.c
        yxorp_log.c
        yxorp_util.c
        yxorp_signal.c)
add_executable(yxorp ${APP_SRC})
target_link_libraries(yxorp ${APP_LIBRARIES} zmalloc ae)
