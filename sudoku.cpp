#include "sudoku.h"

Sudoku::Sudoku(int puzzle_size)
{
	kSize = puzzle_size;
	kSqrtSize = (int) sqrt(kSize);

	puzzle = new int* [kSize];
	num_possible_vals = new int* [kSize];
	possible_vals = new bool** [kSize];
	neighbors = new Pair** [kSize];

	for(int row = 0; row < kSize; row++)
	{
		puzzle[row] = new int[kSize];
		num_possible_vals[row] = new int[kSize];
		possible_vals[row] = new bool* [kSize];
		neighbors[row] = new Pair* [kSize];

		for(int col = 0; col < kSize; col++)
		{
			puzzle[row][col] = 0;
			num_possible_vals[row][col] = kSize;
			possible_vals[row][col] = new bool[kSize];
			neighbors[row][col] = new Pair[kSize * 3];

			for(int value = 0; value < kSize; value++)
			{
				possible_vals[row][col][value] = true;
			}

			//populate neighbors for this cell
			//this contains duplicates but that shouldn't be an issue
			//this also contains each cell as its own neighbor, which is worse
			int block_row = kSqrtSize * floor(row / kSqrtSize);
			int block_col = kSqrtSize * floor(col / kSqrtSize);
			for(int i = 0; i < kSize; i++)
			{
				//0 to (N-1): same row
				neighbors[row][col][i].row = row;
				neighbors[row][col][i].col = i;

				//N to (2N-1): same col
				neighbors[row][col][i + kSize].row = i;
				neighbors[row][col][i + kSize].col = col;

				//2N to (3N-1): same box, not same row or col
				neighbors[row][col][i + 2 * kSize].row = block_row + i / kSqrtSize;
				neighbors[row][col][i + 2 * kSize].col = block_col + i % kSqrtSize;
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
			delete[] neighbors[row][col];
		}

		delete[] puzzle[row];
		delete[] num_possible_vals[row];
		delete[] possible_vals[row];
		delete[] neighbors[row];
	}

	delete[] puzzle;
	delete[] num_possible_vals;
	delete[] possible_vals;
	delete[] neighbors;
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
				DEBUG_OUT("error: puzzle not valid: ");
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
	for(int i = 0; i < kSize * 3; i++)
	{
		Pair curr_neighbor = neighbors[row][col][i];
		int r = curr_neighbor.row;
		int c = curr_neighbor.col;
		if((r != row || c != col) &&
			!RemovePossibility(r, c, value))
			return false;
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
		DEBUG_OUT("error: RemovePossibility conflict: ");
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
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (int row = 0; row < kSize; row++)
		{
			for (int col = 0; col < kSize; col++)
			{
				if (num_possible_vals[row][col] == 1)
					continue;

				int counts[kSize] = {0};
				for (int i = 0; i < kSize * 3; i++)
				{
					Pair neigh = neighbors[row][col][i];
					if (neigh.row == row && neigh.col == col)
						continue;

					for (int value = 0; value < kSize; value++)
					{
						if (possible_vals[neigh.row][neigh.col][value])
							counts[value]++;
					}
				}

				for (int value = 0; value < kSize; value++)
				{
					if (possible_vals[row][col][value] &&
						counts[value] == 0)
					{
						DEBUG_OUT("SET(%d, %d, %x)\n", row, col, value+1);
						if (!Set(row, col, value+1))
							return false;
						changed = true;
					}
				}
			}
		}
	}

	//pick an unsolved square
	Pair search_target = FindMostSolvedOptimal();
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
		if(!possible_vals[search_row][search_col][value])
		{
			continue;
		}

		DEBUG_OUT("searching on %d, %d; value %d\n", search_row, search_col, value+1);

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
/*
Pair Sudoku::FindMostSolvedOptimal()
{
	int min_count = 3 * kSize * kSize + 1;
	Pair result;
	result.row = -1;
	result.col = -1;

	//reused for each cell
	int* counts = new int[kSize];

	//
	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			if(num_possible_vals[row][col] > 1)
			{
				//find value with fewest dependencies
				for(int value = 0; value < kSize; value++)
				{
					counts[value] = 0;
				}

				//check col
				for(int r = 0; r < kSize; r++)
				{
					if(r == row)
					{
						continue;
					}

					for(int value = 0; value < kSize; value++)
					{
						if(possible_vals[r][col][value])
						{
							counts[value]++;
						}
					}
				}

				//check row
				for(int c = 0; c < kSize; c++)
				{
					if(c == col)
					{
						continue;
					}

					for(int value = 0; value < kSize; value++)
					{
						if(possible_vals[row][c][value])
						{
							counts[value]++;
						}
					}
				}

				//check box
				int block_row = (int)(kSqrtSize * floor(row / kSqrtSize));
				int block_col = (int)(kSqrtSize * floor(col / kSqrtSize));
				for(int r = block_row; r < block_row + kSqrtSize; r++)
				{
					for(int c = block_col; c < block_col + kSqrtSize; c++)
					{
						for(int value = 0; value < kSize; value++)
						{
							if(r != row && c != col &&
							   possible_vals[r][c][value])
							{
								counts[value]++;
							}
						}
					}
				}

				//find possible value with lowest count
				int min_count2 = 3 * kSize * kSize + 1;
				int min_value2 = -1;
				for(int value = 0; value < kSize; value++)
				{
					if(possible_vals[row][col][value] &&
					   counts[value] < min_count2)
					{
						min_count2 = counts[value];
						min_value2 = value;
					}
				}

				//check this dependency count against global min
				if(min_count2 < min_count)
				{
					min_count = min_count2;
					//min_value = min_value2;
					result.row = row;
					result.col = col;
				}
			}
		}
	}

	delete[] counts;
	return result;
}
*/
//TODO: refactor this shiz
Pair Sudoku::FindMostSolvedOptimal()
{
	int min = kSize * kSize + 1;
	Pair result;
	result.row = -1;
	result.col = -1;

	// find optimal value
	int* counts = new int[kSize];
	for(int value = 0; value < kSize; value++)
	{
		counts[value] = 0;
	}

	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			int num_poss_vals = num_possible_vals[row][col];
			if(num_poss_vals > 1)
			{
				for(int value = 0; value < kSize; value++)
				{
					if(possible_vals[row][col][value])
					{
						counts[value]++;
					}
				}
			}
		}
	}

	int max_count = 0;
	int max_value = 0;
	for(int value = 0; value < kSize; value++)
	{
		if(counts[value] > max_count)
		{
			max_count = counts[value];
			max_value = value;
		}
	}

	//DEBUG_OUT("max_value %d", max_value);

	// find most solved cell with optimal value
	for(int row = 0; row < kSize; row++)
	{
		for(int col = 0; col < kSize; col++)
		{
			int current = num_possible_vals[row][col];
			if(current < min && current > 1 &&
			   possible_vals[row][col][max_value])
			{
				result.row = row;
				result.col = col;
				min = num_possible_vals[row][col];
			}
		}
	}

	delete[] counts;
	return result;
}
