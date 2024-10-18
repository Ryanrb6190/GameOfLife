#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <limits>
#include <random>
#include <unordered_set>
#include <cassert>
#include <sstream>

using namespace std;
mutex mtx;
struct ClearAndIgnore {}; // Custom struct to help clear any error inputs.



// Base cell class
template <typename T>
class CellBase
{

	public:
		virtual ~CellBase() = default; // Virtual Destructor for cleanup.

		virtual bool isAlive() const = 0;
		virtual void setAlive(T status) = 0;
		virtual char getIcon() const = 0;
};

// Class for a normal cell
template <typename T>
class NormalCell : public CellBase<T>
{

	private:
		T status;
	public:

		NormalCell(T status) : status(status) {}

		bool isAlive() const override { return status; }

		void setAlive(T status) override { this->status = status; }

		// Returns a 'O' if the cell is alive, ' ' if the cell is dead.
		char getIcon() const override
		{
			return status ? 'O' : ' ';
		}

};

// Class to store csv data in after being read for a .csv file
class CSVData
{

	private:
		int xSpaces;
		int ySpaces;
		unsigned int seed;
		int totalCells;
		int totalCycles;
	public:
		CSVData(int xSpaces, int ySpaces, unsigned int seed,  int totalCycles, int totalCells)
			: xSpaces(xSpaces), ySpaces(ySpaces), seed(seed), totalCycles(totalCycles), totalCells(totalCells) {}

		// Get functions
		int getXSpaces() const { return xSpaces; }
		int getYSpaces() const { return ySpaces; }
		unsigned int getSeed() const { return seed; }
		int getTotalCycles() const { return totalCycles; }
		int getTotalCells() const { return totalCells; }

		
};

// Creates a Template for to enhance code conciseness.
template <typename T>
using Grid = vector<vector<CellBase<T>*>>;

// Operator overide of << to print the grid of cells.
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

// Operator override of >> to clean the input stream.
istream& operator >> (istream& in, ClearAndIgnore)
{
	in.clear();
	in.ignore(numeric_limits<streamsize>::max(), '\n');
	return in;
}

// Function to check whether the input is valid or not.
bool isValidInput(int input)
{
	if (cin.fail() || input <= 0)
	{
		cout << endl << "Error: Invalid Input. Please enter a positive number.";
		cin >> ClearAndIgnore();
		return false;
	}
	return true;
}

// Creates an empty 2D vector to use as a grid. Can be generated with user inputs or by int pointers.
template <typename T>
Grid<T> generateGrid(int* xSizePointer, int* ySizePointer)
{
	int xSpaces;
	int ySpaces;

	if (xSizePointer == nullptr || ySizePointer == nullptr)
	{
		bool allowedInput = false;
		while (!allowedInput) {

			cout << endl << "Enter number of spaces on the X Axis: ";
			cin >> xSpaces;

			if (isValidInput(xSpaces)) {

				cout << endl << "Enter number of spaces on the Y Axis: ";
				cin >> ySpaces;
				
				if (isValidInput(ySpaces))
				{
					allowedInput = true;
				}
			}
		}
	}
	else
	{
		xSpaces = *xSizePointer;
		ySpaces = *ySizePointer;
	}

	Grid<T> grid(xSpaces, vector<CellBase<T>*>(ySpaces));
	

	return grid;
}

// Function to clean up the grid and deallocate any memory
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
	for (size_t x = 0; x < grid.size(); x++)
	{
		for (size_t y = 0; y < grid[x].size(); y++)
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
			if (i == 0 && j == 0) 
			{ 
				continue; 
			} // Ignore self.
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

// Function to check whether an orientation of the pattern fits the grid
template <typename T>
bool patternFits(Grid<T>& grid, const vector<vector<bool>>& pattern, int startX, int startY)
{
	int patternRows = pattern.size();
	int patternCols = pattern[0].size();
	int gridRows = grid.size();
	int gridCols = grid[0].size();

	// Check if pattern fits within grid boundaries
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
	return true;
}

// Function that rotates the pattern by 90 degrees.
template <typename T>
vector<vector<T>> rotatePattern(const vector<vector<T>>& pattern)
{
	int rows = pattern.size();
	int cols = pattern[0].size();
	vector<vector<T>> rotated(cols, vector<T>(rows));

	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			rotated[j][rows - 1 - i] = pattern[i][j];
		}
	}
	return rotated;
}

// Function that flips a pattern horizontally
template <typename T>
vector<vector<T>> flipHorizontal(const vector<vector<T>>& pattern)
{
	int rows = pattern.size();
	vector<vector<T>> hFlipped = pattern;

	for (int i = 0; i < rows; ++i)
	{
		reverse(hFlipped[i].begin(), hFlipped[i].end()); // reverses the elements in a row.
	}
	return hFlipped;
}

