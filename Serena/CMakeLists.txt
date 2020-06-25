add_executable(Serena_sim  Serena.hpp Serena_sim.cpp ${common_headers})
target_link_libraries(Serena_sim ${common_libraries})

add_custom_command(TARGET Serena_sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Serena_sim> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:Serena_sim>)

install(TARGETS Serena_sim DESTINATION bin)