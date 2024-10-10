#include <iostream>
#include <vector>
#include <cstdlib>

using namespace std;

class Cell 
{
	public:
		bool isAlive;
	Cell()
	{
		isAlive = NULL;
	}
	Cell(bool isAlive) : isAlive(isAlive) {}
	~Cell()
	{
		// Delete any allocated memory used by the object
	}

	// Operator = to set the status of a cell.
	Cell& operator=(const bool status)
	{
		this->isAlive = status;
		return *this;
	}
	char getIcon() const
	{
		return (isAlive) ? 'O' : ' ';
	}

};

// Operator << to print the grid of cells.
ostream& operator << (ostream& os, const vector<vector<Cell>>& grid)
{
	for (const auto& row : grid)
	{
		for (const auto& cell : row)
		{
			os << "." << cell.getIcon();
		}
		os << "." << endl;
	}
	return os;
}

// Operator + to count the total of live neighbours.
int operator +(const Cell& a, const Cell& b)
{
	return (a.isAlive ? 1 : 0) + (b.isAlive ? 1 : 0);
}

vector<vector<Cell>> generateGrid()
{
	// Generates the size of the grid based on user inputs.
	int xSpaces;
	int ySpaces;

	cout << endl << "Enter number of spaces on the X Axis: ";
	cin >> xSpaces;

	cout << endl << "Enter number of spaces on the Y Axis: ";
	cin >> ySpaces;

	vector<vector<Cell>> grid(xSpaces, vector<Cell>(ySpaces));
	

	return grid;
}
void createCells(vector<vector<Cell>> &grid) 
{
	for (int x = 0; x < grid.size(); x++)
	{
		for (int y = 0; y < grid[x].size(); y++)
		{
			grid[x][y] = Cell(false);
		}
	}
}

void outputGrid(vector<vector<Cell>> &grid)
{
	// Outputs the grid with the overloaded << operator.
	cout << grid;
}


void scatterCells(vector<vector<Cell>> &grid)
{
	int numCells;
	int totalCells = 0;

	srand(time(0)); // Generate a new seed

	cout << endl << "Enter total of alive cells: ";
	cin >> numCells;
	
	// Ensure the number of live cells is not greater than the total grid spaces.
	int maxCells = grid.size() * grid[0].size();
	if (numCells > maxCells)
	{
		numCells = maxCells;
	}

	while (totalCells < numCells)
	{	
		int xPos = rand() % grid.size();
		int yPos = rand() % grid[0].size();

		// If the cell is not already alive
		if (!grid[xPos][yPos].isAlive)
		{
			grid[xPos][yPos] = 1;
			totalCells++;
		}
	}
}

// Count the total of live cells around cell at grid (x, y)
int countLiveNeighbours(const vector<vector<Cell>>& grid, int x, int y)
{
	int liveNeighbours = 0;
	int rows = grid.size();
	int cols = grid[0].size();

	for (int i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			if (i == 0 && j == 0) { continue; } // Ignore self.
			int newX = x + i;
			int newY = y + j;

			// Ensure the indices are within bounds
			if ( (newX >= 0 && newX < rows) && (newY >= 0 && newY < cols) )
			{
				liveNeighbours += grid[newX][newY].isAlive ? 1 : 0;
			}
		}
	}
	return liveNeighbours;
}

void UpdateGrid(vector<vector<Cell>> &grid)
{
	for (int x = 0; x < grid.size(); x++)
	{
		for (int y = 0; y < grid.size(); y++)
		{
			
		}
	}
}


void runSimulation() 
{
	// Runs the simulation for x cycles.
}
void saveSimulation()
{
	// Saves the simulation to the drive.
}
void loadSimulation() 
{
	// Loads the simulation from the drive.
}



int main()
{
	vector<vector<Cell>> grid = generateGrid();
	createCells(grid);
	scatterCells(grid);
	outputGrid(grid);

	int liveNeighbours = countLiveNeighbours(grid, 2, 2);
	cout << endl << "Live neighbours of cell (2, 2): " << liveNeighbours << endl;
	return 0;
}