// Function that flips a pattern vertically
template <typename T>
vector<vector<T>> flipVertical(const vector<vector<T>>& pattern)
{
	vector<vector<T>> vFlipped = pattern;
	reverse(vFlipped.begin(), vFlipped.end()); // reverses the elements in a 2d array column

	return vFlipped;
}

// Hashing function for 2D vector to prevent duplicate patterns
template <typename T>
size_t hashPattern(const vector<vector<T>>& pattern)
{
	size_t seed = 0;
		for (const auto& row : pattern) 
		{
			for (const auto& elem : row) 
			{
				seed ^= hash<T>{}(elem)+0x9e3779b9 + (seed << 6) + (seed >> 2); // Boost hash combine algorithm 
			}
		}
	return seed;
}


// Generate all pattern variants for multiple uses
template <typename T>
vector<vector<vector<T>>> generateAllPatternVariants(const vector<vector<T>>& pattern)
{
	vector<vector<vector<T>>> variants;
	unordered_set<size_t> uniquePatterns; // Ensures only unique patterns aere added to the vector.

	auto addUniqueVariant = [&](const vector<vector<T>>& p) // function to only add unique pattern varients to the set.
		{
			size_t hash = hashPattern(p);
			if (uniquePatterns.insert(hash).second)
			{
				variants.push_back(p);
			}
		};

	// add original pattern
	addUniqueVariant(pattern);


	// Generate all 90, 180, and 270 degree rotations
	auto rotated = pattern;
	for (int i = 0; i < 3; ++i)
	{
		rotated = rotatePattern(rotated);
		addUniqueVariant(rotated);
	}

	vector<vector<vector<T>>> patternsToFlip = variants;

	// Add horizontal and vertical flips
	for (const auto& p : patternsToFlip)
	{
		addUniqueVariant(flipHorizontal(p));
		addUniqueVariant(flipVertical(p));
	}

	return variants;
}

// Function to check whether a pattern is in a grid by comparing all posible variants (rotations and flips) of a pattern.
template <typename T>
bool matchesPattern(Grid<T>& grid, const vector<vector<bool>>& pattern, int startX, int startY)
{
	auto patternVariants = generateAllPatternVariants(pattern);

	bool found = false;
	vector<thread> threads;

	auto checkPattern = [&](const vector<vector<bool>>& p)
	{
		if (!found && patternFits(grid, p, startX, startY))
		{	
			lock_guard<mutex> lock(mtx); // lock when modifying shared data
			if (!found)
			{
				found = true;
			}

		}
	};

	// Limit number of threads
	int numThreads = min(thread::hardware_concurrency(), static_cast<unsigned int>(patternVariants.size()));

	// Start threads for each rotation and flip combination
	for (int i = 0; i < numThreads; ++i)
	{
		threads.push_back(thread([&, i]() {
			for (int j = i; j < patternVariants.size(); j += numThreads)
			{
				if (found)
				{
					return;
				}
				checkPattern(patternVariants[j]);
			}
		}));
	}

	for (auto& th : threads)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	return found;
}

// Function to define a block and beehive and then checks to see if the found pattern is either or.
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

	// Check every position in the grid for both patterns
	for (int x = 0; x < rows; ++x)
	{
		for (int y = 0; y < cols; ++y)
		{
			// Count neighbours if cell is dead and has no neighbours skip this cell

			int neighbours = countLiveNeighbours(grid, x, y);
			if (grid[x][y] != nullptr && !grid[x][y]->isAlive() && neighbours == 0)
			{
				continue;
			}

			// Check for pattern at this position
			if (matchesPattern(grid, blockPattern, x, y) || matchesPattern(grid, beehivePattern, x, y))
			{
				return true;
			}
		}
	}
	return false;
}

// Function to define a blinker and toad and then checks to see if the found pattern is either or.
template <typename T>
bool isBlinkerOrToad(Grid<T>& grid)
{
	// Define patterns
	
	// Blinker Phase 1
	
	vector<vector<bool>> blinkerPhase1 =
	{
		{true, true, true}
	};

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

	vector<vector<bool>> toadPhase2 =
	{
		{false, false, true, false},
		{true, false, false, true},
		{true, false, false, true},
		{false, true, false, false}
	};
	
	int rows = grid.size();
	int cols = grid[0].size();

	// Check every position in the grid for both patterns
	for (int x = 0; x < rows; ++x)
	{
		for (int y = 0; y < cols; ++y)
		{
			// Count neighbours if cell is dead and has no neighbours skip this cell

			int neighbours = countLiveNeighbours(grid, x, y);
			if (grid[x][y] != nullptr && !grid[x][y]->isAlive() && neighbours == 0)
			{
				continue;
			}
			// Check for pattern at this position
			if (matchesPattern(grid, blinkerPhase1, x, y) || matchesPattern(grid, blinkerPhase2, x, y) || matchesPattern(grid, toadPhase1, x, y) || matchesPattern(grid, toadPhase2, x, y))
			{
				return true;
			}
		}
	}
	return false;

}

