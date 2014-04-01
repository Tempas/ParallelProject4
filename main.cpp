#include "grid.cpp"
#include <iostream>

int testCount = 0;
int successCount = 0;


int main(int argc, const char * argv[])
{
	Grid grid(5, 4, 0, 1, GridDirectionRight);

	grid.printCars();
	grid.finishTimeStep();
	std::cout << std::endl;
	grid.printCars();
}