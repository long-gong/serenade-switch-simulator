add_executable(iSLIP_sim  iSLIP.hpp iSLIP_sim.cpp ${common_headers})
target_link_libraries(iSLIP_sim ${common_libraries})

add_custom_command(TARGET iSLIP_sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:iSLIP_sim> ${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:iSLIP_sim>)

install(TARGETS iSLIP_sim DESTINATION bin)