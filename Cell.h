#ifndef  CELL
#define CELL

enum CellType { shark = 0, fish = 1, empty = 2 };
const int MAX_ENERGY = 30;
const int REPRODUCTION_TIME = 40;

struct Cell {
	unsigned i, j;
	unsigned reproductionTime;
	unsigned energy;
	bool hasMoved;
	CellType type;
	
	Cell() : i(0), j(0), reproductionTime(0), energy(MAX_ENERGY), type(CellType::empty), hasMoved(false) {}
	Cell(unsigned i, unsigned j, CellType type) : i(i), j(j), type(type), reproductionTime(0), energy(MAX_ENERGY), hasMoved(false) {}
	Cell birth() {
		return Cell(i, j, type);
	}
};

#endif // ! CELL