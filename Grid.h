#ifndef GRID
#define GRID
#include <vector>
#include <tuple>
#include <string>
#include <mutex>
#include "Cell.h"

class Grid {
public:
	std::vector<std::vector<Cell>> grid;
	const unsigned width;
	const unsigned height;
	const unsigned numFrames;
	const unsigned numFish;
	const unsigned numSharks;
	const unsigned frameLen;

	Grid(const unsigned w, 
		const unsigned h, 
		const unsigned numFish, 
		const unsigned numSharks,
		const unsigned numFrames,
		const unsigned frameLen) 
	: width(w), height(h), numFish(numFish), numSharks(numSharks), numFrames(numFrames + 1), frameLen(frameLen), grid(std::vector(h, std::vector<Cell>(w))) {}

	void populateGrid();
	void simulate(const unsigned numThreads, const bool timeit, const bool alternate, std::string filename);
	void gridToDisplay(std::vector<std::vector<Cell>>& grid, std::vector<uint8_t>& display);
private:
	void doWork(size_t rowBegin, size_t rowEnd, std::vector<std::mutex>& mutexes, const unsigned currentThreadIdx, const unsigned nextThreadIdx);
	void doWorkAlternate(size_t rowBegin, size_t rowEnd);
	void generatePos(std::pair<int, int>& currentPos, std::vector<std::pair<int, int>>& dirs, unsigned randomNum);
	// Maybe add a param here for switching between configs
	void timeStep(const unsigned numThreads);
	void timeStepAlternate(const unsigned numThreads);
};


#endif // !GRID
