#include "Grid.h"
#include <iostream>
#include <exception>

unsigned WIDTH = 1920;
unsigned HEIGHT = 1080;
unsigned NUM_FISH = 200000;
unsigned NUM_SHARKS = 30000;
unsigned NUM_FRAMES = 1000;
unsigned FRAME_LEN = 4;
unsigned NUM_THREADS = 2;
bool ALTERNATE_IMPLEMENTATION = false;
bool TIME_IT = true;

void parseArguments(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (std::strlen(argv[i]) != 2) continue;
		if (argv[i][0] == '-') {
			if (argv[i][1] == 't') {
				TIME_IT = true;
				continue;
			}
			if (argv[i][1] == 'a') {
				ALTERNATE_IMPLEMENTATION = true;
				continue;
			}
			if (i == argc - 1) {
				continue;
			}

			int num = atoi(argv[i + 1]);
			if (num < 0) {
				throw std::invalid_argument("Cannot have negative numbers as arguments");
			}

			switch (argv[i][1]) {
				case 'w':
					WIDTH = num;
					break;
				case 'h':
					HEIGHT = num;
					break;
				case 'f':
					NUM_FISH = num;
					break;
				case 's':
					NUM_SHARKS = num;
					break;
				case 'c':
					NUM_FRAMES = num;
					break;
				case 'p':
					NUM_THREADS = num;
					break;
				default:
					throw std::invalid_argument("Invalid flag");
			}
			i++;
		}
	}
}

int main(int argc, char* argv[]) {
	parseArguments(argc, argv);
	std::cout << "Width: " << WIDTH << std::endl;
	std::cout << "Height: " << HEIGHT << std::endl;
	std::cout << "Number of fish: " << NUM_FISH << std::endl;
	std::cout << "Number of sharks: " << NUM_SHARKS << std::endl;
	std::cout << "Number of chronons: " << NUM_FRAMES << std::endl;
	std::cout << "Parallelism: " << NUM_THREADS << std::endl;
	std::cout << "Should time it: " << std::boolalpha << TIME_IT << std::endl;
	std::cout << "Alternate implementation: " << std::boolalpha << ALTERNATE_IMPLEMENTATION << std::endl;
	std::cout << std::endl;

	Grid grid(WIDTH, HEIGHT, NUM_FISH, NUM_SHARKS, NUM_FRAMES, FRAME_LEN);
	grid.populateGrid();
	grid.simulate(NUM_THREADS, TIME_IT, ALTERNATE_IMPLEMENTATION, "watorParallel3.gif");
}