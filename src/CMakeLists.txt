file(GLOB SRC *.c)
set(SRC
  yxorp.c net.c ae.c zmalloc)

add_executable(yxorp ${SRC})

target_link_libraries(yxorp ${APP_LIBRARIES})
