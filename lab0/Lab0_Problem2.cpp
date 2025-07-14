/*
Author: Kamya Hari
Class: ECE 6122
Last Date Modified: 09-03-2024

Description:

This program takes in any natural number and outputs the sum of primes less than or equal to the number.
*/

#include <iostream>
#include<limits>
#include<cctype>
#include<string>

int numberIsAPrime(int givenNumber) // This function checks if a given number is a prime number - input is the given integer - returns 0 if the number is not prime, 1 if the number is prime
{
	if (givenNumber < 2) //Prime numbers start from 2
	{
		return 0;
	}
	for (int i = 2; i < givenNumber; i++) // This for loop iterates over the preceding numbers to check if the given number is divisible by a preceding number
	{
		if (givenNumber % i == 0) //If the number has a divisor other than 1 or itself, it's not a prime number
		{
			return 0;
		}
	}
	return 1;
}

int sumOfPrimes(int givenNumber) //This function computes the sum of all the preceding prime numbers to the given number -input is the given integer - the output is an integer that is the sum of all primes before the given number
{
	int sumOfPrimesValue = 0;
	for (int i = 2; i <= givenNumber; i++)
	{
		if (numberIsAPrime(i))
		{
			sumOfPrimesValue += i; //Loops through the preceding numbers and checks for prime numbers - if found, summed with the previous value of sumOfPrimesValue variable
		}
	}
	return sumOfPrimesValue;
}

int inputValidityfn(const std::string& input) //to check if the given input is valid - input is the input string from the user - output is 0 if the input is invalid, 1 if it is valid
{
	if (input.empty()) //checking for empty input
	{
		return 0;
	}
	for (char c : input)
	{
		if (!isdigit(c)) //checking if each character of the string is a numeric value
		{
			return 0;
		}
	}
	return 1;
}

int main(/* The code to execute the previous functions and outputs the sum of primes value to the user*/)
{
	while (true)
	{
		std::string inputNumberString;
		std::cout << "Please enter a natural number (0 to quit):"; //Ask the user to enter the number
		std::cin >> inputNumberString; //getting the input from the user
		if (inputNumberString == "0") //To terminate the program
		{
			std::cout << "Program Terminated. \nHave a nice day!";
			break;
		}
		else if (!inputValidityfn(inputNumberString)) //conditional to check the validity of the input
		{
			std::cout << "Error! Invalid input!\n";
		}
		else
		{
			try // try block to handle the input<2^32 criteria
			{
				int inputNumber = std::stoi(inputNumberString);

				if (inputNumber >= UINT32_MAX)
				{
					;
				}
				int sumOfPrimesValue = 0;
				sumOfPrimesValue = sumOfPrimes(inputNumber); //computes the sum of primes for the correct input
				std::cout << "The sum of the primes is " << sumOfPrimesValue << "\n"; //outputs the sum of primes to the user
			}
			catch (const std::out_of_range&) //outputs the invalid error statement
			{
				std::cout << "Error! Invalid input!\n";
			}
		}
	}
}