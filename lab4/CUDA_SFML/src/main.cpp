/*
Author: Kamya Hari
Class: ECE6122 A
Last Date Modified: 11/08/2024
Description:
This is the main function that displays Lab4 - using CUDA to run Game of Life. This function parses through the input arguments and generates the SFML graphics 
required to show the Game of Life in action. Kernel calls are established and the time for updating the kernel using each memory type is then printed out.
*/

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <deque>
#include <iomanip>
#include <cuda_runtime.h>
#include "cuda_kernels.cuh"
#include"common.h"

int numThreads = 32;         // Threads per block (default 32, multiple of 32)
int cellSize = 5;            // Cell size (default 5)
int windowWidth = 800;       // Window width (default 800)
int windowHeight = 600;      // Window height (default 600)

MemoryType memoryType = NORMAL;  // Memory type (default NORMAL)

// For tracking generation times (in microseconds)
std::deque<long long> generationTimes;  

// Function to parse memory type argument
MemoryType parseMemoryType(const std::string& type) {
    if (type == "NORMAL") return NORMAL;
    else if (type == "PINNED") return PINNED;
    else if (type == "MANAGED") return MANAGED;
    else throw std::invalid_argument("Invalid memory type. Use NORMAL, PINNED, or MANAGED.");
}

// Array with enum names
const std::string memStrings[] = { "Normal", "Pinned", "Managed" };
std::string memToString(MemoryType mem) {
    return memStrings[mem];
}

// Function to parse command-line arguments
void parseArguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            numThreads = std::atoi(argv[++i]);
            if (numThreads < 32 || numThreads % 32 != 0) {
                throw std::invalid_argument("Number of threads (-n) must be >= 32 and a multiple of 32.");
            }
        }
        else if (std::strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cellSize = std::atoi(argv[++i]);
            if (cellSize < 1) {
                throw std::invalid_argument("Cell size (-c) must be >= 1.");
            }
        }
        else if (std::strcmp(argv[i], "-x") == 0 && i + 1 < argc) {
            windowWidth = std::atoi(argv[++i]);
            if (windowWidth < 1) {
                throw std::invalid_argument("Window width (-x) must be >= 1.");
            }
        }
        else if (std::strcmp(argv[i], "-y") == 0 && i + 1 < argc) {
            windowHeight = std::atoi(argv[++i]);
            if (windowHeight < 1) {
                throw std::invalid_argument("Window height (-y) must be >= 1.");
            }
        }
        else if (std::strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            memoryType = parseMemoryType(argv[++i]);
        }
        else {
            throw std::invalid_argument("Unknown argument or missing value.");
        }
    }
}


// Function to compute and print the average processing time for the last 100 generations
void printAverageGenerationTime() {
    if (generationTimes.empty()) return;

    long long sum = 0;
    for (long long time : generationTimes) {
        sum += time;
    }

    //long long average = sum / generationTimes.size();

    // Output the average time (in microseconds)
    std::cout << "100 generations took " << sum << " microsecs with " << numThreads << " threads per block using " <<memToString(memoryType) << " memory allocation." << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        parseArguments(argc, argv);

        // Set up SFML window
        sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Game of Life");
        window.setFramerateLimit(120);  // Set frame rate to control speed

        int gridWidth = windowWidth / cellSize;
        int gridHeight = windowHeight / cellSize;

        // Allocate memory based on memory type
        bool* grid_current;
        bool* grid_next;

        switch (memoryType) {
        case NORMAL:
            grid_current = new bool[gridWidth * gridHeight];
            grid_next = new bool[gridWidth * gridHeight];
            break;

        case PINNED:
            cudaMallocHost(&grid_current, gridWidth * gridHeight * sizeof(bool));
            cudaMallocHost(&grid_next, gridWidth * gridHeight * sizeof(bool));
            break;

        case MANAGED:
            cudaMallocManaged(&grid_current, gridWidth * gridHeight * sizeof(bool));
            cudaMallocManaged(&grid_next, gridWidth * gridHeight * sizeof(bool));
            break;
        }

        // Seed the grid with random values
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        for (int i = 0; i < gridWidth * gridHeight; ++i) {
            grid_current[i] = std::rand() % 2;
        }

        // Allocate and initialize device memory for the grid
        bool* d_grid_current;
        bool* d_grid_next;

        // Only allocate separate device memory if not using managed memory
        if (memoryType != MANAGED) {
            cudaMalloc(&d_grid_current, gridWidth * gridHeight * sizeof(bool));
            cudaMalloc(&d_grid_next, gridWidth * gridHeight * sizeof(bool));

            // Copy the initial state to device
            cudaMemcpy(d_grid_current, grid_current,
                gridWidth * gridHeight * sizeof(bool),
                cudaMemcpyHostToDevice);
        }
        else {
            // For managed memory, use the same pointers
            d_grid_current = grid_current;
            d_grid_next = grid_next;
        }

        // Main loop
        int generationCount = 0; // To count the number of generations
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            // Measure the time for kernel execution (excluding rendering)
            auto startKernel = std::chrono::high_resolution_clock::now();

            // Launch the CUDA kernel to update the grid

            updateGameOfLife(grid_current, grid_next, gridWidth, gridHeight, numThreads, memoryType);

            auto endKernel = std::chrono::high_resolution_clock::now();
            auto kernelDuration = std::chrono::duration_cast<std::chrono::microseconds>(endKernel - startKernel).count();

            // Measure the time for copying the data from the device to the host
            auto startMemcpy = std::chrono::high_resolution_clock::now();

            // Copy updated state back to current grid
            std::memcpy(grid_current, grid_next, gridWidth * gridHeight * sizeof(bool));

            auto endMemcpy = std::chrono::high_resolution_clock::now();
            auto memcpyDuration = std::chrono::duration_cast<std::chrono::microseconds>(endMemcpy - startMemcpy).count();

            // Total time for the generation (kernel + memory copy)
            long long totalGenerationTime = kernelDuration + memcpyDuration;

            // Add the generation time to the list
            generationTimes.push_back(totalGenerationTime);

            // Limit the number of times we store to 100 generations
            if (generationTimes.size() > 100) {
                generationTimes.pop_front();
            }

            // Clear the window
            window.clear();

            // Draw the grid using SFML
            for (int x = 0; x < gridWidth; ++x) {
                for (int y = 0; y < gridHeight; ++y) {
                    if (grid_current[y * gridWidth + x]) {
                        sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
                        cell.setPosition(x * cellSize, y * cellSize);
                        cell.setFillColor(sf::Color::White);
                        window.draw(cell);
                    }
                }
            }

            // Display the updated window
            window.display();

            // Print the generation time every 100 generations
            if (++generationCount % 100 == 0) {
                printAverageGenerationTime();
            }
        }

        cleanupGameOfLife();


    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