// Function to define a glider and LWSS and then checks to see if the found pattern is either or.
template <typename T>
bool isGliderOrLWSS(Grid<T>& grid)
{
	// Define patterns

	// Glider Phase 1
	
	vector<vector<bool>> gliderPhase1 =
	{
		{false, true, false},
		{false, false, true},
		{true, true, true}
	};

	// Glider Phase 2

	vector<vector<bool>> gliderPhase2 =
	{
		{true, false, true},
		{false, true, true},
		{false, true, false}
	};

	// LWSS Phase 1

	vector<vector<bool>> lwssPhase1 =
	{
		{false, true, false, false, true},
		{true, false, false, false, false},
		{true, false, false, false, true},
		{true, true, true, true, false}
	};

	// LWSS Phase 2

	vector<vector<bool>> lwssPhase2 =
	{
		{false, true, true, false, false},
		{true, true, false, true, true},
		{false, true, true, true, true},
		{false, false, true, true, false}
	};
	// Don't need other phases as they are just rotations of phase 1 and 2

	int rows = grid.size();
	int cols = grid[0].size();

	// Check every position in the grid for both patterns
	for (int x = 0; x < rows; ++x)
	{
		for (int y = 0; y < cols; ++y)
		{
			// Count neighbours if cell is dead and has no neighbours skip this cell

			int neighbours = countLiveNeighbours(grid, x, y);
			if (grid[x][y] != nullptr && !grid[x][y]->isAlive() && neighbours == 0)
			{
				continue;
			}

			// Check for pattern at this position
			if (matchesPattern(grid, gliderPhase1, x, y) || matchesPattern(grid, gliderPhase2, x, y) || matchesPattern(grid, lwssPhase1, x, y) || matchesPattern(grid, lwssPhase2, x, y))
			{
				return true;
			}
		}
	}
	return false;
}

