/*
Author: Kamya Hari
Class: ECE 6122
Last Date Modified: 30-09-2024

Description:

This file contains the whole code of the retro centipede arcade game. 
*/

#include <SFML/Graphics.hpp>
#include <list>
#include <random>
#include <sstream>
#include <cstdlib>  // For rand()
#include <ctime>    // For time()
#include <iostream>
#include <string>

// Constants
const int WINDOW_WIDTH = 1024; 
const int WINDOW_HEIGHT = 1024;
const int TILE_SIZE = 40;   // Each mushroom occupies a grid of this size
const int NUM_MUSHROOMS = 30; //Number of mushrooms
const int TOP_MARGIN = 100.f; //Top info area
const int BOTTOM_MARGIN = 50.f; //Bottom spaceship area
const float SPACESHIP_SPEED = 5.0f; //speed of spaceship
const float LASER_SPEED = 10.0f; //speed of laser
const float BOTTOM_THRESHOLD = 300; //spider roam area threshold
const int INITIAL_LIVES = 3; //Number of Lives

using namespace sf;

// Mushroom class definition
class Mushroom {
public:
    sf::Sprite sprite;
    sf::Texture textureAlive;
    sf::Texture textureDestroyed;
    int health;  // Number of hits before the mushroom is destroyed
    bool isDestroyed;  // Flag to check if the mushroom is destroyed

    Mushroom(float x, float y) {
        textureAlive.loadFromFile("graphics/Mushroom0.png"); //Loading healthy mushroom
        textureDestroyed.loadFromFile("graphics/Mushroom1.png"); //loading unhealthy mushroom
        sprite.setTexture(textureAlive);
        sprite.setPosition(x, y);
        health = 2;  // Set initial health
        isDestroyed = false;  // Initially, the mushroom is not destroyed
    }

    // Function called when the mushroom is hit by a laser
    void hit() {
        if (!isDestroyed) {
            health--;  // Decrease health on each hit

            if (health == 1) {
                // Change to destroyed state when health reaches 0
                sprite.setTexture(textureDestroyed);  // Change texture to destroyed
            }

            else if(health==0){
                isDestroyed = true;
            }
        }
    }

    // Return the global bounds of the mushroom's sprite (for collision detection)
    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
};

// Randomly initialize mushrooms
void initializeMushrooms(std::list<Mushroom>& mushrooms) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> xDist(0, ((WINDOW_WIDTH)/ TILE_SIZE) - 1); //uniform distribution is used to obtain values
    std::uniform_int_distribution<> yDist((TOP_MARGIN/TILE_SIZE), ((WINDOW_HEIGHT-BOTTOM_MARGIN) / TILE_SIZE) - 1);

    mushrooms.clear(); // Clear any existing mushrooms
    for (int i = 0; i < NUM_MUSHROOMS; ++i) {
        float x = xDist(gen) * TILE_SIZE;
        float y = yDist(gen) * TILE_SIZE;
        float state = 1; // state is 1 when

        mushrooms.emplace_back(x, y);
    }
}

// Spaceship class definition
class Spaceship {
public:
    sf::Sprite sprite;
    sf::Texture texture;
    bool isActive;

