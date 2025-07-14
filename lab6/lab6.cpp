/*
Author: Kamya Hari
Course: ECE 6122 A
Last Date Modified: 11/27/2024

Description:
This program computes two integrals (1 or 2) chosen by the user using Monte carlo estimation. This program also uses OpenMPI to distribute the work across available processors. 
The program takes in two inputs - P: 1 or 2, to choose the intergral we want to compute; N is the number of random samples to be generated in total that would be distributed across
processors.
*/

#include <mpi.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

//Integral 1 - computes integral 1 function
double func1(double x) {
    return x * x;
}

//Integral 2 - computes integral 2 function
double func2(double x) {
    return exp(-x * x);
}

//Monte carlo function - takes in the function, limit values and the number of samples for which to compute the integral; outputs the integral estimate
double monteCarlo(double (*func)(double), double a, double b, int numSamples) {
    double sum = 0.0;
    for (int i = 0; i < numSamples; ++i) {
        double x = a + (b - a) * ((double) rand() / RAND_MAX);
        sum += func(x);
    }
    return (b - a) * sum / numSamples;
}

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv); //Initialize MPI

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Process command-line arguments
    if (argc != 5) {
        if (rank == 0) {
            std::cerr << "Usage: " << argv[0] << " -P <1|2> -N <numSamples>\n";
        }
        MPI_Finalize();
        return 1;
    }

    //Parse through the command line values
    int P = atoi(argv[2]);
    int N = atoi(argv[4]);
    int localSamples = N / size;
    double a = 0.0, b = 1.0; //Integral range

    // Select the function
    double (*selectedFunc)(double) = (P == 1) ? func1 : func2;

    // Seed random number generator
    srand(time(nullptr) + rank);

    // Perform local computation
    double localResult = monteCarlo(selectedFunc, a, b, localSamples);

    // Gather results from all processes
    double globalResult = 0.0;
    MPI_Reduce(&localResult, &globalResult, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Output the result
    if (rank == 0) {
        std::cout << "The estimate for integral " << P << " is " << globalResult / size << "\n";
	 std::cout<<"Bye!"<<std::endl;
    }

    MPI_Finalize();
    return 0;
}
