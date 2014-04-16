#include "sudoku.h"

Sudoku::Sudoku(int puzzle_size)
{
	kSize = puzzle_size;
	kSqrtSize = (int) sqrt(kSize);

	puzzle = new int* [kSize];
	num_possible_vals = new int* [kSize];
	possible_vals = new bool** [kSize];

	for(int row = 0; row < kSize; row++)
	{
		puzzle[row] = new int[kSize];
		num_possible_vals[row] = new int[kSize];
		possible_vals[row] = new bool* [kSize];

		for(int col = 0; col < kSize; col++)
		{
			puzzle[row][col] = 0;
			num_possible_vals[row][col] = kSize;
			possible_vals[row][col] = new bool[kSize];

			for(int value = 0; value < kSize; value++)
			{
				possible_vals[row][col][value] = true;
			}
		}
	}
}

Sudoku::~Sudoku()
{
	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			delete[] possible_vals[row][col];
		}
		
		delete[] puzzle[row];
		delete[] num_possible_vals[row];
		delete[] possible_vals[row];
	}

	delete[] puzzle;
	delete[] num_possible_vals;
	delete[] possible_vals;
}

void Sudoku::Copy(const Sudoku& source)
{
	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			puzzle[row][col] = source.puzzle[row][col];
			num_possible_vals[row][col] = source.num_possible_vals[row][col];
			for(int value = 0; value < kSize; value++)
			{
				possible_vals[row][col][value] = source.possible_vals[row][col][value];
			}
		}
	}
}

//SetOnly
//enters the data of input_puzzle
//sets "obvious" values, i.e. squares with only one possibility
//
//returns true if all values set correctly
//returns false if a contradiction occurred or input_puzzle is NULL
bool Sudoku::SetOnly(int const* const* input_puzzle)
{
	if(!input_puzzle)
	{
		DEBUG_OUT("error: input_puzzle is NULL");
		return false;
	}

	//reinitialize array values
	//(failsafe in case someone tries solving a new puzzle in an already modified object)
	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			puzzle[row][col] = 0;

			for(int value = 0; value < kSize; value++)
			{
				possible_vals[row][col][value] = true;
				num_possible_vals[row][col] = kSize;
			}
		}
	}

	//set values from input_puzzle
	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			if(!Set(row, col, input_puzzle[row][col]))
			{
				DEBUG_OUT("error: puzzle not valid\n");
				DEBUG_OUT("row %d, col %d, value %d\n",
						row, col, input_puzzle[row][col]);
				return false;
			}
		}
	}

	return true;
}

//Set
//helper for SetOnly
//assigns the square at the given row and col to the given value
//and propagate the change
//IMPORTANT: value is in range [1..kSize]
//
//returns false if a conflict occurs, true otherwise
bool Sudoku::Set(int row, int col, int value)
{
	//if value is 0, skip setting this square
	if(value == 0) return true;

	//if value is not a possibility for (row, col) there is a conflict
	if(!possible_vals[row][col][value-1])
	{
		DEBUG_OUT("error: conflict setting\n");
		DEBUG_OUT("row %d, col %d, value %d\n", row, col, value);
		
		for(int v = 0; v < kSize; v++)
		{
			DEBUG_OUT("%d", possible_vals[row][col][v]);
		}
		DEBUG_OUT("\n");

		return false;
	}

	//make value the only possibility for (row, col)
	puzzle[row][col] = value;
	num_possible_vals[row][col] = 1;
	for(int v = 0; v < kSize; v++)
	{
		if(v != value-1)
			possible_vals[row][col][v] = false;
	}

	//remove value as a possibility for all squares logically adjacent
	//to (row, col), i.e. same row, col, or block
	
	//same col
	for(int r = 0; r < kSize; r++)
	{
		if(r != row)
			if(!RemovePossibility(r, col, value))
				return false;
	}

	//same row
	for(int c = 0; c < kSize; c++)
	{
		if(c != col)
			if(!RemovePossibility(row, c, value))
				return false;
	}

	//same block
	//this math works because the block has height/width of kSqrtSize
	int block_row = (int)(kSqrtSize * floor(row / kSqrtSize));
	int block_col = (int)(kSqrtSize * floor(col / kSqrtSize));
	for(int r = block_row; r < block_row + kSqrtSize; r++)
	{
		for(int c = block_col; c < block_col + kSqrtSize; c++)
		{
			if(r != row && c != col)
				if(!RemovePossibility(r, c, value))
					return false;
		}
	}

	return true;
}