    Spaceship() {
        texture.loadFromFile("graphics/StarShip.png");
        sprite.setTexture(texture);
        sprite.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT - TILE_SIZE);  // Start near the bottom center of the screen
        isActive = true;
    }

    // Move the spaceship based on user input
    void move(sf::Keyboard::Key key, const std::list<Mushroom>& mushrooms) {
        if (isActive) {
            sf::Vector2f oldPosition = sprite.getPosition(); //getting the old position for collision detection
            switch (key) {
            case sf::Keyboard::Left:
                if (sprite.getPosition().x > 0)
                    sprite.move(-SPACESHIP_SPEED, 0);  // Move left
                break;
            case sf::Keyboard::Right:
                if (sprite.getPosition().x < WINDOW_WIDTH - TILE_SIZE)
                    sprite.move(SPACESHIP_SPEED, 0);  // Move right
                break;
            case sf::Keyboard::Up:
                // Check if the spaceship is above the top threshold
                if (sprite.getPosition().y > (WINDOW_HEIGHT - BOTTOM_THRESHOLD))
                    sprite.move(0, -SPACESHIP_SPEED);  // Move up, but don't exceed the threshold
                break;
            case sf::Keyboard::Down:
                if (sprite.getPosition().y < WINDOW_HEIGHT - TILE_SIZE)
                    sprite.move(0, SPACESHIP_SPEED);  // Move down
                break;
            default:
                break;
            }
            for (const auto& Mushroom : mushrooms) {
                if (sprite.getGlobalBounds().intersects(Mushroom.getBounds())) {
                    // Collision detected, move the spaceship back to the old position
                    sprite.setPosition(oldPosition);
                    break;  // Exit the loop after the first collision 
                }
            }
        }
    }
};

//Spider class
class Spider {
public:
    sf::Sprite sprite;
    sf::Texture texture;
    float speed;
    sf::Vector2f direction;

    sf::Clock directionTimer;

    Spider() {
        texture.loadFromFile("graphics/spider.png"); 
        sprite.setTexture(texture);

        // Set initial position in the bottom space
        float initialY = WINDOW_HEIGHT - BOTTOM_THRESHOLD;
        sprite.setPosition(0, initialY);

        speed = 100.0f;  // Adjust speed as needed
    }

    void changeDirection() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(-1.0, 1.0);

        // Generate random x and y components
        direction.x = dist(gen);
        direction.y = dist(gen);

        // Normalize the direction vector
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length != 0) {
            direction /= length;
        }
    }

    void update(float deltaTime) {
        // Change direction every 2 seconds
        if (directionTimer.getElapsedTime().asSeconds() > 2.0f) {
            changeDirection();
            directionTimer.restart();
        }

        // Update position
        sf::Vector2f movement = direction * speed * deltaTime;
        sprite.move(movement);

        // Check boundaries
        sf::FloatRect spiderBounds = sprite.getGlobalBounds();
        sf::Vector2f position = sprite.getPosition();

        // Left and right boundaries
        if (position.x < 0) {
            position.x = 0;
            direction.x = std::abs(direction.x);
        }
        else if (position.x + spiderBounds.width > WINDOW_WIDTH) {
            position.x = WINDOW_WIDTH - spiderBounds.width;
            direction.x = -std::abs(direction.x);
        }

        // Top and bottom boundaries of the bottom space
        float bottomSpaceTop = WINDOW_HEIGHT - BOTTOM_THRESHOLD;
        if (position.y < bottomSpaceTop) {
            position.y = bottomSpaceTop;
            direction.y = std::abs(direction.y);
        }
        else if (position.y + spiderBounds.height > WINDOW_HEIGHT) {
            position.y = WINDOW_HEIGHT - spiderBounds.height;
            direction.y = -std::abs(direction.y);
        }

        sprite.setPosition(position);

    }
    //Checking collision of spider with mushrooms
    void checkMushroomCollisions(std::list<Mushroom>& mushrooms) {
        for (auto& mushroom : mushrooms) {
            if (!mushroom.isDestroyed && sprite.getGlobalBounds().intersects(mushroom.sprite.getGlobalBounds())) { //collision detection
                mushroom.hit(); //the mushroom health decreases
            }
        }
    }
    //Checking collision with the Spaceship - reducing the number of lives
    bool checkSpaceshipCollisions(Spaceship& spaceship) {
            return sprite.getGlobalBounds().intersects(spaceship.sprite.getGlobalBounds()); //returning if it collides or no
    }
    
    // Reset the spider to its initial state
    void reset() {
        sprite.setPosition(10.f, (WINDOW_HEIGHT - BOTTOM_THRESHOLD));  // Set to its starting position
        speed = 100.0f;  // Set movement speed
        direction = sf::Vector2f(1.0f, 0.0f);  // Initial movement direction
    }
};

