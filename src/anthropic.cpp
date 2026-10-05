// anthropic.cpp
// Object-oriented, recursive brute-force Sudoku solver (depth-first search
// with backtracking).
//
// Usage:
//   ./anthropic                      solves puzzle1.txt .. puzzle5.txt in the
//                                    current directory
//   ./anthropic <directory>          solves puzzle1.txt .. puzzle5.txt in
//                                    <directory>
//   ./anthropic a.txt b.txt ...      solves exactly the files listed
//
// Puzzle file format: 81 cells in row-major order. Digits 1-9 are filled
// cells and an underscore (_) is a blank cell. Whitespace is ignored.

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// A 9x9 Sudoku grid. Knows how to load and print itself and how to answer
// questions about which digits may legally be placed in a cell.
class SudokuBoard {
public:
    static const int SIZE = 9;
    static const int BLOCK = 3;
    static const int EMPTY = 0;

    SudokuBoard() {
        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c)
                cells[r][c] = EMPTY;
    }

    // Reads a puzzle from a file. Returns false (and sets error) if the file
    // cannot be opened or does not describe exactly 81 cells.
    bool loadFromFile(const std::string& path, std::string& error) {
        std::ifstream in(path.c_str());
        if (!in) {
            error = "could not open file";
            return false;
        }

        int count = 0;
        char ch;
        while (in >> ch) {  // operator>> skips whitespace
            int value;
            if (ch == '_') {
                value = EMPTY;
            } else if (ch >= '1' && ch <= '9') {
                value = ch - '0';
            } else {
                error = std::string("unexpected character '") + ch + "'";
                return false;
            }
            if (count >= SIZE * SIZE) {
                error = "more than 81 cells in file";
                return false;
            }
            cells[count / SIZE][count % SIZE] = value;
            ++count;
        }

        if (count != SIZE * SIZE) {
            error = "expected 81 cells but found " + std::to_string(count);
            return false;
        }
        return true;
    }

    int get(int row, int col) const { return cells[row][col]; }
    void set(int row, int col, int value) { cells[row][col] = value; }
    void clear(int row, int col) { cells[row][col] = EMPTY; }
    bool isEmpty(int row, int col) const { return cells[row][col] == EMPTY; }

    // Finds the first empty cell in row-major order. Returns false if the
    // board is completely filled.
    bool findEmptyCell(int& row, int& col) const {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (cells[r][c] == EMPTY) {
                    row = r;
                    col = c;
                    return true;
                }
            }
        }
        return false;
    }

    // True if value does not already appear in the row, column, or block of
    // (row, col). The cell at (row, col) itself is ignored.
    bool canPlace(int row, int col, int value) const {
        for (int i = 0; i < SIZE; ++i) {
            if (i != col && cells[row][i] == value) return false;
            if (i != row && cells[i][col] == value) return false;
        }
        int blockRow = row - row % BLOCK;
        int blockCol = col - col % BLOCK;
        for (int r = blockRow; r < blockRow + BLOCK; ++r)
            for (int c = blockCol; c < blockCol + BLOCK; ++c)
                if ((r != row || c != col) && cells[r][c] == value)
                    return false;
        return true;
    }

    // The digits that may legally go into the cell at (row, col).
    std::vector<int> candidates(int row, int col) const {
        std::vector<int> result;
        for (int value = 1; value <= SIZE; ++value)
            if (canPlace(row, col, value))
                result.push_back(value);
        return result;
    }

    // True if none of the cells already filled in conflict with each other.
    bool isConsistent() const {
        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c)
                if (cells[r][c] != EMPTY && !canPlace(r, c, cells[r][c]))
                    return false;
        return true;
    }

    void print(std::ostream& out) const {
        for (int r = 0; r < SIZE; ++r) {
            if (r != 0 && r % BLOCK == 0)
                out << "------+-------+------\n";
            for (int c = 0; c < SIZE; ++c) {
                if (c != 0) out << ' ';
                if (c != 0 && c % BLOCK == 0) out << "| ";
                if (cells[r][c] == EMPTY)
                    out << '_';
                else
                    out << cells[r][c];
            }
            out << '\n';
        }
    }

private:
    int cells[SIZE][SIZE];
};

// Solves a SudokuBoard by recursive depth-first search with backtracking,
// collecting every solution rather than stopping at the first one.
class SudokuSolver {
public:
    explicit SudokuSolver(const SudokuBoard& puzzle) : board(puzzle) {}

    // Runs the search and returns the number of solutions found.
    std::size_t solve() {
        solutions.clear();
        // Clues that already break the rules can never lead to a solution.
        if (board.isConsistent())
            search();
        return solutions.size();
    }

    const std::vector<SudokuBoard>& getSolutions() const { return solutions; }

private:
    // Fills one empty cell and recurses on the rest. When no empty cell
    // remains (k == 81) the board is a solution. Returning from this call
    // with the cell cleared is the backtrack step.
    void search() {
        int row, col;
        if (!board.findEmptyCell(row, col)) {
            solutions.push_back(board);
            return;
        }

        // An empty candidate list means an earlier guess was wrong; the loop
        // body never runs and we fall straight back to the caller.
        std::vector<int> options = board.candidates(row, col);
        for (std::size_t i = 0; i < options.size(); ++i) {
            board.set(row, col, options[i]);
            search();
            board.clear(row, col);
        }
    }

    SudokuBoard board;
    std::vector<SudokuBoard> solutions;
};

// Drives the program: loads each puzzle file, solves it, and reports the
// file name, the original puzzle, and every solution.
class PuzzleRunner {
public:
    PuzzleRunner(std::ostream& output) : out(output) {}

    void addFile(const std::string& path) { files.push_back(path); }

    void addDefaultFiles(const std::string& directory) {
        for (int i = 1; i <= 5; ++i) {
            std::string name = "puzzle" + std::to_string(i) + ".txt";
            files.push_back(directory.empty() ? name : directory + "/" + name);
        }
    }

    void run() {
        for (std::size_t i = 0; i < files.size(); ++i) {
            if (i != 0) out << '\n';
            runOne(files[i]);
        }
    }

private:
    static std::string fileName(const std::string& path) {
        std::size_t slash = path.find_last_of("/\\");
        return slash == std::string::npos ? path : path.substr(slash + 1);
    }

    void runOne(const std::string& path) {
        out << "========================================\n";
        out << fileName(path) << '\n';
        out << "========================================\n";

        SudokuBoard puzzle;
        std::string error;
        if (!puzzle.loadFromFile(path, error)) {
            out << "Error reading " << path << ": " << error << '\n';
            return;
        }

        out << "Puzzle:\n";
        puzzle.print(out);
        out << '\n';

        SudokuSolver solver(puzzle);
        std::size_t count = solver.solve();
        if (count == 0) {
            out << "No solution found\n";
            return;
        }

        const std::vector<SudokuBoard>& solutions = solver.getSolutions();
        for (std::size_t i = 0; i < count; ++i) {
            if (i != 0) out << '\n';
            if (count == 1)
                out << "Solution:\n";
            else
                out << "Solution " << (i + 1) << " of " << count << ":\n";
            solutions[i].print(out);
        }
    }

    std::ostream& out;
    std::vector<std::string> files;
};

int main(int argc, char* argv[]) {
    PuzzleRunner runner(std::cout);

    if (argc == 1) {
        runner.addDefaultFiles("");
    } else if (argc == 2 && std::string(argv[1]).find(".txt") == std::string::npos) {
        runner.addDefaultFiles(argv[1]);
    } else {
        for (int i = 1; i < argc; ++i)
            runner.addFile(argv[i]);
    }

    runner.run();
    return 0;
}
