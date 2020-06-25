add_executable(O_Serenade_Proof_sim Serenade_Opportunistic_Proof.hpp Serenade_Opportunistic_Proof_sim.cpp ${common_headers})
target_link_libraries(O_Serenade_Proof_sim ${common_libraries})

add_custom_command(TARGET O_Serenade_Proof_sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:O_Serenade_Proof_sim> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:O_Serenade_Proof_sim>)

install(TARGETS O_Serenade_Proof_sim DESTINATION bin)