class ECE_LaserBlast {
public:
    // Create a rectangle shape for the Laser
    sf::RectangleShape rectangle;
    
    bool isActive; // To check if the laser is currently active

    ECE_LaserBlast() {
        isActive = false; // Initially inactive

        rectangle.setSize(sf::Vector2f(5.f, 20.f));
        rectangle.setFillColor(sf::Color::Red);
        rectangle.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT - TILE_SIZE);
    }

    void fire(float x, float y) {
        rectangle.setPosition(x, y);
        isActive = true; // Activate the laser
    }

    void update() {
        if (isActive) {
            rectangle.move(0, -LASER_SPEED); // Move the laser upwards

            // Deactivate if it moves off the screen
            if (rectangle.getPosition().y < 0) {
                isActive = false;
            }
        }
    }
    // Check for collision with another object
    bool checkCollision(const sf::Sprite& other) {
        return isActive && rectangle.getGlobalBounds().intersects(other.getGlobalBounds());
    }
};

//To update Laser values
void updateLaserBlasts(std::list<ECE_LaserBlast>& lasers) {
    for (auto it = lasers.begin(); it != lasers.end();) { //from start to end of list
        (it)->update(); //update using the update function

        // Remove inactive lasers
        if (!(it)->isActive) {
            it = lasers.erase(it); // Remove from list and update iterator
        }
        else {
            ++it; // Move to next laser
        }
    }
}
// Function to calculate the distance between two points
float distance(sf::Vector2f a, sf::Vector2f b) {
    return std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}
// Function to normalize a vector
sf::Vector2f normalize(sf::Vector2f v) {
    float mag = std::sqrt(v.x * v.x + v.y * v.y);
    if (mag == 0) return sf::Vector2f(0, 0);
    return sf::Vector2f(v.x / mag, v.y / mag);
}

class ECE_Centipede {
public:
    struct Segment {
        sf::Sprite sprite;
        bool isHead;
        sf::Vector2f direction;
    };

    std::vector<Segment> segments;
    float speed;
    sf::Texture headTexture;
    sf::Texture bodyTexture;
    int segmentSize;
    int length;

    // Constructor for the Centipede
    ECE_Centipede(int length = 10) : speed(100.0f) {
        headTexture.loadFromFile("graphics/CentipedeHead.png");
        bodyTexture.loadFromFile("graphics/CentipedeBody.png"); //Loading Head/Body textures

        // Set segment size based on the texture size
        segmentSize = headTexture.getSize().x; 

        float startX = WINDOW_WIDTH / 2; //initial position, in the middle of the screen
        float startY = TOP_MARGIN; //At the top

        // Initialize the centipede segments
        for (int i = 0; i < length; ++i) {
            Segment segment;
            segment.isHead = (i == 0);
            segment.sprite.setTexture(segment.isHead ? headTexture : bodyTexture); //set based on if 0th segment or not
            segment.sprite.setPosition(startX - (i * segmentSize), startY); //set position
            segment.direction = sf::Vector2f(1.0f, 0.0f);  // Start moving right
            segments.push_back(segment);
        }
    }

