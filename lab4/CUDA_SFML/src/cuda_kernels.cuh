/*
Author: Kamya Hari
Class: ECE6122 A
Last Date Modified: 11/08/2024
Description:
Header file for the CUDA kernel function
*/
// cuda_kernels.cuh
#ifndef CUDA_KERNELS_H
#define CUDA_KERNELS_H

#include "common.h"

void updateGameOfLife(bool* currentGrid, bool* nextGrid, int width, int height, int numThreads, MemoryType memoryType);
void cleanupGameOfLife();

#endif
