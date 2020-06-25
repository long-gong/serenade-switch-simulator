
add_executable(C_Serenade_sim Serenade_Conservative.hpp Serenade_Conservative_sim.cpp ${common_headers})
target_link_libraries(C_Serenade_sim ${common_libraries})

add_custom_command(TARGET C_Serenade_sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:C_Serenade_sim> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:C_Serenade_sim>)

install(TARGETS C_Serenade_sim DESTINATION bin)