// Function to update cells via threading
template <typename T>
void updateCellsSegment(Grid<T>& grid, Grid<T>& newGrid, int startRow, int endRow)
{
	for (size_t x = startRow; x < endRow; x++)
	{
		for (size_t y = 0; y < grid[x].size(); y++)
		{ 	
			// calculate neighbours
			int totalNeighbours = countLiveNeighbours(grid, x, y);

			// skip if dead and has no neighbours
			if (!grid[x][y]->isAlive() && totalNeighbours == 0)
			{
				newGrid[x][y] = new NormalCell<T>(false);
				continue;
			}

			// if lonely or overpopulated: die
			if (totalNeighbours < 2 || totalNeighbours > 3) 
			{
				newGrid[x][y] = new NormalCell<T>(false);
			}

			// if exactly 3 neighbours the dead cell is born
			else if (totalNeighbours == 3) 
			{
				newGrid[x][y] = new NormalCell<T>(true);
			}
			// if 2 neighbours stay the same
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

	// Gain the number of threads and how many rows a thread can process.
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


	// clean grid
	cleanupGrid(grid);

	// copy newgrid to grid
	grid = newGrid;
}

// Updates the grid for X cycles.
template <typename T>
void runSimulation(Grid<T> &grid, int totalCycles) 
{
	// Runs the simulation for x cycles
	int currentCycle = 0;
	
	while (currentCycle < totalCycles)
	{
		cout << grid;
		UpdateCells(grid);
		currentCycle++;

		// checks to see if all cells are dead. if so stops function prematurely
		if (checkForDeadCells(grid))
		{
			cout << endl << "All cells have died. Stopping simulation.";
			break;
		}
	}
}

// returns based if still life has remained for required generations
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

// returns based if oscillator has remained for required generations
template <typename T>
bool checkForStableOscillator(Grid<T>& grid, int& stableGenerations, int currentCycle)
{
	if (currentCycle > 0 && isBlinkerOrToad(grid))
	{
		stableGenerations++;
	}
	else
	{
		stableGenerations = 0;
	}
	if (stableGenerations >= 3)
	{
		return true;
	}
	return false;
}

// returns based if spaceship has remained for required generations
template <typename T>
bool checkForStableSpaceship(Grid<T>& grid, int& stableGenerations, int currentCycle)
{
	if (currentCycle > 0 && isGliderOrLWSS(grid))
	{
		stableGenerations++;
	}
	else
	{
		stableGenerations = 0;
	}
	if (stableGenerations >= 5)
	{
		return true;
	}
	return false;
}

// Function to check if all cells are dead
template <typename T>
bool checkForDeadCells(Grid<T>& grid)
{
	int rows = grid.size();
	int cols = grid[0].size();

	for (int x = 0; x < rows; ++x)
	{
		for (int y = 0; y < cols; ++y)
		{
			if (grid[x][y]->isAlive())
			{
				return false;
			}
		}
	}
	return true;
}

// Displays pattern menu
int menu_displayPatternMenu()
{
	int patternChoice;


	// Poorly implemented due to implementation with runExperiement will change if have time
	while (true) {
		cout << endl << "|| 1. Block and Beehive";
		cout << endl << "|| 2. Blinker or toad";
		cout << endl << "|| 3. Glider or LWSS";
		cout << endl << "|| Choose a pattern to search for: ";

		if (cin >> patternChoice)
		{
			if (patternChoice >= 1 && patternChoice <= 3)
			{
				break;
			}
			else
			{
				cout << "Error: Invalid choice. Please try again.";
			}
		}
		else {
			cout << endl << "Error: Invalid Option. Please try again.";
			cin >> ClearAndIgnore();
		}
	}
;

	return patternChoice;
}

// runs infinite simulation until chosen patterns are found
template <typename T>
void runExperiment(Grid<T>& grid)
{
	int MAX_EXPERIMENT = 300;
	int patternChoice = menu_displayPatternMenu();
	int experimentCount = 0;
	int stableGenerations = 0; // Track how many 'frames' the pattern appears for.
	bool patternFound = false;
	int cycles = 1;
	int totalCycles;
	int totalCells;
	bool allowedInput = false;
	
	// Check input
	while (!allowedInput)
	{
		cout << endl << "Enter the number of phases to run per experiement: ";
		cin >> totalCycles;

		if (isValidInput(totalCycles))
		{
			cout << endl << "Enter the number of living cells per experiment: ";
			cin >> totalCells;
			
			if (isValidInput(totalCells))
			{
				allowedInput = true;
			}
		}
	}

	while (!patternFound && experimentCount < MAX_EXPERIMENT)
	{
		random_device rd; // Generate new seed.
		unsigned int seed = rd();
		experimentCount++;
		createCells(grid);
		scatterCells(grid, totalCells, seed);

		cout << endl << "Running experiment #" << experimentCount << endl;

		int currentCycle = 0;
		// need to add max cycle limit
		while (currentCycle < totalCycles && !patternFound)
		{
			runSimulation(grid, cycles);
			switch (patternChoice)
			{
				case 1:
					// Check for block or beehive after each generation of cells.
					if (checkForStableStillLife(grid, stableGenerations, currentCycle))
					{
						patternFound = true;
						cout << endl << "Block or Beehive detected in experiment #" << experimentCount << " after " << currentCycle << " generations!";
						calculateERN(grid, totalCells, &patternChoice);
					}
					break;
				case 2:
					// Check for blinker or toad after each geneation of cells
					if (checkForStableOscillator(grid, stableGenerations, currentCycle))
					{
						patternFound = true;
						cout << endl << "Blinker or Toad detected in experiment #" << experimentCount << " after " << currentCycle << " generations!";
						calculateERN(grid, totalCells, &patternChoice);
					}
					break;
				case 3:
					// Check for glider or Lwss after each generation of cells
					if (checkForStableSpaceship(grid, stableGenerations, currentCycle))
					{
						patternFound = true;
						cout << endl << "Glider or LWSS detected in experiment #" << experimentCount << " after " << currentCycle << " generations!";
						calculateERN(grid, totalCells, &patternChoice);
					}
					break;


			}

			if (patternFound)
			{
				menu_displaySaveMenu(grid, seed, totalCycles, totalCells);
				break;
			}
			
			// stop experiment prematurely if grid contains only dead cells. Prevents waiting if cycles is set to a large number.
			if (checkForDeadCells(grid))
			{
				cout << grid;
				cout << endl << "All Cells for experiment #" << experimentCount << " have died.";
				break;
			}
			currentCycle++;
		}
		cleanupGrid(grid);
		if (experimentCount == MAX_EXPERIMENT)
		{
			cout << endl << "Error: Hard Limit Reached. Start another experiment";
		}
	}
}

// Calculates the ERN for the simulation or pattern
template <typename T>
void calculateERN(Grid<T> grid, int totalCells, int* patternChoice)
{
	int xSpaces = grid.size();
	int ySpaces = grid[0].size();
	// Create a dictionary of patterns with the phase that has the minium amount of available cells to appear.
	int ern = xSpaces + ySpaces + totalCells;

	if (patternChoice != nullptr)
	{
		map<string, int> minGridForPattern = {
			{"Block", 4},
			{"Beehive", 12},
			{"Blinker", 9},
			{"Toad", 16},
			{"Glider", 9},
			{"LWSS", 20}
		};

		switch (*patternChoice)
		{
		case 1:
		{
			int blockMinCells = minGridForPattern["Block"];
			int beehiveMinCells = minGridForPattern["Beehive"];
			int gridSize = xSpaces * ySpaces;

			if (gridSize < blockMinCells && gridSize < beehiveMinCells) {
				cout << endl << "A block or a beehive cannot appear in a " << xSpaces << "x" << ySpaces << "grid. ERN Unavailable.";
				break;
			}
			else if (gridSize >= blockMinCells && gridSize < beehiveMinCells) {
				cout << endl << "A beehive cannot appear in a " << xSpaces << "x" << ySpaces << " grid.";
				cout << endl << "ERN for Block is: " << ern;
				break;
			}
			else {
				cout << endl << "The ERN for a block or a beehive in a " << xSpaces << "x" << ySpaces << " grid is: " << ern;
				break;
			}
			break;
		}

		case 2:
		{
			int blinkerMinCells = minGridForPattern["Blinker"];
			int toadMinCells = minGridForPattern["Toad"];
			int gridSize = xSpaces * ySpaces;

			if (gridSize < blinkerMinCells && gridSize < toadMinCells)
			{
				cout << endl << "A blinker or a toad cannot appear in a " << xSpaces << "x" << ySpaces << " grid. ERN Unavailable.";
				break;
			}
			else if (gridSize >= blinkerMinCells && gridSize < toadMinCells)
			{
				cout << endl << "A toad cannot appear in a " << xSpaces << "x" << ySpaces << " grid.";
				cout << endl << "ERN for blinker is: " << ern;
				break;
			}
			else
			{
				cout << endl << "The ERN for a blinker or a toad in a " << xSpaces << "x" << ySpaces << " grid is: " << ern;
				break;
			}
			break;
		}
		case 3:
		{
			int gliderMinCells = minGridForPattern["Glider"];
			int lwssMinCells = minGridForPattern["LWSS"];
			int gridSize = xSpaces * ySpaces;

			if (gridSize < gliderMinCells && gridSize < lwssMinCells)
			{
				cout << endl << "A glider or a LWSS cannot appear in a " << xSpaces << "x" << ySpaces << " grid. ERN Unavailable";
				break;
			}
			else if (gridSize >= gliderMinCells && gridSize < lwssMinCells)
			{
				cout << endl << "A LWSS cannot appear in a " << xSpaces << "x" << ySpaces << " grid.";
				cout << "The ERN for a glider is: " << ern;
				break;
			}
			else
			{
				cout << endl << "The ERN for a glider or a LWSS in a " << xSpaces << "x" << ySpaces << " grid is: " << ern;
				break;
			}
			break;
		}

		default:
			cout << endl << "Unknown pattern. ERN Unavailable";
		}
	}

	cout << endl << "The ERN for a " << xSpaces << "x" << ySpaces << " grid with " << totalCells << " live cells is: " << ern;
}

// Calculates and displays the lowest possible ERN for all paterns by checking all possible grid sizes
void displayLowestPossibleERN()
{
	vector<vector<bool>> block = {
		{true, true},
		{true, true},
	};

	vector<vector<bool>> beehive = {
		{false, true, true, false},
		{true, false, false, true},
		{false, true, true, false}
	};

	vector<vector<bool>> blinker =
	{
		{true},
		{true},
		{true}
	};

	vector<vector<bool>> toad =
	{
		{false, true, true, true},
		{true, true, true, false}
	};

	vector<vector<bool>> glider =
	{
		{false, true, false},
		{false, false, true},
		{true, true, true}
	};

	vector<vector<bool>> lwss =
	{
		{false, true, false, false, true},
		{true, false, false, false, false},
		{true, false, false, false, true},
		{true, true, true, true, false}
	};

	int maxGridSize = 10;
	vector<vector<vector<bool>>> patterns = { block, beehive, blinker, toad, glider, lwss };
	vector<string> patternNames = { "Block", "Beehive", "Blinker", "Toad", "Glider", "LWSS" };

	for (int p = 0; p < patterns.size(); ++p) {
		const auto& pattern = patterns[p];
		int lowestErn = INT_MAX; // initialise to large number to check if ern is lower
		int bestWidth = 0;
		int bestHeight = 0;
		int totalAliveCells = 0;

		// Count only alive cells from pattern
		for (const auto& row : pattern)
		{
			for (bool cell : row)
			{
				if (cell)
				{
					totalAliveCells++;
				}
			}
		}

		for (int rows = 1; rows < maxGridSize + 1; ++rows)
		{
			for (int cols = 1; cols < maxGridSize + 1; ++cols)
			{
				// Check if the pattern fits in the grid
				int pHeight = pattern.size();
				int pWidth = pattern[0].size();
				if (pHeight < rows + 1 && pWidth < cols + 1)
				{
					//calculate ern if better set variables as new best grid size and ern
					int ern = rows + cols + totalAliveCells;
					if (ern < lowestErn) {
						lowestErn = ern;
						bestWidth = rows;
						bestHeight = cols;
					}
				}
			}
		}
		cout << endl << "The lowest possible ERN for a " << patternNames[p] << " in a " << bestWidth << "x" << bestHeight << " grid is: " << lowestErn;
	}

}

// SAVE FUNCTIONS

// Saves the Grid onto the system storage. Creates a .txt file with user defined filename
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
		// save the grid from main grid into txt file
		gridSaveFile << grid;
	}
	gridSaveFile.close();
}

