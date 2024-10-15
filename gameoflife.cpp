#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <limits>
#include <random>

using namespace std;

template <typename T>
class CellBase
{
	public:
		virtual ~CellBase() = default; // Virtual Destructor for cleanup.

		virtual bool isAlive() const = 0;
		virtual void setAlive(T status) = 0;
		virtual char getIcon() const = 0;
};



template <typename T>
class NormalCell : public CellBase<T>
{
	private:
		T status;
	public:

		NormalCell(T status) : status(status) {}

		bool isAlive() const override { return status; }

		void setAlive(T status) override { this->status = status;}

		// Returns a 'O' if the cell is alive, ' ' if the cell is dead.
		char getIcon() const override
		{
			return status ? 'O' : ' ';
		}

};



// Creates a Template for to enhance code conciseness.
template <typename T>
using Grid = vector<vector<CellBase<T>*>>;

// Operator << to print the grid of cells.
template <typename T>
ostream& operator << (ostream& os, const Grid<T>& grid)
{
	os << "-------------------------------------------------------------------------------" << endl;
	for (const auto& row : grid)
	{
		for (const auto& cell : row)
		{
			if (cell)
			{
				os << "." << cell->getIcon();
			}
			else
			{
				os << "X"; // print empty for null pointers.
			}
		}
		os << "." << endl;
	}
	return os;
}

// Creates an empty 2D vector to use as a grid.
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

	Grid<T> grid(xSpaces, vector<CellBase<T>*>(ySpaces));
	

	return grid;
}

template <typename T>
void cleanupGrid(Grid<T>& grid)
{
	for (auto& row : grid)
	{
		for (auto& cell : row)
		{
			if (cell)
			{
				delete cell; // Deallocate memory used by each cell object.
				cell = nullptr; // Set pointer to nullptr to avoid dangling pointers.
			}
		}
	}
}


// Fills the grid with dead cells.
template <typename T>
void createCells(Grid<T> &grid) 
{
	for (int x = 0; x < grid.size(); x++)
	{
		for (int y = 0; y < grid[x].size(); y++)
		{
			grid[x][y] = new NormalCell<T>(false);
		}
	}
}

// Randomly distribute cells across the grid.
template <typename T>
void scatterCells(Grid<T> &grid, int numCells, unsigned int& seed)
{
	mt19937 gen(seed);
	uniform_int_distribution<> xDist(0, grid.size() - 1);
	uniform_int_distribution<> yDist(0, grid[0].size() - 1);

	int totalCells = 0;

	if (!numCells) {
		cout << endl << "Enter total of alive cells: ";
		cin >> numCells;
	}
	
	// Ensure the number of live cells is not greater than the total grid spaces.
	int maxCells = grid.size() * grid[0].size();
	if (numCells > maxCells)
	{
		numCells = maxCells;
	}

	while (totalCells < numCells)
	{	
		// Generate an X and Y position within grid boundaries.
		int xPos = xDist(gen);
		int yPos = yDist(gen);

		// If the cell is not already alive
		if (!grid[xPos][yPos]->isAlive())
		{
			grid[xPos][yPos]->setAlive(true);
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
				// Add 1 or 0 based on cell's status.
				liveNeighbours += grid[newX][newY]->isAlive() ? 1 : 0;
			}
		}
	}
	return liveNeighbours;
}

template <typename T>
bool matchesPattern(Grid<T>& grid, const vector<vector<bool>>& pattern, int startX, int startY)
{
	int patternRows = pattern.size();
	int patternCols = pattern[0].size();
	int gridRows = grid.size();
	int gridCols = grid[0].size();

	// Check if the pattern fits within the grid boundaries

	if (startX + patternRows > gridRows || startY + patternCols > gridCols)
	{
		return false;
	}

	for (int i = 0; i < patternRows; ++i)
	{
		for (int j = 0; j < patternCols; ++j)
		{
			if (grid[startX + i][startY + j]->isAlive() != pattern[i][j])
			{
				return false;
			}
		}
	}

	// Ensure that no extra live cells exist
	for (int i = -1; i < patternRows + 1; ++i)
	{
		for (int j = -1; j < patternCols + 1; ++j)
		{
			// Skip cells inside the pattern
			if (i >= 0 && i < patternRows && j >= 0 && j < patternCols)
			{
				continue;
			}

			int checkX = startX + i;
			int checkY = startY + j;

			// Ensure  the cell is within grid boundaries
			if (checkX >= 0 && checkX < gridRows && checkY >= 0 && checkY < gridCols)
			{
				// If any surrounding cell is alive, return false
				if (grid[checkX][checkY]->isAlive())
				{
					return false;
				}
			}
		}
	}

	return true;
}

