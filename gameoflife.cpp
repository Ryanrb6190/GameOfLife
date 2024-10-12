#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

using namespace std;

class Cell 
{
	public:
		bool isAlive;
	Cell()
	{
		isAlive = false;
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
			grid[xPos][yPos] = true;
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

void UpdateCells(vector<vector<Cell>> &grid)
{
	vector<vector<Cell>> newGrid = grid;

	for (int x = 0; x < grid.size(); x++)
	{
		for (int y = 0; y < grid[x].size(); y++)
		{
			int totalNeighbours = countLiveNeighbours(grid, x, y);

			if (totalNeighbours < 2 || totalNeighbours > 3)
			{
				// Unpopulated case: Death
				newGrid[x][y] = false;
			}
			else if (totalNeighbours == 3)
			{
				// Handle Reproduction
				newGrid[x][y] = true;
			}
			// No change needed if total neighbours == 2.
		}
	}
	grid = newGrid;
}


void runSimulation(vector<vector<Cell>> &grid) 
{
	// Runs the simulation for x cycles.
	int currentCycle = 0;
	int totalCycles;
	cout << endl << "Enter the number of phases to run: ";
	cin >> totalCycles;
	
	outputGrid(grid);
	while (currentCycle < totalCycles)
	{
		UpdateCells(grid);
		outputGrid(grid);
		currentCycle++;
	}
}
void saveSimulation(vector<vector<Cell>> &grid)
{
	// Saves the simulation to the drive.
	string filename;
	cout << endl << "Enter file name: ";
	cin >> filename;
	ofstream gridSaveFile(filename + ".txt");
	if (gridSaveFile.is_open())
	{
		gridSaveFile << grid;
	}
	gridSaveFile.close();
}
void loadSimulation(vector<vector<Cell>> &grid) 
{
	// Loads the simulation from the drive.
	string loadedRow;
	string filename;

	cout << endl << "Enter file name to load: ";
	cin >> filename;

	ifstream gridLoadFile(filename + ".txt");

	if (!gridLoadFile.is_open()) 
	{
		cout << endl << "Error: Unable to open the file.";
		return;
	}

	grid.clear(); 

	while (getline(gridLoadFile, loadedRow))
	{
		vector<Cell> newRow;

		for (char cellChar : loadedRow)
		{
			if (cellChar == 'O') // If alive cell.
			{
				newRow.push_back(Cell(true));
			}
			else if (cellChar == ' ')
			{
				newRow.push_back(Cell(false));
			}
		}
		grid.push_back(newRow);
	}
	gridLoadFile.close();
}



int main()
{
	srand(time(0)); // Generate a new seed
	vector<vector<Cell>> grid = generateGrid();
	/*createCells(grid);
	scatterCells(grid);
	runSimulation(grid);
	saveSimulation(grid);*/

	loadSimulation(grid);
	cout << grid;
	runSimulation(grid);
	return 0;
}