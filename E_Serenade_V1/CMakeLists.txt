add_executable(E_Serenade_sim E_Serenade.hpp E_Serenade_sim.cpp ${common_headers})
target_link_libraries(E_Serenade_sim ${common_libraries})

add_custom_command(TARGET E_Serenade_sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:E_Serenade_sim> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:E_Serenade_sim>)

install(TARGETS E_Serenade_sim DESTINATION bin)