template <typename T>
bool isBlockOrBeehive(Grid<T>& grid)
{
	int rows = grid.size();
	int cols = grid[0].size();

	// Define Patterns

	// Block

	vector<vector<bool>> blockPattern = {
		{true, true},
		{true, true},
	};

	// Behive

	vector<vector<bool>> beehivePattern = {
		{false, true, true, false},
		{true, false, false, true},
		{false, true, true, false}
	};

	vector<vector<bool>> verticalBeehivePattern =
	{
		{false, true, false},
		{true, false, true},
		{true, false, true},
		{false, true, false}
	};

	// Check every position in the grid for both patterns
	for (int x = 0; x < rows; ++x)
	{
		for (int y = 0; y < cols; ++y)
		{
			// Check for pattern at this position
			if (matchesPattern(grid, blockPattern, x, y) || matchesPattern(grid, beehivePattern, x, y) || matchesPattern(grid, verticalBeehivePattern, x, y))
			{
				return true;
			}
		}
	}
	return false;
}

template <typename T>
bool isBlinkerOrToad(Grid<T>& grid)
{
	// Define patterns
	
	// Blinker Phase 1
	
	vector<vector<bool>> blinkerPhase1 =
	{
		{true, true, true}
	};

	// Blinker Phase 2
	vector<vector<bool>> blinkerPhase2 =
	{
		{true},
		{true},
		{true}
	};

	// Toad Phase 1
	vector<vector<bool>> toadPhase1 =
	{
		{false, true, true, true},
		{true, true, true, false}
	};

	vector<vector<bool>> toadPhase1Flipped =
	{
		{true, true, true, false},
		{false, true, true, true}
	};

	vector<vector<bool>> toadPhase2 =
	{
		{false, false, true, false},
		{true, false, false, true},
		{true, false, false, true},
		{false, true, false, false}
	};

	vector<vector<bool>> toadPhase2Flipped =
	{
		{false, true, false, false},
		{true, false, false, true},
		{true, false, false, true},
		{false, false, true, false}
	};
	
	int rows = grid.size();
	int cols = grid[0].size();

	// Check every position in the grid for both patterns
	for (int x = 0; x < rows; ++x)
	{
		for (int y = 0; y < cols; ++y)
		{
			// Check for pattern at this position
			if (matchesPattern(grid, blinkerPhase1, x, y) || matchesPattern(grid, blinkerPhase2, x, y) || matchesPattern(grid, toadPhase1, x, y) || matchesPattern(grid, toadPhase1Flipped, x, y) || matchesPattern(grid, toadPhase2, x, y) || matchesPattern(grid, toadPhase2Flipped, x, y))
			{
				return true;
			}
		}
	}
	return false;

}

template <typename T>
void updateCellsSegment(Grid<T>& grid, Grid<T>& newGrid, int startRow, int endRow)
{
	for (int x = startRow; x < endRow; x++)
	{
		for (int y = 0; y < grid[x].size(); y++)
		{
			int totalNeighbours = countLiveNeighbours(grid, x, y);

			if (totalNeighbours < 2 || totalNeighbours > 3) 
			{
				newGrid[x][y] = new NormalCell<T>(false);
			}
			else if (totalNeighbours == 3) 
			{
				newGrid[x][y] = new NormalCell<T>(true);
			}
			else 
			{
				newGrid[x][y] = new NormalCell<T>(grid[x][y]->isAlive());
			}
		}
	}
}

// Updates Cells in parallel.
template <typename T>
void UpdateCells(Grid<T> &grid)
{
	Grid<T> newGrid (grid.size(), vector<CellBase<T>*>(grid[0].size()));
	vector<thread> threads;

	int numThreads = thread::hardware_concurrency();
	int rowsPerThread = grid.size() / numThreads;

	for (int i = 0; i < numThreads; ++i)
	{
		int startRow = i * rowsPerThread;
		int endRow = (i == numThreads - 1) ? grid.size() : startRow + rowsPerThread;
		threads.push_back(thread(updateCellsSegment<T>, ref(grid), ref(newGrid), startRow, endRow));
	}

	for (auto& th : threads)
	{
		th.join(); // Wait for all threads to finish
	}

	cleanupGrid(grid);
	grid = newGrid;
}

// Updates the grid for X cycles.
template <typename T>
void runSimulation(Grid<T> &grid, int* totalCyclesPtr) 
{
	// Runs the simulation for x cycles.
	int totalCycles;
	int currentCycle = 0;

	if (totalCyclesPtr == nullptr) {
		cout << endl << "Enter the number of phases to run: ";
		cin >> totalCycles;
	}
	else
	{
		totalCycles = *totalCyclesPtr;
	}
	
	while (currentCycle < totalCycles)
	{
		cout << grid;
		UpdateCells(grid);
		currentCycle++;
	}
}

template <typename T>
bool checkForStableStillLife(Grid<T>& grid, int &stableGenerations, int currentCycle){
	if (currentCycle > 0 && isBlockOrBeehive(grid))
	{
		stableGenerations++;
	}
	else
	{
		stableGenerations = 0;
	}
	if (stableGenerations >= 2)
	{
		return true;
	}
	return false;

}

