/*
Author: Kamya Hari
Class: ECE6122 A
Last Date Modified: 11/19/2024
Description:
This is the function to run the UDP Server. On running the executable, a black window appears, and when a client connects to the server, a robot image appears. 
According to the commands sent by the server, the robot changes direction and speed. On getting the quit command, the robot disappears from the screen.
On closing this window, the server sends a message to the client and it shuts down.
*/


#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <cmath>

// Constants
const unsigned short PORT = 61000;
const sf::Vector2f WINDOW_SIZE(800, 600);
const float INITIAL_SPEED = 3.0f;
const sf::Vector2f ROBOT_SIZE(25.0f, 25.0f);

// Function to calculate rotation angle from velocity
float calculateRotation(const sf::Vector2f& velocity) {
    /* Convert velocity to angle in degrees, atan2 returns angle in radians, convert to degrees, Subtract 90 because SFML sprites face up by default*/
    if (velocity.x == 0 && velocity.y == 0) return 0;
    return (atan2(velocity.y, velocity.x) * 180 / M_PI) + 90;
}

// Function to calculate vector length
float vectorLength(const sf::Vector2f& vector) {
    return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

int main(int argc, char* argv[])
{
    // Set up the window
    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(WINDOW_SIZE.x), 
                                      static_cast<unsigned int>(WINDOW_SIZE.y)), 
                                      "UDP Robot Server");
    window.setFramerateLimit(60);

    // Set up UDP socket
    sf::UdpSocket socket;
    socket.setBlocking(false);
    if (socket.bind(PORT) != sf::Socket::Done) {
        std::cerr << "Error binding UDP socket to port " << PORT << std::endl;
        return 1;
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    // Declare sender information
    sf::IpAddress sender;
    unsigned short senderPort;
    bool clientConnected = false;

    // Robot representation
    sf::Texture robotTexture;
    if (!robotTexture.loadFromFile("robo.png")) {
        std::cerr << "Error loading robot image!" << std::endl;
        return 1;
    }

    sf::Sprite robotSprite;
    robotSprite.setTexture(robotTexture);
    robotSprite.setScale(0.1f, 0.1f);
    
    // Set the origin to the center of the sprite for proper rotation
    sf::FloatRect bounds = robotSprite.getLocalBounds();
    robotSprite.setOrigin(bounds.width / 2, bounds.height / 2);
    
    // Position the sprite (accounting for the new origin)
    robotSprite.setPosition(WINDOW_SIZE.x / 2, WINDOW_SIZE.y / 2);
    robotSprite.setColor(sf::Color(255, 255, 255, 0)); // Start invisible
    
    sf::Vector2f velocity(INITIAL_SPEED, 0.0f);
    float currentSpeed = INITIAL_SPEED;

    // Clock for timing
    sf::Clock clock;    

    bool running = true;
    while (running && window.isOpen()) {
        // Event handling
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                // Notify client about server shutdown
                if (clientConnected) {
                    const std::string shutdownMessage = "server_shutdown";
                    socket.send(shutdownMessage.c_str(), shutdownMessage.size(), sender, senderPort);
                }
                running = false;
            }
        }

        // Receive data from client
        char data[128];
        std::size_t received = 0;
        
        if (socket.receive(data, sizeof(data), received, sender, senderPort) == sf::Socket::Done) {
            std::string message(data, received);
            std::cout << "Received: " << message << " from " << sender << ":" << senderPort << std::endl;

            if (message == "connect" && !clientConnected) {
                clientConnected = true;
                robotSprite.setColor(sf::Color(255, 255, 255, 255));
                velocity = sf::Vector2f(currentSpeed, 0);
                robotSprite.setRotation(calculateRotation(velocity));
            }
            else if (message == "disconnect") {
                clientConnected = false;
                robotSprite.setColor(sf::Color(255, 255, 255, 0));
                velocity = sf::Vector2f(0, 0);
            }
            else if (clientConnected) {
                sf::Vector2f newVelocity = velocity;
                bool velocityChanged = false;

                if (message == "up") {
                    newVelocity = sf::Vector2f(0, -currentSpeed);
                    velocityChanged = true;
                }
                else if (message == "down") {
                    newVelocity = sf::Vector2f(0, currentSpeed);
                    velocityChanged = true;
                }
                else if (message == "left") {
                    newVelocity = sf::Vector2f(-currentSpeed, 0);
                    velocityChanged = true;
                }
                else if (message == "right") {
                    newVelocity = sf::Vector2f(currentSpeed, 0);
                    velocityChanged = true;
                }
                else if (message == "faster") {
                    currentSpeed += 1.0f;
                    // Maintain direction but update speed
                    float length = vectorLength(velocity);
                    if (length > 0) {
                        newVelocity = velocity * (currentSpeed / length);
                        velocityChanged = true;
                    }
                }
                else if (message == "slower") {
                    currentSpeed = std::max(1.0f, currentSpeed - 1.0f);
                    // Maintain direction but update speed
                    float length = vectorLength(velocity);
                    if (length > 0) {
                        newVelocity = velocity * (currentSpeed / length);
                        velocityChanged = true;
                    }
                }

                if (velocityChanged) {
                    velocity = newVelocity;
                    robotSprite.setRotation(calculateRotation(velocity));
                }
            }
        }

        // Update robot position if connected
        if (clientConnected) {
            sf::Time elapsed = clock.restart();
            sf::Vector2f movement = velocity * elapsed.asSeconds();
            robotSprite.move(movement);

            // Boundary handling with bouncing
            sf::Vector2f pos = robotSprite.getPosition();
            bool velocityChanged = false;
            
            // Get the current bounds considering the sprite's size and origin
            float effectiveRadius = std::max(ROBOT_SIZE.x, ROBOT_SIZE.y) * 0.1f / 2; 

            // Horizontal boundaries
            if (pos.x - effectiveRadius < 0) {
                robotSprite.setPosition(effectiveRadius, pos.y);
                velocity.x = std::abs(velocity.x);
                velocityChanged = true;
            }
            else if (pos.x + effectiveRadius > WINDOW_SIZE.x) {
                robotSprite.setPosition(WINDOW_SIZE.x - effectiveRadius, pos.y);
                velocity.x = -std::abs(velocity.x);
                velocityChanged = true;
            }

            // Vertical boundaries
            if (pos.y - effectiveRadius < 0) {
                robotSprite.setPosition(pos.x, effectiveRadius);
                velocity.y = std::abs(velocity.y);
                velocityChanged = true;
            }
            else if (pos.y + effectiveRadius > WINDOW_SIZE.y) {
                robotSprite.setPosition(pos.x, WINDOW_SIZE.y - effectiveRadius);
                velocity.y = -std::abs(velocity.y);
                velocityChanged = true;
            }

            // Update rotation if velocity changed due to bouncing
            if (velocityChanged) {
                robotSprite.setRotation(calculateRotation(velocity));
            }
        }
        else {
            clock.restart();
        }

        // Render
        window.clear(sf::Color::Black);
        window.draw(robotSprite);
        window.display();
    }

    return 0;
}
