/*
Author: Kamya Hari
Class: ECE6122 A
Last Date Modified: 11/19/2024
Description:
This is the function to run the UDP Client. On running the executable,the client starts running - asks for the server to connect.
You give commands on the command prompt and appropriate changes happen in the window displayed by the Server. 
*/
#include <SFML/Network.hpp>
#include <iostream>
#include <string>

// Constants
const unsigned short SERVER_PORT = 61000;

int main(int argc, char* argv[])
{
    // Ask for the server address
    sf::IpAddress server;
    do {
        std::cout << "Type the address or name of the server to connect to: ";
        std::cin >> server;
    } while (server == sf::IpAddress::None);

    // Create a UDP socket
    sf::UdpSocket socket;
    socket.setBlocking(false); // Non-blocking for continuous input handling

     // Send initial connect message
    const std::string connectMessage = "connect";
    if (socket.send(connectMessage.c_str(), connectMessage.size(), server, SERVER_PORT) != sf::Socket::Done) {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    bool running = true;
    while (running) {
        // Prompt the user for input
        std::cout << "Enter command (w/a/s/d for direction, g for faster, h for slower, q to quit): ";
        std::string command;
        std::cin >> command;

        // Send command to server
        if (command == "q") {
            const std::string disconnectMessage = "disconnect";
            if (socket.send(disconnectMessage.c_str(), disconnectMessage.size(), server, SERVER_PORT) != sf::Socket::Done) {
                std::cerr << "Error sending disconnect message to server" << std::endl;
            }
            running = false;
        } else if (command == "w") {
            const std::string moveUp = "up";
            socket.send(moveUp.c_str(), moveUp.size(), server, SERVER_PORT);
        } else if (command == "s") {
            const std::string moveDown = "down";
            socket.send(moveDown.c_str(), moveDown.size(), server, SERVER_PORT);
        } else if (command == "a") {
            const std::string moveLeft = "left";
            socket.send(moveLeft.c_str(), moveLeft.size(), server, SERVER_PORT);
        } else if (command == "d") {
            const std::string moveRight = "right";
            socket.send(moveRight.c_str(), moveRight.size(), server, SERVER_PORT);
        } else if (command == "g") {
            const std::string faster = "faster";
            socket.send(faster.c_str(), faster.size(), server, SERVER_PORT);
        } else if (command == "h") {
            const std::string slower = "slower";
            socket.send(slower.c_str(), slower.size(), server, SERVER_PORT);
        } else {
            std::cout << "Invalid command!" << std::endl;
        }

        // Check for server messages
        char buffer[1024];
        std::size_t received;
        sf::IpAddress sender;
        unsigned short senderPort;

        if (socket.receive(buffer, sizeof(buffer), received, sender, senderPort) == sf::Socket::Done) {
            std::string message(buffer, received);

            // Handle server shutdown message
            if (message == "server_shutdown") {
                std::cout << "\nServer is shutting down. Closing connection..." << std::endl;
                running = false;
                break;
            }
        }

    }

    return 0;
}
