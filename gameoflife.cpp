#include <iostream>
#include <vector>

using namespace std;

class Cell 
{
	private:
		bool isAlive;
	public:
		int xPosition;
		int yPosition;
		char icon = 'O';

		Cell()
		{
			xPosition = 0;
			yPosition = 0;
			isAlive = true;
			icon = 'O';
		}
		Cell(int xPosition, int yPosition)
		{
			this->xPosition = xPosition;
			this->yPosition = yPosition;
			isAlive = true;
		}
		void birth() {}
		int numberOfNeighbours() {}
		void death() { icon = ' ';}
};

vector<vector<Cell>> generateGrid()
{
	// Generates the size of the grid based on user inputs.
	int xSpaces;
	int ySpaces;

	cout << endl << "Enter number of spaces on the X Axis";
	cin >> xSpaces;

	cout << endl << "Enter number of spaces on the Y Axis";
	cin >> ySpaces;

	vector<vector<Cell>> grid(xSpaces, vector<Cell>(ySpaces));
	

	return grid;
}
void createCells(vector<vector<Cell>> cellGrid) 
{
	for (int x = 0; x < cellGrid.size(); x++)
		for (int y = 0; y < cellGrid[x].size(); y++)
		{
			cellGrid[x][y] = *(new Cell());
		}
}
void outputGrid(vector<vector<Cell>> cellGrid)
{
	// Outputs the grid to the command line.
	cout << endl;

	for (int x = 0; x < cellGrid.size(); x++)
	{
		for (int y = 0; y < cellGrid[x].size(); y++)
		{
			cout << "." << cellGrid[x][y].icon;
		}
		cout << "." << endl;
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
	outputGrid(grid);
	return 0;
}