// Saves the paramaters used to generate a simulation. Creates a .CSV file with user defined filename
template <typename T>
void saveParameters(Grid<T>& grid, unsigned int seed, int totalCells, int totalCycles)
{
	// Saves the parameters to generate the case again
	string filename;
	int rows;
	int cols;
	cout << endl << "Enter file name: ";
	cin >> filename;
	
	ofstream parametersSaveFile(filename + ".csv");
	if (parametersSaveFile.is_open())
	{
		rows = grid.size();
		parametersSaveFile << rows << ",";
		cols = grid[0].size();
		parametersSaveFile << cols << ",";
		parametersSaveFile << seed << ",";
		parametersSaveFile << totalCells << ",";
		parametersSaveFile << totalCycles << ",";
	}

	parametersSaveFile.close();
}

// LOAD FUNCTIONS

// Loads a .txt file from the system storage and updates the grid based on the position of the cells inside the text file.
template <typename T>
bool loadGridSimulation(Grid<T> &grid) 
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
			cin >> ClearAndIgnore();
			return false;
		}

	// Clears the grid allowing for the loaded layout to be copied onto it.

	cleanupGrid(grid);
	grid.clear();


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

// Loads a .csv file from the system storage and stores the values into a CSVData class to pass into simulation
CSVData LoadParamSimulation()
{
	string filename;
	string line;
	ifstream paramLoadFile;

	do {

		cout << endl << "Enter file name to load: ";
		cin >> filename;

		paramLoadFile.open(filename + ".csv");

		if (!paramLoadFile.is_open())
		{
			cout << endl << "Error: Unable to open the file.";
			cin >> ClearAndIgnore();

		}
	} while (!paramLoadFile.is_open());

	if (getline(paramLoadFile, line))
	{
		stringstream ss(line);
		string token;
		int xSpaces;
		int ySpaces;
		int totalCells;
		int totalCycles;
		unsigned int seed;

		getline(ss, token, ',');
		xSpaces = stoi(token);

		getline(ss, token, ',');
		ySpaces = stoi(token);

		getline(ss, token, ',');
		seed = static_cast<unsigned int>(stoul(token));

		getline(ss, token, ',');
		totalCycles = stoi(token);

		getline(ss, token, ',');
		totalCells = stoi(token);

		return CSVData(xSpaces, ySpaces, seed, totalCycles, totalCells);

	}




}

