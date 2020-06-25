add_executable(SC_Serenade_sim Serenade_Conservative_for_Proof.hpp Serenade_Conservative_for_Proof_sim.cpp ${common_headers})
target_link_libraries(SC_Serenade_sim ${common_libraries})

add_custom_command(TARGET SC_Serenade_sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SC_Serenade_sim> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:SC_Serenade_sim>)

install(TARGETS SC_Serenade_sim DESTINATION bin)