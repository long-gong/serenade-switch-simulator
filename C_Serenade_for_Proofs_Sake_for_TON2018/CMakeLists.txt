add_executable(C_Serenade_ton_sim Serenade_Conservative_for_Proof.hpp Serenade_Conservative_for_Proof_sim.cpp ${common_headers})
target_link_libraries(C_Serenade_ton_sim ${common_libraries})

add_custom_command(TARGET C_Serenade_ton_sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:C_Serenade_ton_sim> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:C_Serenade_ton_sim>)

install(TARGETS C_Serenade_ton_sim DESTINATION bin)