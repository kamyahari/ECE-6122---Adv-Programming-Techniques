/*
Author: Kamya Hari
Class: ECE 6122
Date: 10-11-2024

Description: Running John Conway's Game of Life using Multithreading methods. This function takes in command line arguments
for the window height, width, pixel size, number of threads and which type of threading to use. 
Three methods are implemented: Sequential processing, Multithreading using std::thread and Multithreading using OpenMP.
*/

#include <SFML/Graphics.hpp>
#include <array>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <omp.h>
#include<cstring>

//Global variables
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int PIXEL_SIZE = 5;
int GRID_WIDTH = WINDOW_WIDTH / PIXEL_SIZE;
int GRID_HEIGHT = WINDOW_HEIGHT / PIXEL_SIZE;
int NUM_OF_THREADS = 8;
std::string processingType = "SEQ";  

using Grid = std::vector<std::vector<bool>>; //creating a boolean vector of vectors

void seedRandomGrid(Grid& grid) //Randomly seed the array to start the game; input is the reference to the vector
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int x = 0; x < GRID_WIDTH; ++x)
    {
        for (int y = 0; y < GRID_HEIGHT; ++y)
        {
            grid[x][y] = (std::rand() % 2 == 0);  // Randomly seed each pixel
        }
    }
}

int countNeighbors(const std::vector<std::vector<bool>>& grid, int x, int y) //Calculates number of live 8-neighbors
{
    /* Input: Vector containing the information about each pixel, coordinates of position x and y*/
    int count = 0;
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            if (i == 0 && j == 0)
            {
                continue;
            }
            int nx = (x + i + GRID_WIDTH) % GRID_WIDTH;
            int ny = (y + j + GRID_HEIGHT) % GRID_HEIGHT;
            count += grid[nx][ny];
        }
    }
    return count;
}
int countNeighborsOMP(const std::vector<std::vector<bool>>& grid, int x, int y) //Function to perform counting parallely using OpenMP
{
    int count = 0;
    #pragma omp parallel for collapse(2) schedule(static) //using pragma statement
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            if (i == 0 && j == 0)
            {
                continue;
            }
            int nx = (x + i + GRID_WIDTH) % GRID_WIDTH;
            int ny = (y + j + GRID_HEIGHT) % GRID_HEIGHT;
            count += grid[nx][ny];
        }
    }
    return count;
}


void updateGridSEQ(Grid& grid, Grid& newGrid) //function to update the network sequentially 
{
    for (int x = 0; x < GRID_WIDTH; ++x)
    {
        for (int y = 0; y < GRID_HEIGHT; ++y)
        {
            int neighbors = countNeighbors(grid, x, y);

            if (grid[x][y])
            {
                newGrid[x][y] = !(neighbors < 2 || neighbors > 3);  // Cell survives
            }
            else
            {
                newGrid[x][y] = (neighbors == 3);  // Cell becomes alive
            }
        }
    }
}

void updateGridOMP(Grid& grid, Grid& newGrid) //Function to update the network using OpenMP
{
    #pragma omp parallel for collapse(2) schedule(static)
    for (int x = 0; x < GRID_WIDTH; ++x)
    {
        for (int y = 0; y < GRID_HEIGHT; ++y)
        {
            int neighbors = countNeighborsOMP(grid, x, y);

            if (grid[x][y])
            {
                newGrid[x][y] = !(neighbors < 2 || neighbors > 3);  // Cell survives
            }
            else
            {
                newGrid[x][y] = (neighbors == 3);  // Cell becomes alive
            }
        }
    }
}

// Thread function to update a portion of the grid
void updateGridSection(const Grid& grid, Grid& newGrid, int startRow, int endRow) {
    for (int x = startRow; x < endRow; ++x) {
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            int neighbors = countNeighbors(grid, x, y);

            // Apply Game of Life rules
            newGrid[x][y] = (grid[x][y] && (neighbors == 2 || neighbors == 3)) ||
                (!grid[x][y] && neighbors == 3);
        }
    }
}