//RemovePossibility
//helper for SetOnly
//removes value as a possibility for the square at row, col
//if this leaves one remaining possibility, sets as the value for the square
//
//returns false if a conflict occurs, true otherwise
bool Sudoku::RemovePossibility(int row, int col, int value)
{
	//if value is already not a possibility no change is needed
	if(!possible_vals[row][col][value-1])
		return true;

	//if this square is already solved (i.e. value is the only possibility),
	//there is a contradiction
	if(num_possible_vals[row][col] == 1)
	{
		DEBUG_OUT("error: RemovePossibility conflict\n");
		DEBUG_OUT("row %d, col %d, value %d\n", row, col, value);
		return false;
	}

	possible_vals[row][col][value-1] = false;
	num_possible_vals[row][col]--;

	if(num_possible_vals[row][col] == 1)
	{
		//find the one possible value and set it
		for(int v = 0; v < kSize; v++)
		{
			if(possible_vals[row][col][v])
				return Set(row, col, v+1);
		}
	}

	return true;
}

//Search
//fills unknown values by trial and error
//
//returns true if puzzle completed correctly
//returns false if a contradiction occurred
bool Sudoku::Search()
{
	//pick an unsolved square
	Pair search_target = FindFirstUnsolved();
	int search_row = search_target.row;
	int search_col = search_target.col;

	//base case: no unsolved square
	if(search_row == -1)
	{
		DEBUG_OUT("Search base case reached.\n");
		return true;
	}

	//try all possibilities -- break loop if success
	for(int value = 0; value < kSize; value++)
	{
		DEBUG_OUT("searching on %d, %d; value %d\n", search_row, search_col, value+1);

		if(!possible_vals[search_row][search_col][value])
		{
			continue;
		}
		Sudoku current_branch(kSize);
		current_branch.Copy(*this);
		if(current_branch.Set(search_row, search_col, value+1) && 
		   current_branch.Search())
		{
			Copy(current_branch);
			return true;
		}
	}

	//if no possibilities work, there must be a contradiction
	return false;
}

//FindLeastSolved
//helper for Search
//finds the square with the most possibilities
//
//returns Pair with the row, col of the result
Pair Sudoku::FindLeastSolved()
{
	int max = 0;
	Pair result;
	result.row = -1;
	result.col = -1;

	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			if(num_possible_vals[row][col] > max)
			{
				result.row = row;
				result.col = col;
				max = num_possible_vals[row][col];
			}
		}
	}

	return result;
}

//FindMostSolved
//helper for Search
//finds the square with the fewest possibilities
//
//returns Pair with the row, col of the result
Pair Sudoku::FindMostSolved()
{
	int min = kSize+1;
	Pair result;
	result.row = -1;
	result.col = -1;

	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			int current = num_possible_vals[row][col];
			if(current < min && current > 1)
			{
				result.row = row;
				result.col = col;
				min = num_possible_vals[row][col];
			}
		}
	}

	return result;
}

//FindFirstUnsolved
//helper for Search
//finds the first square with more than one possibility
//
//returns Pair with the row, col of the result
Pair Sudoku::FindFirstUnsolved()
{
	Pair result;
	result.row = -1;
	result.col = -1;

	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			if(num_possible_vals[row][col] > 1)
			{
				result.row = row;
				result.col = col;
				return result;
			}
		}
	}

	return result;
}
