#ifndef SUDOKU_H
#define SUDOKU_H

#include <math.h>
#include <stdio.h>
using namespace std;

#define DEBUG
#include "debug.h"

//Pair struct used for returning row, col pairs from functions
typedef struct _pair
{
	int row;
	int col;
} Pair;

class Sudoku
{
	public:
		friend class SudokuTest;

		Sudoku(int puzzle_size);
		~Sudoku();

		void Copy(const Sudoku& source);

		//Solve
		//enters the data of input_puzzle and solves
		//
		//returns true if the puzzle completed properly
		//returns false if a contradiction occurred or input_puzzle is NULL
		bool Solve(int const* const* input_puzzle)
		{
			return(SetOnly(input_puzzle) && Search());
		}

		//SetOnly
		//enters the data of input_puzzle
		//sets "obvious" values, i.e. squares with only one possibility
		//
		//returns true if all values set correctly
		//returns false if a contradiction occurred or input_puzzle is NULL
		bool SetOnly(int const* const* input_puzzle);

		//Search
		//fills unknown values by trial and error
		//
		//returns true if puzzle completed correctly
		//returns false if a contradiction occurred
		bool Search();

		//call this to get the final result
		int** get_puzzle()
		{
			return puzzle;
		}

	private:
		int kSize, kSqrtSize;
		Pair*** neighbors;
		bool*** possible_vals;
		int** num_possible_vals;
		int** puzzle;

		//Set
		//helper for SetOnly
		//assigns the square at the given row and col to the given value
		//and propagate the change
		//IMPORTANT: value is in range [1..kSize]
		//
		//returns false if a conflict occurs, true otherwise
		bool Set(int row, int col, int value);

		//RemovePossibility
		//helper for SetOnly
		//removes value as a possibility for the square at row, col
		//if this leaves one remaining possibility, sets as the value for the square
		//
		//returns false if a conflict occurs, true otherwise
		bool RemovePossibility(int row, int col, int value);

		//FindLeastSolved
		//helper for Search
		//finds the square with the most possibilities
		//
		//returns Pair with the row, col of the result
		Pair FindLeastSolved();

		//FindMostSolved
		//helper for Search
		//finds the square with the fewest possibilities
		//
		//returns Pair with the row, col of the result
		Pair FindMostSolved();

		//FindFirstUnsolved
		//helper for Search
		//finds the first square with more than one possibility
		//
		//returns Pair with the row, col of the result
		Pair FindFirstUnsolved();

		//FindMostSolvedOptimal
		//helper for Search
		//finds the square with the fewest possibilities that
		//contains the most common unsolved value
		//
		//returns Pair with row, col of the result
//		Pair FindMostSolvedOptimal(int &min_value);
		Pair FindMostSolvedOptimal();
};

#endif //SUDOKU_H
