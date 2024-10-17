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
#include <unordered_set>
#include <cassert>
#include <sstream>

using namespace std;

mutex mtx;

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

		void setAlive(T status) override { this->status = status; }

		// Returns a 'O' if the cell is alive, ' ' if the cell is dead.
		char getIcon() const override
		{
			return status ? 'O' : ' ';
		}

};

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

		int getXSpaces() const { return xSpaces; }
		int getYSpaces() const { return ySpaces; }
		unsigned int getSeed() const { return seed; }
		int getTotalCycles() const { return totalCycles; }
		int getTotalCells() const { return totalCells; }

		
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
Grid<T> generateGrid(int* xSizePointer, int* ySizePointer)
{
	// Generates the size of the grid based on user inputs.
	int xSpaces;
	int ySpaces;

	if (xSizePointer == nullptr || ySizePointer == nullptr)
	{
		cout << endl << "Enter number of spaces on the X Axis: ";
		cin >> xSpaces;

		cout << endl << "Enter number of spaces on the Y Axis: ";
		cin >> ySpaces;
	}
	else
	{
		xSpaces = *xSizePointer;
		ySpaces = *ySizePointer;
	}

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
void scatterCells(Grid<T> &grid, int* numCellsPtr, unsigned int& seed)
{
	mt19937 gen(seed);
	uniform_int_distribution<> xDist(0, grid.size() - 1);
	uniform_int_distribution<> yDist(0, grid[0].size() - 1);
	int numCells;
	int totalCells = 0;

	if (numCellsPtr == nullptr) {
		cout << endl << "Enter the number of alive cells: ";
		cin >> numCells;
	}
	else
	{
		numCells = *numCellsPtr;
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
		reverse(hFlipped[i].begin(), hFlipped[i].end());
	}
	return hFlipped;
}

// Function that flips a pattern vertically
template <typename T>
vector<vector<T>> flipVertical(const vector<vector<T>>& pattern)
{
	vector<vector<T>> vFlipped = pattern;
	reverse(vFlipped.begin(), vFlipped.end());

	return vFlipped;
}

// Hashing function for 2D vector
template <typename T>
size_t hashPattern(const vector<vector<T>>& pattern)
{
	size_t seed = 0;
		for (const auto& row : pattern) 
		{
			for (const auto& elem : row) 
			{
				seed ^= hash<T>{}(elem)+0x9e3779b9 + (seed << 6) + (seed >> 2);
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

	auto addUniqueVariant = [&](const vector<vector<T>>& p)
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
			// Check for pattern at this position
			if (matchesPattern(grid, blockPattern, x, y) || matchesPattern(grid, beehivePattern, x, y))
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

	vector<vector<bool>> blinkerPhase2 =
	{
		{true},
		{true},
		{true}
	};

	// Don't need phase 2 as it is a rotation

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
			// Check for pattern at this position
			if (matchesPattern(grid, blinkerPhase1, x, y) || matchesPattern(grid, blinkerPhase2, x, y) || matchesPattern(grid, toadPhase1, x, y) || matchesPattern(grid, toadPhase2, x, y))
			{
				return true;
			}
		}
	}
	return false;

}

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
			// Check for pattern at this position
			if (matchesPattern(grid, gliderPhase1, x, y) || matchesPattern(grid, gliderPhase2, x, y) || matchesPattern(grid, lwssPhase1, x, y) || matchesPattern(grid, lwssPhase2, x, y))
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

		if (checkForDeadCells(grid))
		{
			cout << endl << "All cells have died. Stopping simulation.";
			break;
		}
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

int displayPatternMenu()
{
	int patternChoice;


	// Poorly implemented due to implementation with runExperiemtn will change if have time
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
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
	}
;

	return patternChoice;
}

template <typename T>
void runExperiment(Grid<T>& grid)
{
	int patternChoice = displayPatternMenu();
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
		scatterCells(grid, &totalCells, seed);

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
					}
					break;
				case 2:
					// Check for blinker or toad after each geneation of cells
					if (checkForStableOscillator(grid, stableGenerations, currentCycle))
					{
						patternFound = true;
						cout << endl << "Blinker or Toad detected in experiment #" << experimentCount << " after " << currentCycle << " generations!";
					}
					break;
				case 3:
					// Check for glider or Lwss after each generation of cells
					if (checkForStableSpaceship(grid, stableGenerations, currentCycle))
					{
						patternFound = true;
						cout << endl << "Glider or LWSS detected in experiment #" << experimentCount << " after " << currentCycle << " generations!";
					}
					break;


			}

			if (patternFound)
			{
				displaySaveMenu(grid, seed, totalCycles, totalCells);
				break;
			}
			

			if (checkForDeadCells(grid))
			{
				cout << grid;
				cout << endl << "All Cells for experiment #" << experimentCount << " have died. Starting next experiment.";
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


// Loads a file from the system storage and updates the grid based on the position of the cells inside the text file.
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
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
void testIsBlockOrBeehive(Grid<T>& grid)
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

template <typename T>
void testIsBlinkerOrToad(Grid<T>& grid)
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

template <typename T>
void testIsGliderOrLWSS(Grid<T>& grid)
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

int displayLoadMenu()
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

template <typename T>
void displaySaveMenu(Grid<T> &grid, unsigned int seed, int totalCells, int totalCycles)
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
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				break;
		}
	}


}


int displayOptionMenu()
{
	int choice;

	cout << endl << "|| 1. Create New Simulation";
	cout << endl << "|| 2. Load Simulation from Storage";
	cout << endl << "|| 3. Run experiment to find pattern";
	cout << endl << "|| 4. Test Functions";
	cout << endl << "|| 5. Exit";
	cout << endl << "|| Select an option: ";

	cin >> choice;
	return choice;
}
template <typename T>
void displayWelcomeMenu(Grid<T> &grid)
{
	int xSpaces = 5;
	int ySpaces = 5;
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
			{
				
				grid = generateGrid<bool>(nullptr, nullptr);
				random_device rd; // Generate new seed
				seed = rd();
				createCells(grid);
				scatterCells(grid, nullptr, seed);
				runSimulation(grid, nullptr);
				//displaySaveMenu(grid);
				break;
			}
			case 2:
				choice = displayLoadMenu();
				switch (choice)
				{
					case 1:

						grid = generateGrid<bool>(&xSpaces, &ySpaces);
						loadGridSimulation(grid);
						runSimulation(grid, nullptr);
						break;
					case 2:
					{
						CSVData loadedParams = LoadParamSimulation();
						xSpaces = loadedParams.getXSpaces();
						ySpaces = loadedParams.getYSpaces();
						unsigned int seed = loadedParams.getSeed();

						int totalCells = loadedParams.getTotalCells();
						int totalCycles = loadedParams.getTotalCycles();

						grid = generateGrid<bool>(&xSpaces, &ySpaces);
						
						createCells(grid);
						scatterCells(grid, &totalCells, seed);
						runSimulation(grid, &totalCycles);
						break;
					}
				}
				break;
			case 3:
				int patternChoice;
				grid = generateGrid<bool>(nullptr, nullptr);
				runExperiment(grid);
				break;
			case 4:
				grid = generateGrid<bool>(&xSpaces, &ySpaces);
				createCells(grid);
				testIsBlockOrBeehive(grid);
				testIsBlinkerOrToad(grid);
				testIsGliderOrLWSS(grid);
				break;
			case 5:
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