#include <math.h>
#include <stdio.h>

#include "sudoku.h"

int n;

int** read_file(char* filename)
{
	FILE* input = fopen(filename, "r");

	fscanf(input, "%d", &n);

	int** read_values = new int* [n];
	for(int row = 0; row < n; row++)
	{
		read_values[row] = new int[n];
		for(int col = 0; col < n; col++)
		{
			fscanf(input, "%d", &(read_values[row][col]));
		}
	}

	fclose(input);

	return read_values;
}

void print_solution(int** solution)
{
	if(!solution) return;

	int sqrt_n = (int) sqrt(n);

	for(int row = 0; row < n; row++)
	{
		for(int col = 0; col < n; col++)
		{
			printf("%d ", solution[row][col]);
			if((col+1)%sqrt_n == 0 && col+1 != n)
				printf(" ");
		}
		printf("\n");
		if((row+1)%sqrt_n == 0 && row+1 != n)
			printf("\n");
	}
}

int main(int argc, char* argv[])
{
	if(argc == 1)
	{
		printf("Please give a filename for the puzzle.\n");
		return 1;
	}

	char* filename = argv[1];

	int** puzzle = read_file(filename);
	printf("\ninput puzzle\n");
	print_solution(puzzle);

	Sudoku solver(n);
	solver.Solve(puzzle);
	int** solution = solver.get_puzzle();
	printf("\nsolution\n");
	print_solution(solution);

	for(int row = 0; row < n; row++)
	{
		delete[] puzzle[row];
	}
	delete[] puzzle;

	return 0;
}