void updateGridTHRD(Grid& grid, Grid& newGrid) { //Function to update Grids by using std::thread
    std::vector<std::thread> threads;
    int rowsPerThread = GRID_WIDTH / NUM_OF_THREADS;

    for (int i = 0; i < NUM_OF_THREADS; ++i) {
        int startRow = i * rowsPerThread;
        int endRow = (i == NUM_OF_THREADS - 1) ? GRID_WIDTH : startRow + rowsPerThread;
        threads.emplace_back(updateGridSection, std::cref(grid), std::ref(newGrid), startRow, endRow);
    }

    for (auto& t : threads) {
        //if (t.joinable()) {
            t.join(); //Join all the threads together
        //}
    }
}

// Function to parse command-line arguments
void parseArguments(int argc, char* argv[]) {

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            PIXEL_SIZE = std::atoi(argv[++i]); // Get the next argument as pixel size
        }
        else if (strcmp(argv[i], "-x") == 0 && i + 1 < argc) {
            WINDOW_WIDTH = std::atoi(argv[++i]); // Get the next argument as window width
        }
        else if (strcmp(argv[i], "-y") == 0 && i + 1 < argc) {
            WINDOW_HEIGHT = std::atoi(argv[++i]); // Get the next argument as window height
        }
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            processingType = argv[++i]; // Get the next argument as processing type 
        }
        else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            NUM_OF_THREADS = std::atoi(argv[++i]); // Get the next argument as number of threads
        }
    }
    GRID_WIDTH = WINDOW_WIDTH / PIXEL_SIZE; //update grid sizes
    GRID_HEIGHT = WINDOW_HEIGHT / PIXEL_SIZE;
}

int main(int argc, char* argv[])
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Game of Life");
    window.setFramerateLimit(120);

    // Parse command line arguments
    parseArguments(argc, argv);

    /*sanity checks
    std::cout << NUM_OF_THREADS << std::endl;
    std::cout << processingType << std::endl;
    std::cout << WINDOW_HEIGHT << std::endl;
    std::cout << WINDOW_WIDTH << std::endl;
    std::cout << PIXEL_SIZE << std::endl;
    */

    Grid grid_current(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));  // Initialize the arrays
    Grid grid_next(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));

    seedRandomGrid(grid_current); //Random instantiation

    std::chrono::duration<double, std::micro> duration; //variables to calculate the time taken
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_stop = std::chrono::high_resolution_clock::now();
    double time100Gen = 0.0;
    unsigned long numGenerations = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        if (processingType == "SEQ")  //For sequential processing
        {
            t_start = std::chrono::high_resolution_clock::now();
            updateGridSEQ(grid_current, grid_next);
            t_stop = std::chrono::high_resolution_clock::now();
            duration = t_stop - t_start;
            time100Gen += duration.count();
        }
        else if (processingType == "THRD") //For std::thread
        {
            t_start = std::chrono::high_resolution_clock::now();
            updateGridTHRD(grid_current, grid_next);
            t_stop = std::chrono::high_resolution_clock::now();
            duration = t_stop - t_start;
            time100Gen += duration.count();
        }
        else if(processingType == "OMP") //For OpenMP
        {
            t_start = std::chrono::high_resolution_clock::now();
            updateGridOMP(grid_current, grid_next);
            t_stop = std::chrono::high_resolution_clock::now();
            duration = t_stop - t_start;
            time100Gen += duration.count();
        }
        
        std::swap(grid_current, grid_next);  // Just swap the grids to avoid copying

        if (numGenerations % 100 == 0 && numGenerations != 0) //For every 100 Generations
        {
            std::cout << "Time for 100 generations: " << time100Gen << " microseconds" << std::endl; //Print the time taken
	    time100Gen = 0.0; //Reinitialize time
        }

        numGenerations++;

        window.clear();

        for (int x = 0; x < GRID_WIDTH; ++x) //Render the graphics window to show game movement
        {
            for (int y = 0; y < GRID_HEIGHT; ++y)
            {
                if (grid_next[x][y])
                {
                    sf::RectangleShape cell(sf::Vector2f(PIXEL_SIZE, PIXEL_SIZE));
                    cell.setPosition(x * PIXEL_SIZE, y * PIXEL_SIZE);
                    cell.setFillColor(sf::Color::White);
                    window.draw(cell);
                }
            }
        }

        window.display();
    }

    return 0;
}
