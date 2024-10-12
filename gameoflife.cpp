#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

using namespace std;

template <typename T>
class Cell 
{
	public:
		T isAlive;
	Cell()
	{
		isAlive = T();
	}
	Cell(T isAlive) : isAlive(isAlive) {}
	~Cell()
	{
		// Delete any allocated memory used by the object
	}

	// Operator = to set the status of a cell.
	Cell& operator=(const T& status)
	{
		this->isAlive = status;
		return *this;
	}
	char getIcon() const
	{
		return isAlive ? 'O' : ' ';
	}

};

template <typename T>
using Grid = vector<vector<Cell<T>>>;

// Operator << to print the grid of cells.
template <typename T>
ostream& operator << (ostream& os, const Grid<T>& grid)
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

//// Operator + to count the total of live neighbours.
//template <typename T>
//int operator +(const Cell& a, const Cell& b)
//{
//	return (a.isAlive ? 1 : 0) + (b.isAlive ? 1 : 0);
//}

template <typename T>
Grid<T> generateGrid()
{
	// Generates the size of the grid based on user inputs.
	int xSpaces;
	int ySpaces;

	cout << endl << "Enter number of spaces on the X Axis: ";
	cin >> xSpaces;

	cout << endl << "Enter number of spaces on the Y Axis: ";
	cin >> ySpaces;

	Grid<T> grid(xSpaces, vector<Cell<T>>(ySpaces));
	

	return grid;
}

template <typename T>
void createCells(Grid<T> &grid) 
{
	for (int x = 0; x < grid.size(); x++)
	{
		for (int y = 0; y < grid[x].size(); y++)
		{
			grid[x][y] = Cell(false);
		}
	}
}


template <typename T>
void scatterCells(Grid<T> &grid)
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
template <typename T>
int countLiveNeighbours(Grid<T>& grid, int x, int y)
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

template <typename T>
void UpdateCells(Grid<T> &grid)
{
	Grid<T> newGrid = grid;

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

template <typename T>
void runSimulation(Grid<T> &grid) 
{
	// Runs the simulation for x cycles.
	int currentCycle = 0;
	int totalCycles;
	cout << endl << "Enter the number of phases to run: ";
	cin >> totalCycles;
	
	cout << grid;
	while (currentCycle < totalCycles)
	{
		UpdateCells(grid);
		cout << grid;
		currentCycle++;
	}
}
template <typename T>
void saveSimulation(Grid<T> &grid)
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

template <typename T>
void loadSimulation(Grid<T> &grid) 
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
		vector<Cell<T>> newRow;

		for (char cellChar : loadedRow)
		{
			if (cellChar == 'O') // If alive cell.
			{
				newRow.push_back(Cell<T>(true));
			}
			else if (cellChar == ' ')
			{
				newRow.push_back(Cell<T>(false));
			}
		}
		grid.push_back(newRow);
	}
	gridLoadFile.close();
}



int main()
{
	srand(time(0)); // Generate a new seed
	Grid<bool> grid = generateGrid<bool>();
	/*createCells(grid);
	scatterCells(grid);
	runSimulation(grid);
	saveSimulation(grid);*/

	loadSimulation(grid);
	cout << grid;
	runSimulation(grid);
	return 0;
}