#include <iostream>
#include <random>
#include <cstdlib>
#include <cassert>
#include <chrono>
#include <thread>
#include <mutex>
#include "gif.h"
#include "Grid.h"

// Maybe add a param here for switching between configs
void Grid::populateGrid() {
	std::cout << "Populating grid" << std::endl;
	// Current strategy - init at random
	std::vector<std::vector<bool>> map(this->height, std::vector(this->width, false));

	for (unsigned i = 1; i <= numFish; i++) {
		unsigned row = std::rand() % height;
		unsigned col = std::rand() % width;

		if (map[row][col]) {
			i--;
			continue;
		}
		map[row][col] = 1;
		grid[row][col] = Cell(row, col, CellType::fish);
	}
	for (unsigned i = 1; i <= numSharks; i++) {
		unsigned row = std::rand() % height;
		unsigned col = std::rand() % width;

		if (map[row][col]) {
			i--;
			continue;
		}
		map[row][col] = 1;
		grid[row][col] = Cell(row, col, CellType::shark);
	}
}

// currentPos == newPos if there is any
// currentPos == itself otherwise
void Grid::generatePos(std::pair<int, int>& currentPos, std::vector<std::pair<int, int>>& dirs, unsigned random) {
	if (grid[currentPos.first][currentPos.second].type == fish) {
		std::vector<std::pair<int, int>> pos;
		for (auto& [i, j] : dirs) {
			int newI = (currentPos.first + i + height) % height;
			int newJ = (currentPos.second + j + width) % width;

			// shark < fish < empty
			if (grid[currentPos.first][currentPos.second].type < grid[newI][newJ].type) {
				pos.push_back({ newI, newJ });
			}
		}
		if (!pos.size()) return;
		currentPos = pos[random % pos.size()]; // Not truly uniform but will do for now
	}
	else {
		std::vector<std::pair<int, int>> fishPos;
		std::vector<std::pair<int, int>> emptyPos;
		for (auto& [i, j] : dirs) {
			int newI = (currentPos.first + i + height) % height;
			int newJ = (currentPos.second + j + width) % width;

			// shark < fish < empty
			if (grid[newI][newJ].type == fish) {
				fishPos.push_back({ newI, newJ });
			}
			else if (grid[newI][newJ].type == empty) {
				emptyPos.push_back({ newI, newJ });
			}
		}
		if (fishPos.size()) {
			currentPos = fishPos[random % fishPos.size()];
			return;
		}
		else if (emptyPos.size()) {
			currentPos = emptyPos[random % emptyPos.size()];
			return;
		}
	}
}

void Grid::timeStep(const unsigned numThreads) {
	unsigned rowsPerThread = height / numThreads;
	unsigned threadsToSpawn = numThreads - 1;

	std::vector<std::thread> threads(threadsToSpawn);

	// mutex[i] synchronizes between thread i and thread i - 1
	std::vector<std::mutex> mutexes(threadsToSpawn + 1);
	unsigned beginRow = 0;

	for (unsigned threadIdx = 0; threadIdx < threadsToSpawn; threadIdx++) {

		threads[threadIdx] = std::thread([this, beginRow, rowsPerThread, &mutexes, threadIdx]() -> void {
			this->doWork(beginRow, beginRow + rowsPerThread - 1, mutexes, threadIdx, threadIdx + 1);
		});

		beginRow += rowsPerThread;
	}

	// Main thread is reused here
	doWork(beginRow, height - 1, mutexes, threadsToSpawn, 0);

	for (auto& t : threads) {
		t.join();
	}
}

void Grid::timeStepAlternate(const unsigned numThreads) {
	if (numThreads == 1) {
		this->doWorkAlternate(0, height - 1);
		return;
	}
	unsigned threadsToSpawn = numThreads - 1;
	unsigned rowsPerThread = height / (2 * numThreads);
	unsigned remainder = height % (2 * numThreads);
	std::vector<std::thread> threads(threadsToSpawn);

	unsigned beginRow = 0;
	unsigned mainThreadBegin = beginRow;
	unsigned mainThreadEnd = beginRow + rowsPerThread - 1 + remainder / 2;

	int parity = 1;
	while (parity <= 2) {
		if (parity == 1) {
			beginRow = 0;
		}
		else {
			beginRow = rowsPerThread;
		}
		for (unsigned threadIdx = 0; threadIdx < threadsToSpawn; threadIdx++) {
			threads[threadIdx] = std::thread([this, beginRow, rowsPerThread]() -> void {
				this->doWorkAlternate(beginRow, beginRow + rowsPerThread - 1);
				});
			// Skip the other parity sectors
			beginRow += 2 * rowsPerThread;
		}

		if (parity == 1) {
			mainThreadBegin = beginRow;
			mainThreadEnd = beginRow + rowsPerThread - 1 + remainder / 2;
		}
		else {
			mainThreadBegin = beginRow + remainder / 2;
			mainThreadEnd = height - 1;
		}
		// Main thread is reused here
		doWorkAlternate(mainThreadBegin, mainThreadEnd);
		for (auto& t : threads) {
			t.join();
		}
		parity++;
	}
}

