add_executable(Serenade_sim_true  Serena.hpp Serena_sim.cpp ${common_headers})
target_link_libraries(Serenade_sim_true ${common_libraries})

add_custom_command(TARGET Serenade_sim_true
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Serenade_sim_true> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:Serenade_sim_true>)

install(TARGETS Serenade_sim_true DESTINATION bin)