// TEST FUNCTIONS

template <typename T>
// test to ensure isBlockOrBeehive functions as intended. Outputs to console if successful.
void test_isBlockOrBeehive(Grid<T>& grid)
{
	// Test empty

	bool foundNothing = isBlockOrBeehive(grid);
	assert(foundNothing == false);


	// Test Block
	grid[1][1]->setAlive(true);
	grid[2][1]->setAlive(true);
	grid[1][2]->setAlive(true);
	grid[2][2]->setAlive(true);

	bool foundBlock = isBlockOrBeehive(grid);
	assert(foundBlock == true);

	grid[1][1]->setAlive(false);
	grid[2][1]->setAlive(false);
	grid[1][2]->setAlive(false);
	grid[2][2]->setAlive(false);

	// Test Beehive
	grid[2][0]->setAlive(true);
	grid[1][1]->setAlive(true);
	grid[3][1]->setAlive(true);
	grid[1][2]->setAlive(true);
	grid[3][2]->setAlive(true);
	grid[2][3]->setAlive(true);

	bool foundBeehive = isBlockOrBeehive(grid);
	assert(foundBeehive == true);

	grid[2][0]->setAlive(false);
	grid[1][1]->setAlive(false);
	grid[3][1]->setAlive(false);
	grid[1][2]->setAlive(false);
	grid[3][2]->setAlive(false);
	grid[2][3]->setAlive(false);

	cout << endl << "All tests passed for isBlockOrBeehive()";

}

// test to ensure isBlinkerOrToad functions as intended. Outputs to console if successful (toads were rare >:| )
template <typename T>
void test_isBlinkerOrToad(Grid<T>& grid)
{
	// Test empty

	bool foundNothing = isBlinkerOrToad(grid);
	assert(foundNothing == false);

	// Test Blinker
	grid[1][1]->setAlive(true);
	grid[1][2]->setAlive(true);
	grid[1][3]->setAlive(true);

	bool foundBlinker = isBlinkerOrToad(grid);
	assert(foundBlinker == true);

	grid[1][1]->setAlive(false);
	grid[1][2]->setAlive(false);
	grid[1][3]->setAlive(false);

	// Test Toad

	grid[0][1]->setAlive(true);
	grid[0][2]->setAlive(true);
	grid[1][3]->setAlive(true);
	grid[2][0]->setAlive(true);
	grid[3][1]->setAlive(true);
	grid[3][2]->setAlive(true);

	bool foundToad = isBlinkerOrToad(grid);
	assert(foundToad == true);

	grid[0][1]->setAlive(false);
	grid[0][2]->setAlive(false);
	grid[1][3]->setAlive(false);
	grid[2][0]->setAlive(false);
	grid[3][1]->setAlive(false);
	grid[3][2]->setAlive(false);

	cout << endl << "All tests passed for isBlinkerOrToad()";

}