    // Update the centipede's movement
    void update(float deltaTime, const std::list<Mushroom>& mushrooms) {
        // Move the head
        sf::Vector2f newPos = segments[0].sprite.getPosition() + segments[0].direction * speed * deltaTime;

        // Check for collisions with screen edges or mushrooms
        bool needsToMoveDown = false;

        // Check for collisions with screen edges
        if (newPos.x < 0) {
            newPos.x = 0;  // Prevent moving out of left boundary
            segments[0].direction.x *= -1;  // Reverse horizontal direction
            needsToMoveDown = true;  // Move down after hitting the wall
        }
        else if (newPos.x + segmentSize > WINDOW_WIDTH) {
            newPos.x = WINDOW_WIDTH - segmentSize;  // Prevent moving out of right boundary
            segments[0].direction.x *= -1;  // Reverse horizontal direction
            needsToMoveDown = true;  // Move down after hitting the wall
        }

        // Check for collisions with mushrooms
        for (const auto& mushroom : mushrooms) {
            if (segments[0].sprite.getGlobalBounds().intersects(mushroom.getBounds())) {
                segments[0].direction.x *= -1;  // Reverse horizontal direction
                newPos.y += TILE_SIZE;  // Move down
                break;  // Stop checking once we find a mushroom
            }
        }

        // Set new head position
        segments[0].sprite.setPosition(newPos);


        // Rotate the head sprite based on direction
        float angle = (segments[0].direction.x > 0) ? 0 : 180;
        segments[0].sprite.setRotation(angle);


        // For each particle except the lead one, make it follow the one ahead
        for (int i = 1; i < length; ++i) {
            sf::Vector2f dir = segments[i - 1].sprite.getPosition() - segments[i].sprite.getPosition();
            float dist = distance(segments[i - 1].sprite.getPosition(), segments[i].sprite.getPosition());

            if (dist > segmentSize) {
                // Normalize the direction vector and move the particle towards the one ahead
                segments[i].sprite.move(normalize(dir) * (speed * deltaTime));
            }
        }
    }
};


    int main() {
        // Create a window
        sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Centipede Game", Style::Default);

        Texture textureBackground;

        // Load a graphic into the texture
        textureBackground.loadFromFile("graphics/Startup Screen BackGround.png");

        // Create a sprite
        Sprite spriteBackground;

        // Attach the texture to the sprite
        spriteBackground.setTexture(textureBackground);

        // Define the target rectangle dimensions
        sf::FloatRect targetRect(0.f, 0.f, 1024.f, 1024.f); // Position (x, y) and size (width, height)

        // Get the original size of the sprite
        sf::FloatRect originalSize = spriteBackground.getGlobalBounds();

        // Calculate scale factors
        float scaleX = targetRect.width / originalSize.width;
        float scaleY = targetRect.height / originalSize.height;

        // Set the scale of the sprite to fit the target rectangle
        spriteBackground.setScale(scaleX, scaleY);

        // Set the spriteBackground to cover the screen
        spriteBackground.setPosition(0, 0);

        // Draw some text for the score
        int score = 0;

        sf::Text scoreText;

        // We need to choose a font
        sf::Font font;
        font.loadFromFile("fonts/KOMIKAP_.ttf");

        // Set the font to our message
        scoreText.setFont(font);

        // Assign the actual message
        scoreText.setString("0");

        // Make it really big
        scoreText.setCharacterSize(50);

        // Choose a color
        scoreText.setFillColor(Color::White);

        // Position the text
        scoreText.setPosition(WINDOW_HEIGHT / 2, 0);

        // Create a list for mushrooms 
        std::list<Mushroom> mushrooms;

        // Create a spaceship
        Spaceship spaceship;

        //Create spider
        Spider spider;

        // List to hold active laser blasts
        std::list<ECE_LaserBlast> lasers;

        //creating a vector to initial spaceship position
        sf::Vector2f spaceship_position(WINDOW_WIDTH / 2, WINDOW_HEIGHT - TILE_SIZE);

        // Creating life sprites to keep track of lifes
        std::vector<sf::Sprite> lives;
        for (int i = 0; i < INITIAL_LIVES; ++i) {
            sf::Sprite lifeSprite;
            if (i < INITIAL_LIVES - 1) {
                lifeSprite.setTexture(spaceship.texture);
            }
            lifeSprite.setPosition(((WINDOW_WIDTH / 2) + (i + 5) * TILE_SIZE), 0); // Position lives in the top right corner
            lives.push_back(lifeSprite);
        }

        //Create the centipede class
        std::list<ECE_Centipede> centipedes;
        centipedes.push_back(ECE_Centipede(10));

        //remaining lives
        int remainingLives = INITIAL_LIVES;
        // Boolean to track whether the game is running
        bool isGameRunning = false;
        //To sync up
        sf::Clock clock;

        // Main game loop
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();


                // Handle key press events
                if (event.type == sf::Event::KeyPressed) {
                    if (!isGameRunning && event.key.code == sf::Keyboard::Enter) {
                        // Start the game and initialize mushrooms
                        isGameRunning = true;
                        initializeMushrooms(mushrooms);  // Randomize the mushrooms

                        std::stringstream ss;
                        ss << score;
                        scoreText.setString(ss.str());

                    }


                    if (isGameRunning) {

                        if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down) {
                            spaceship.move(event.key.code, mushrooms); //moving the spaceship
                        }

                        // Handle firing laser
                        if (event.key.code == sf::Keyboard::Space) {
                            ECE_LaserBlast laser;
                            spaceship_position = spaceship.sprite.getPosition();
                            laser.fire(spaceship_position.x, spaceship_position.y); // Fire laser from spaceship position
                            lasers.push_back(laser);

                        }





                    }

                }
            }

            if (isGameRunning) {
                float deltaTime = clock.restart().asSeconds();

                // Move the spider
                spider.update(deltaTime);

                // Update laser blasts
                updateLaserBlasts(lasers);

                //Updating centipede position
                for (auto& centipede : centipedes) {
                    centipede.update(deltaTime, mushrooms);
                }
                //Laser mushroom collision
                for (auto& laser : lasers) {
                    if (laser.isActive) {
                        // Check for collisions with each mushroom
                        for (auto& mushroom : mushrooms) {
                            if (laser.checkCollision(mushroom.sprite)) {
                                laser.isActive = false;  // Deactivate the laser upon collision
                                mushroom.hit(); // Call the hit() function on the mushroom
                                score = score + 4; //adding points
                                scoreText.setString(std::to_string(score));
                            }
                        }
                        //checking if laser hits the spider
                        if (laser.checkCollision(spider.sprite)) {
                            score = score + 300; 
                            scoreText.setString(std::to_string(score));
                            spider.reset();
                        }
                    }
                }

                //Checking if spider hits the mushrooms
                spider.checkMushroomCollisions(mushrooms);
                //Checking if spider hits the spaceship
                if (spider.checkSpaceshipCollisions(spaceship)) {
                    remainingLives--; //If so, reduce the number of lives {somehow the spider seems to hit 2 always, getting an error here}
                    if (remainingLives >= 1 && remainingLives <= INITIAL_LIVES) {
                        lives[remainingLives].setColor(sf::Color::Transparent);
                    }

                    // Check for game over
                    if (remainingLives <= 0) {
                        isGameRunning = false;
                    }
                }

                // Remove mushrooms with health <= 0
                mushrooms.remove_if([](const Mushroom& mushroom) {
                    return mushroom.health <= 0;  // Remove mushrooms if health is zero or less
                    });
            }

            window.clear();

            // Draw based on game state
            if (!isGameRunning) {
                // Show start screen
                window.draw(spriteBackground);
            }
            else {
                // Draw mushrooms in the main game
                for (const auto& mushroom : mushrooms) {
                    window.draw(mushroom.sprite);
                }
                //Draw score
                window.draw(scoreText);

                //Draw spaceship
                window.draw(spaceship.sprite);

                //Draw spider
                window.draw(spider.sprite);

                //Draw the remaining number of lives
                for (const auto& lives : lives) {
                    if (lives.getTexture())
                        window.draw(lives);
                }
                // Draw laser blasts
                for (const auto& laser : lasers) {
                    window.draw(laser.rectangle); 
                }

                // Draw centipedes
                for (auto& centipede : centipedes) {
                    for (auto& segment : centipede.segments) {
                        window.draw(segment.sprite);
                    }
                }
            }
            // Display the window contents
            window.display();
        }

        return 0;
    }