void Grid::doWork(size_t rowBegin, size_t rowEnd, std::vector<std::mutex>& mutexes, const unsigned currentThreadIdx, const unsigned nextThreadIdx) {

	std::vector<std::pair<int, int>> dirs = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 } };

	for (size_t i = rowBegin; i <= rowEnd; i++) {
		if (i == rowBegin) {
			mutexes[currentThreadIdx].lock();
		}
		if (i == rowEnd) {
			mutexes[nextThreadIdx].lock();
		}
		for (size_t j = 0; j < width; j++) {
			if (grid[i][j].type == CellType::empty || grid[i][j].hasMoved) continue;

			std::pair<int, int> currentPos = { i, j };
			generatePos(currentPos, dirs, std::rand());//rand(gen));//std::rand());//rand(gen));

			auto [newI, newJ] = currentPos;

			// Move
			if (currentPos != std::pair<int, int>({ i, j })) {
				// Feed
				if (grid[i][j].type == CellType::shark && grid[newI][newJ].type == CellType::fish) {
					grid[i][j].energy = MAX_ENERGY;
				}
				grid[i][j].hasMoved = true;
				grid[newI][newJ] = grid[i][j];

				// Reproduce
				if (grid[i][j].reproductionTime == REPRODUCTION_TIME) {
					grid[newI][newJ].reproductionTime = 0;
					grid[i][j] = grid[newI][newJ].birth();
					grid[i][j].hasMoved = true; // Maybe not needed
				}
				else {
					grid[newI][newJ].reproductionTime++;
					grid[i][j] = Cell(i, j, CellType::empty);
				}
			}

			// Die
			if (grid[newI][newJ].type == CellType::shark) {
				if (grid[newI][newJ].energy == 0) {
					grid[newI][newJ] = Cell(newI, newJ, CellType::empty);
				}
				else {
					grid[newI][newJ].energy--;
				}
			}
		}
		if (i == rowBegin) {
			mutexes[currentThreadIdx].unlock();
		}
		if (i == rowEnd) {
			mutexes[nextThreadIdx].unlock();
		}
	}
}

void Grid::doWorkAlternate(size_t rowBegin, size_t rowEnd) {
	std::vector<std::pair<int, int>> dirs = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 } };

	for (size_t i = rowBegin; i <= rowEnd; i++) {
		for (size_t j = 0; j < width; j++) {
			if (grid[i][j].type == CellType::empty || grid[i][j].hasMoved) continue;

			std::pair<int, int> currentPos = { i, j };
			generatePos(currentPos, dirs, std::rand());

			auto [newI, newJ] = currentPos;

			// Move
			if (currentPos != std::pair<int, int>({ i, j })) {
				// Feed
				if (grid[i][j].type == CellType::shark && grid[newI][newJ].type == CellType::fish) {
					grid[i][j].energy = MAX_ENERGY;
				}
				grid[i][j].hasMoved = true;
				grid[newI][newJ] = grid[i][j];

				// Reproduce
				if (grid[i][j].reproductionTime == REPRODUCTION_TIME) {
					grid[newI][newJ].reproductionTime = 0;
					grid[i][j] = grid[newI][newJ].birth();
					grid[i][j].hasMoved = true; // Maybe not needed
				}
				else {
					grid[newI][newJ].reproductionTime++;
					grid[i][j] = Cell(i, j, CellType::empty);
				}
			}

			// Die
			if (grid[newI][newJ].type == CellType::shark) {
				if (grid[newI][newJ].energy == 0) {
					grid[newI][newJ] = Cell(newI, newJ, CellType::empty);
				}
				else {
					grid[newI][newJ].energy--;
				}
			}
		}
	}
}


void Grid::simulate(const unsigned numThreads, const bool timeit, const bool alternate, std::string filename) {
	assert(numThreads > 0);
	std::chrono::duration <double, std::milli> ms;

	GifWriter gif;
	if (!timeit) {
		GifBegin(&gif, filename.c_str(), width, height, frameLen);
	}

	auto startTime = std::chrono::high_resolution_clock::now();

	for (unsigned t = 0; t < numFrames; t++) {
		if (t != 0) {
			if (alternate) {
				this->timeStepAlternate(numThreads);
			}
			else {
				this->timeStep(numThreads);
			}
			// Reset grid state
			for (size_t i = 0; i < height; i++) {
				for (size_t j = 0; j < width; j++) {
					grid[i][j].hasMoved = false;
				}
			}
		}
		// Don't draw when we are timing
		if (!timeit) {
			std::vector<uint8_t> display(width * height * 4);
			gridToDisplay(grid, display);
			GifWriteFrame(&gif, display.data(), width, height, frameLen);
		}
	}
	
	auto endTime = std::chrono::high_resolution_clock::now();
	ms = endTime - startTime;

	if (timeit) {
		std::cout << "TIME: " << ms.count() << " ms" << std::endl;
	}
	else {
		GifEnd(&gif);
	}
}

void Grid::gridToDisplay(std::vector<std::vector<Cell>>& grid, std::vector<uint8_t>& display) {
	std::vector<std::vector<std::vector<uint8_t>>> pixels(height, std::vector<std::vector<uint8_t>>(width, { 0, 0, 0, 255}));
	for (size_t i = 0; i < height; i++) {
		for (size_t j = 0; j < width; j++) {
			if (grid[i][j].type != CellType::empty) {
				pixels[i][j][grid[i][j].type] = 255;
			}
			for (size_t k = 0; k < 4; k++) {
				// This is weird but we need to transpose it
				display[(i * width + j) * 4 + k] = pixels[i][j][k];
			}
		}
	}
}