// test to ensure isGliderOrLWSS functions as intended. Outputs to console if successful (LWSS impossiblely rare >>:( )
template <typename T>
void test_isGliderOrLWSS(Grid<T>& grid)
{
	// Test empty

	bool foundNothing = isGliderOrLWSS(grid);
	assert(foundNothing == false);


	// Test Glider
	grid[2][1]->setAlive(true);
	grid[3][2]->setAlive(true);
	grid[1][3]->setAlive(true);
	grid[2][3]->setAlive(true);
	grid[3][3]->setAlive(true);

	bool foundGlider = isGliderOrLWSS(grid);
	assert(foundGlider == true);

	grid[2][1]->setAlive(false);
	grid[3][2]->setAlive(false);
	grid[1][3]->setAlive(false);
	grid[2][3]->setAlive(false);
	grid[3][3]->setAlive(false);

	// Test LWSS

	grid[1][0]->setAlive(true);
	grid[4][0]->setAlive(true);
	grid[0][1]->setAlive(true);
	grid[0][2]->setAlive(true);
	grid[4][2]->setAlive(true);
	grid[0][3]->setAlive(true);
	grid[1][3]->setAlive(true);
	grid[2][3]->setAlive(true);
	grid[3][3]->setAlive(true);

	bool foundLWSS = isGliderOrLWSS(grid);
	assert(foundLWSS == true);

	grid[1][0]->setAlive(false);
	grid[4][0]->setAlive(false);
	grid[0][1]->setAlive(false);
	grid[0][2]->setAlive(false);
	grid[4][2]->setAlive(false);
	grid[0][3]->setAlive(false);
	grid[1][3]->setAlive(false);
	grid[2][3]->setAlive(false);
	grid[3][3]->setAlive(false);

	cout << endl << "All tests passed for isGliderOrLWSS()";
}

// INPUT FUNCTIONS

// function to get number of cycles - created to help other functions
int cycleInput()
{
	bool allowedInput = false;
	int totalCycles;
	while (!allowedInput)
	{
		cout << endl << "Enter the number of phases to run: ";
		cin >> totalCycles;

		if (isValidInput(totalCycles))
		{
			allowedInput = true;
		}
	}
	return totalCycles;
}

// function to get number of cells - created to help other functions
int cellInput()
{
	bool allowedInput = false;
	int numCells;
	while (!allowedInput)
	{
		cout << endl << "Enter the number of alive cells: ";
		cin >> numCells;

		if (isValidInput(numCells))
		{
			allowedInput = true;
		}
	}
	return numCells;
}

// MENU FUNCTIONS

// displays the options for loading
int menu_displayLoadMenu()
{
	bool choosing = true;
	int choice;

	while (choosing)
	{
		cout << endl << "|| 1. (.txt) Continue previous grid";
		cout << endl << "|| 2. (.csv) Repeat previous simulation";
		cout << endl << "|| Select the file type you would like to load: ";
		cin >> choice;

		switch (choice)
		{
		case 1:
			choosing = false;
			return choice;
		case 2:
			choosing = false;
			return choice;
		default:
			cout << "Error: Invalid Option. Please try again.";
		}
	}
}

// runs the algorithm for creating a new simulation
template <typename T>
void menu_createNewSimulation(Grid<T> &grid)
{
	grid = generateGrid<bool>(nullptr, nullptr);
	random_device rd; // Generate new seed
	unsigned int seed = rd();
	int totalCells = cellInput();
	int totalCycles = cycleInput();

	createCells(grid);
	scatterCells(grid, totalCells, seed);
	runSimulation(grid, totalCycles);
	calculateERN(grid, totalCells, nullptr);
	menu_displaySaveMenu(grid, seed, totalCycles, totalCells);
}

// runs the algorithm for loading a grid from storage
template <typename T>
void menu_loadGridFromStorage(Grid<T>& grid)
{
	if (loadGridSimulation(grid))
	{
		int totalCycles = cycleInput();
		runSimulation(grid, totalCycles);
		cout << grid;
		menu_displaySaveMenuNoParams(grid);
	}
}

