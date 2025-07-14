# Set the root directory for source files
set(SERVER_SRCROOT ${PROJECT_SOURCE_DIR}/lab5/UDPServer)
set(CLIENT_SRCROOT ${PROJECT_SOURCE_DIR}/lab5/UDPClient)

# All source files for server and client
set(SERVER_SRC ${SERVER_SRCROOT}/udpServer.cpp)
set(CLIENT_SRC ${CLIENT_SRCROOT}/udpClient.cpp)

# Define the UDP server target
sfml_add_example(UDPServer
                 SOURCES ${SERVER_SRC})
target_link_libraries(UDPServer sfml-network sfml-graphics sfml-window sfml-system)
# Copy robo.png to the server executable directory
add_custom_command(
    TARGET UDPServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${SERVER_SRCROOT}/graphics/robo.png
            $<TARGET_FILE_DIR:UDPServer>
)
# Define the UDP client target
sfml_add_example(UDPClient
                 SOURCES ${CLIENT_SRC})
	       target_link_libraries(UDPClient sfml-network)
	       

