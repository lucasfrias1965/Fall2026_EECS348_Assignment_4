#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class SudokuSolver {
private:
    static const int SIZE = 9;
    static const int BLOCK_SIZE = 3;
    int grid[SIZE][SIZE];
    std::string filename;
    int solutionsFound;

    // Checks if placing 'num' at grid[row][col] is valid
    bool isValid(int row, int col, int num) const {
        for (int i = 0; i < SIZE; ++i) {
            // Check row and column
            if (grid[row][i] == num || grid[i][col] == num) {
                return false;
            }
        }

        // Check 3x3 block
        int startRow = (row / BLOCK_SIZE) * BLOCK_SIZE;
        int startCol = (col / BLOCK_SIZE) * BLOCK_SIZE;
        for (int r = 0; r < BLOCK_SIZE; ++r) {
            for (int c = 0; c < BLOCK_SIZE; ++c) {
                if (grid[startRow + r][startCol + c] == num) {
                    return false;
                }
            }
        }

        return true;
    }

    // Recursive helper to find all solutions using depth-first search with backtracking
    void solveRecursive(int row, int col) {
        // If we reached past the last row, a complete solution is found
        if (row == SIZE) {
            solutionsFound++;
            std::cout << "\nSolution " << solutionsFound << ":\n";
            printGrid();
            return;
        }

        // Calculate next cell position
        int nextRow = (col == SIZE - 1) ? row + 1 : row;
        int nextCol = (col == SIZE - 1) ? 0 : col + 1;

        // If cell is already filled, move to next cell
        if (grid[row][col] != 0) {
            solveRecursive(nextRow, nextCol);
            return;
        }

        // Try candidates 1 through 9
        for (int num = 1; num <= SIZE; ++num) {
            if (isValid(row, col, num)) {
                grid[row][col] = num;
                solveRecursive(nextRow, nextCol);
                grid[row][col] = 0; // Backtrack
            }
        }
    }

public:
    SudokuSolver(const std::string& file) : filename(file), solutionsFound(0) {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                grid[r][c] = 0;
            }
        }
    }

    // Reads the grid from the file
    bool loadPuzzle() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        char cell;
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (!(file >> cell)) {
                    std::cerr << "Error: Invalid or incomplete puzzle format in " << filename << std::endl;
                    return false;
                }
                if (cell == '_') {
                    grid[r][c] = 0;
                } else if (cell >= '1' && cell <= '9') {
                    grid[r][c] = cell - '0';
                } else {
                    std::cerr << "Error: Invalid character '" << cell << "' in " << filename << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

    // Prints the grid
    void printGrid() const {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (grid[r][c] == 0) {
                    std::cout << "_ ";
                } else {
                    std::cout << grid[r][c] << " ";
                }
            }
            std::cout << "\n";
        }
    }

    // Primary object method to run solving routine and output formatted results
    void solve() {
        std::cout << "========================================\n";
        std::cout << "File: " << filename << "\n";
        std::cout << "----------------------------------------\n";
        std::cout << "Initial Puzzle:\n";
        printGrid();

        solutionsFound = 0;
        solveRecursive(0, 0);

        if (solutionsFound == 0) {
            std::cout << "\nNo solution found\n";
        } else {
            std::cout << "\nSolutions found: " << solutionsFound << "\n";
        }
        std::cout << "========================================\n\n";
    }
};

int main() {
    std::vector<std::string> puzzleFiles = {
        "puzzle1.txt",
        "puzzle2.txt",
        "puzzle3.txt",
        "puzzle4.txt",
        "puzzle5.txt"
    };

    for (const auto& filename : puzzleFiles) {
        SudokuSolver solver(filename);
        if (solver.loadPuzzle()) {
            solver.solve();
        }
    }

    return 0;
}
