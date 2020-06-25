add_executable(O_Serenade_sim_true Serenade_Opportunistic.hpp Serenade_Opportunistic_sim.cpp ${common_headers})
target_link_libraries(O_Serenade_sim_true ${common_libraries})

add_custom_command(TARGET O_Serenade_sim_true
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:O_Serenade_sim_true> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:O_Serenade_sim_true>)

install(TARGETS O_Serenade_sim_true DESTINATION bin)