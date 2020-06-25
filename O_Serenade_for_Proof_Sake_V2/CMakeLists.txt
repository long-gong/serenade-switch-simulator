add_executable(SO_Serenade_sim Serenade_Opportunistic_Proof.hpp Serenade_Opportunistic_Proof_sim.cpp ${common_headers})
target_link_libraries(SO_Serenade_sim ${common_libraries})

add_custom_command(TARGET SO_Serenade_sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SO_Serenade_sim> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:SO_Serenade_sim>)

install(TARGETS SO_Serenade_sim DESTINATION bin)