// runs the algorithm for loading params from storage
template <typename T>
void menu_loadCSVFromStorage(Grid<T>& grid)
{
	CSVData loadedParams = LoadParamSimulation();

	int xSpaces = loadedParams.getXSpaces();
	int ySpaces = loadedParams.getYSpaces();
	unsigned int seed = loadedParams.getSeed();
	int totalCells = loadedParams.getTotalCells();
	int totalCycles = loadedParams.getTotalCycles();

	grid = generateGrid<bool>(&xSpaces, &ySpaces);

	createCells(grid);
	scatterCells(grid, totalCells, seed);
	runSimulation(grid, totalCycles);
	calculateERN(grid, totalCells, nullptr);
	menu_displaySaveMenu(grid, seed, totalCells, totalCycles);
}

// chooses which load method to use 
template <typename T>
void menu_loadFromStorage(Grid<T>& grid)
{
	int choice = menu_displayLoadMenu();
	switch (choice)
	{
		case 1:
			menu_loadGridFromStorage(grid);
			break;
		case 2:
			menu_loadCSVFromStorage(grid);
			break;
	}
}

// runs the algorithm for creating an experiment
template <typename T>
void menu_runExperiment(Grid<T>& grid)
{
	grid = generateGrid<bool>(nullptr, nullptr);
	runExperiment(grid);
}

// runs the pattern tests
template <typename T>
void menu_runPatternTests(Grid<T>& grid)
{
	int xSpaces = 5;
	int ySpaces = 5;
	grid = generateGrid<bool>(&xSpaces, &ySpaces);
	createCells(grid);
	test_isBlockOrBeehive(grid);
	test_isBlinkerOrToad(grid);
	test_isGliderOrLWSS(grid);
}

// runs the lowest possible ern function
void menu_findLowestPossibleERN()
{
	displayLowestPossibleERN();
}

// displays the save menu options
template <typename T>
void menu_displaySaveMenu(Grid<T> &grid, unsigned int seed, int totalCells, int totalCycles)
{
	bool saving = true;
	int choice;
	cout << endl << "Simulation Finished!";

	while (saving)
	{
		cout << endl << "Would you like to save the final grid or save the parameters?";
		cout << endl << "|| 1. Save Final Grid";
		cout << endl << "|| 2. Save Parameters";
		cout << endl << "|| 3. Don't Save";
		cout << endl << "Select an option: ";
		cin >> choice;
		switch (choice)
		{
			case 1:
				saveSimulation(grid);
				saving = false;
				break;
			case 2:
				saveParameters(grid, seed, totalCells, totalCycles);
				saving = false;
				break;
			case 3:
				saving = false;
				break;
			default:
				cout << "Error: Invalid Option. Please try again.";
				cin >> ClearAndIgnore();
				break;
		}
	}
}

// displays the save menu but can only be used on grids that have no params such as saved .txt 
template <typename T>
void menu_displaySaveMenuNoParams(Grid<T> grid)
{
	bool saving = true;
	int choice;
	cout << endl << "Simulation Finished!";

	while (saving)
	{
		cout << endl << "Would you like to save the final grid?";
		cout << endl << "|| 1. Save Final Grid";
		cout << endl << "|| 2. Don't Save";
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
			cin >> ClearAndIgnore();
			break;
		}
	}
}

// displays the option menu
int menu_displayOptionMenu()
{
	int choice;

	cout << endl << "|| 1. Create New Simulation";
	cout << endl << "|| 2. Load Simulation from Storage";
	cout << endl << "|| 3. Run experiment to find pattern";
	cout << endl << "|| 4. Test Functions";
	cout << endl << "|| 5. Calculate lowest possible efficiency resource number (ERN)";
	cout << endl << "|| 6. Exit";
	cout << endl << "|| Select an option: ";

	cin >> choice;
	return choice;
}

// displays the welcome menu
template <typename T>
void menu_displayWelcomeMenu(Grid<T> &grid)
{
	int xSpaces = 5;
	int ySpaces = 5;
	int choice;
	bool running = true;
	unsigned int seed = 0;

	cout << "|| Welcome to Ryan's version of John Conway's: Game of Life! ||";

	while (running)
	{
		choice = menu_displayOptionMenu();
		

		switch (choice)
		{
			case 1:
				menu_createNewSimulation(grid);
				break;
			case 2:
				menu_loadFromStorage(grid);
				break;
			case 3:
				menu_runExperiment(grid);
				break;
			case 4:
				menu_runPatternTests(grid);
				break;
			case 5:
				menu_findLowestPossibleERN();
				break;
			case 6:
				running = false; // Quit the loop;
				break;
			default:
				cout << endl << "Error: Invalid Option. Please try again.";
				cin >> ClearAndIgnore();
				break;
		}
	}
}

// main :)
int main()
{
	// initilises the grid 
	Grid<bool> grid;

	menu_displayWelcomeMenu(grid);

	cleanupGrid(grid); // Always clean up before exiting.

	return 0;
}