template <typename T>
void runExperiment(Grid<T>& grid, int patternChoice)
{
	int experimentCount = 0;
	int stableGenerations = 0; // Track how many 'frames' the pattern appears for.
	bool patternFound = false;
	int cycles = 1;
	int totalCycles;
	int totalCells;

	cout << endl << "Enter the number of phases to run per experiement: ";
	cin >> totalCycles;

	cout << endl << "Enter the number of living cells per experiment: ";
	cin >> totalCells;

	while (!patternFound)
	{
		random_device rd; // Generate new seed.
		unsigned int seed = rd();
		experimentCount++;
		createCells(grid);
		scatterCells(grid, totalCells, seed);

		cout << endl << "Running experiment #" << experimentCount;

		int currentCycle = 0;
		while (currentCycle < totalCycles && !patternFound)
		{
			runSimulation(grid, &cycles);
			switch (patternChoice)
			{
				case 1:
					// Check for block or beehive after each generation of cells.
					if (checkForStableStillLife(grid, stableGenerations, currentCycle))
					{
						patternFound = true;
						cout << endl << "Block or Beehive detected in experiment #" << experimentCount << " after " << currentCycle << " generations!";
						cout << seed;
					}
					break;


			}
			currentCycle++;
		}
		cleanupGrid(grid);
	}
}

// Saves the Grid onto the system storage. Allows users to enter their own filenames.
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

// Loads a file from the system storage and updates the grid based on the position of the cells inside the text file.
template <typename T>
bool loadSimulation(Grid<T> &grid) 
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
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		return false;
	}


	// Clears the grid allowing for the loaded layout to be copied onto it.
	cleanupGrid(grid);


	while (getline(gridLoadFile, loadedRow))
	{
		if (loadedRow.find("---") != string::npos)
		{
			continue;
		}
		vector<CellBase<T>*> newRow;

		for (char cellChar : loadedRow)
		{
			if (cellChar == 'O') // If alive cell.
			{
				newRow.push_back(new NormalCell<T>(true));
			}
			else if (cellChar == ' ') 
			{
				newRow.push_back(new NormalCell<T>(false));
			}
		}
		grid.push_back(newRow);
	}
	gridLoadFile.close();
	return true;
}

template <typename T>
void displaySaveMenu(Grid<T> &grid)
{
	bool saving = true;
	int choice;
	cout << endl << "Simulation Finished!";

	while (saving)
	{
		cout << endl << "Would you like to save the final grid?";
		cout << endl << "|| 1. Yes";
		cout << endl << "|| 2. No";
		cout << endl << "Select an option: ";
		cin >> choice;
		switch (choice)
		{
			case 1:
				saveSimulation(grid);
				saving = false;
				break;
			case 2:
				saving = false;
				break;
			default:
				cout << "Error: Invalid Option. Please try again.";
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				break;
		}
	}


}

int displayPatternMenu()
{
	int patternChoice;

	cout << endl << "|| 1. Block and Beehive";
	cout << endl << "|| 2. Blinker or toad";
	cout << endl << "|| 3. Glider or LWSS";
	cout << endl << "|| Choose a pattern to search for: ";
	cin >> patternChoice;

	return patternChoice;
}

int displayOptionMenu()
{
	int choice;

	cout << endl << "|| 1. Create New Simulation";
	cout << endl << "|| 2. Load Simulation from Storage";
	cout << endl << "|| 3. Run experiment to find pattern";
	cout << endl << "|| 4. Exit Game";
	cout << endl << "|| Select an option: ";

	cin >> choice;
	return choice;
}
template <typename T>
void displayWelcomeMenu(Grid<T> &grid)
{
	int choice;
	bool running = true;
	unsigned int seed = 0;
	cout << "|| Welcome to Ryan's version of John Conway's: Game of Life! ||";

	while (running)
	{
		choice = displayOptionMenu();
		

		switch (choice)
		{
			case 1:
				grid = generateGrid<bool>();
				random_device rd; // Generate new seed
				seed = rd();
				createCells(grid);
				scatterCells(grid, NULL, seed);
				runSimulation(grid, nullptr);
				displaySaveMenu(grid);
				break;
			case 2:
				if (loadSimulation(grid))
				{
					runSimulation(grid, nullptr);
					displaySaveMenu(grid);
				}
				break;
			case 3:
				int patternChoice;
				grid = generateGrid<bool>();
				patternChoice = displayPatternMenu();
				runExperiment(grid, patternChoice);
				break;
			case 4:
				running = false; // Quit the loop;
				break;
			default:
				cout << endl << "Error: Invalid Option. Please try again.";
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				break;
		}
	}
}

int main()
{

	Grid<bool> grid;

	displayWelcomeMenu(grid);

	cleanupGrid(grid); // Always clean up before exiting.
	return 0;
}