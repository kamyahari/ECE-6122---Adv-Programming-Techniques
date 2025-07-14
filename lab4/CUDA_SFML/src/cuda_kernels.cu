/*
Author: Kamya Hari
Class: ECE6122 A
Last Date Modified: 11/08/2024
Description:
Cuda kernel function to update the grids in Game of Life
*/

#include "cuda_kernels.cuh"
#include <cuda_runtime.h>
#include <iostream>

// Static variables for device memory management
static bool* d_current = nullptr;
static bool* d_next = nullptr;
static size_t last_size = 0;

__global__ void updateGameOfLifeKernel(const bool* currentGrid, bool* nextGrid, int width, int height) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= width * height) return;

    int x = index % width;   // Column
    int y = index / width;   // Row
    int count = 0;

    // Count live neighbors
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;

            int nx = x + i;
            int ny = y + j;

            // Check if neighbor indices are within bounds
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                count += currentGrid[ny * width + nx];
            }
        }
    }

    // Apply Game of Life rules
    nextGrid[index] = (currentGrid[index] && (count == 2 || count == 3)) || 
                     (!currentGrid[index] && count == 3);
}

void cleanupGameOfLife() {
    if (d_current != nullptr) {
        cudaFree(d_current);
        cudaFree(d_next);
        d_current = nullptr;
        d_next = nullptr;
        last_size = 0;
    }
}

void updateGameOfLife(bool* currentGrid, bool* nextGrid, int width, int height, int numThreads, MemoryType memoryType) {
    cudaError_t err;
    size_t size = width * height * sizeof(bool);
    
    // Calculate grid dimensions
    int threadsPerBlock = numThreads;
    int blocksPerGrid = (width * height + threadsPerBlock - 1) / threadsPerBlock;
    
    switch(memoryType) {
        case NORMAL: {
            // Regular CUDA memory allocation and transfers
            err = cudaMalloc(&d_current, size);
            if (err != cudaSuccess) {
                std::cerr << "NORMAL: Failed to allocate d_current: " << cudaGetErrorString(err) << std::endl;
                return;
            }
            
            err = cudaMalloc(&d_next, size);
            if (err != cudaSuccess) {
                std::cerr << "NORMAL: Failed to allocate d_next: " << cudaGetErrorString(err) << std::endl;
                cudaFree(d_current);
                return;
            }
            
            err = cudaMemcpy(d_current, currentGrid, size, cudaMemcpyHostToDevice);
            if (err != cudaSuccess) {
                std::cerr << "NORMAL: Failed to copy to device: " << cudaGetErrorString(err) << std::endl;
                cudaFree(d_current);
                cudaFree(d_next);
                return;
            }
            
            updateGameOfLifeKernel<<<blocksPerGrid, threadsPerBlock>>>(d_current, d_next, width, height);
            cudaDeviceSynchronize();
            
            err = cudaMemcpy(nextGrid, d_next, size, cudaMemcpyDeviceToHost);
            if (err != cudaSuccess) {
                std::cerr << "NORMAL: Failed to copy from device: " << cudaGetErrorString(err) << std::endl;
            }
            
            cudaFree(d_current);
            cudaFree(d_next);
            break;
        }
        
        case PINNED: {
            // Create pinned memory buffers for efficient transfers
            bool* h_pinned_current;
            bool* h_pinned_next;
            
            err = cudaHostAlloc(&h_pinned_current, size, cudaHostAllocDefault);
            if (err != cudaSuccess) {
                std::cerr << "PINNED: Failed to allocate pinned current buffer: " << cudaGetErrorString(err) << std::endl;
                return;
            }
            
            err = cudaHostAlloc(&h_pinned_next, size, cudaHostAllocDefault);
            if (err != cudaSuccess) {
                std::cerr << "PINNED: Failed to allocate pinned next buffer: " << cudaGetErrorString(err) << std::endl;
                cudaFreeHost(h_pinned_current);
                return;
            }
            
            // Copy input data to pinned memory
            memcpy(h_pinned_current, currentGrid, size);
            
            // Allocate device memory
            err = cudaMalloc(&d_current, size);
            if (err != cudaSuccess) {
                std::cerr << "PINNED: Failed to allocate d_current: " << cudaGetErrorString(err) << std::endl;
                cudaFreeHost(h_pinned_current);
                cudaFreeHost(h_pinned_next);
                return;
            }
            
            err = cudaMalloc(&d_next, size);
            if (err != cudaSuccess) {
                std::cerr << "PINNED: Failed to allocate d_next: " << cudaGetErrorString(err) << std::endl;
                cudaFree(d_current);
                cudaFreeHost(h_pinned_current);
                cudaFreeHost(h_pinned_next);
                return;
            }
            
            // Transfer from pinned memory to device
            err = cudaMemcpy(d_current, h_pinned_current, size, cudaMemcpyHostToDevice);
            if (err != cudaSuccess) {
                std::cerr << "PINNED: Failed to copy to device: " << cudaGetErrorString(err) << std::endl;
                cudaFree(d_current);
                cudaFree(d_next);
                cudaFreeHost(h_pinned_current);
                cudaFreeHost(h_pinned_next);
                return;
            }
            
            updateGameOfLifeKernel<<<blocksPerGrid, threadsPerBlock>>>(d_current, d_next, width, height);
            cudaDeviceSynchronize();
            
            // Transfer result to pinned memory
            err = cudaMemcpy(h_pinned_next, d_next, size, cudaMemcpyDeviceToHost);
            if (err != cudaSuccess) {
                std::cerr << "PINNED: Failed to copy from device: " << cudaGetErrorString(err) << std::endl;
            }
            
            // Copy result from pinned memory to output buffer
            memcpy(nextGrid, h_pinned_next, size);
            
            // Cleanup
            cudaFree(d_current);
            cudaFree(d_next);
            cudaFreeHost(h_pinned_current);
            cudaFreeHost(h_pinned_next);
            break;
        }
        
        case MANAGED: {
            // Use CUDA managed memory
            err = cudaMallocManaged(&d_current, size);
            if (err != cudaSuccess) {
                std::cerr << "MANAGED: Failed to allocate managed memory for d_current: " << cudaGetErrorString(err) << std::endl;
                return;
            }
            
            err = cudaMallocManaged(&d_next, size);
            if (err != cudaSuccess) {
                std::cerr << "MANAGED: Failed to allocate managed memory for d_next: " << cudaGetErrorString(err) << std::endl;
                cudaFree(d_current);
                return;
            }
            
            // Copy initial state
            memcpy(d_current, currentGrid, size);
            
            updateGameOfLifeKernel<<<blocksPerGrid, threadsPerBlock>>>(d_current, d_next, width, height);
            err = cudaDeviceSynchronize();
            if (err != cudaSuccess) {
                std::cerr << "MANAGED: Kernel execution failed: " << cudaGetErrorString(err) << std::endl;
            }
            
            // Copy result back
            memcpy(nextGrid, d_next, size);
            
            // Cleanup
            cudaFree(d_current);
            cudaFree(d_next);
            break;
        }
    }
    
    // Check for kernel errors
    err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << "Kernel execution failed: " << cudaGetErrorString(err) << std::endl;
        return;
    }
}