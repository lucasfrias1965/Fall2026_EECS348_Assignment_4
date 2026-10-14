/*
	<@
	(KU/
	 "
human.cpp - by google gemini flash
modified slightly by lucas frias
 Program:       human.cpp
 Description:  Solves Sudoku problems using a naive approach 
 Input:         N/A (assumes local files) 
 Output:        All puzzles.txt and their solutions (if possible)
 Collaborators:  Google Gemini Flash, Lucas Frias
 * Author:        Lucas Frias (by using Gemini Flash)
 * Created:       October 13th 2026 
 * Revised:        2026-10-13, implemented fix features 
 *                category to enum class Category, per Opus 5.5 pairing
 * Build:         g++ human.cpp -std=c++11 -Wall -Wextra -o ./human
 * Run:           ./human (ensure you have puzzles1.txt, puzzles2.txt, all the way to five
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class SudokuSolver {
//sudoku solver, our main class to solve sudoku

private:
    //make a private class with a size, blcok_size
    static const int SIZE = 9;
    static const int BLOCK_SIZE = 3;
    //make a grid with our set size (always a square)
    int grid[SIZE][SIZE];
    //make a filename
    std::string filename;
    //make a counter for the solutions found
    //lucas edit, the solutionsFound should be zero
    //initialised when declared
    int solutionsFound = 0;

    // Checks if placing 'num' at grid[row][col] is valid

    bool isValid(int row, int col, int num) const {
	//changes - added suffix increment
	
        for (int i = 0; i < SIZE; i++) {
            // Check row and column
		//here we check whether or not the number reappears
            if (grid[row][i] == num || grid[i][col] == num) {
                return false;//if it does return false
            }
        }

        // Check 3x3 block
	
	//changes - use human variable names row and col 
	//instead of r and c, and use correct suffix incrementing

        int startRow = (row / BLOCK_SIZE) * BLOCK_SIZE;
        int startCol = (col / BLOCK_SIZE) * BLOCK_SIZE;
        for (int row = 0; row < BLOCK_SIZE; row++) {
            for (int col = 0; col < BLOCK_SIZE; col++) {
                if (grid[startRow + row][startCol + col] == num) {
		    //we're just checking for the same consistency
		    //in just the whole block again
		    //very explicitly
                    return false;
                }
            }
        }
	//if either of these are flase, return true
        return true;
    }

    // Recursive helper to find all solutions using depth-first search with backtracking
    void solveRecursive(int row, int col) {
	// gets the recursive solution
        // If we reached past the last row, a complete solution is found
        if (row == SIZE) {//we went through every single row in the 0....SIZE-1 range, so this means every row is consistent, therefore there is a solution
            solutionsFound++;
            std::cout << "\nSolution " << solutionsFound << ":\n";
            printGrid();
            return;
        }

        // Calculate next cell position
	//calculate with a ternary

	//get the next row and ocl
        int nextRow = (col == SIZE - 1) ? row + 1 : row;
        int nextCol = (col == SIZE - 1) ? 0 : col + 1;

        // If cell is already filled, move to next cell

	//if the grid's current element exist, then
        if (grid[row][col] != 0) {
	    //solve the next one
            solveRecursive(nextRow, nextCol);
            return;
        }
	//see canidates 1-9 on our current element
        for (int num = 1; num <= SIZE; num++) {
            if (isValid(row, col, num)) {
                grid[row][col] = num;
                solveRecursive(nextRow, nextCol);
                grid[row][col] = 0; // Backtrack
            }
        }
    }

public:
	//initialiser method, takes a sting and requires a filename
	//calculates the solutionsFound
    SudokuSolver(const std::string& file) : filename(file), solutionsFound(0) {
        for (int row = 0; row < SIZE; row++) {
            for (int col = 0; col < SIZE; col++) {
		//zero intiialise all data
                grid[row][col] = 0;
            }
        }
    }

    // Reads the grid from the file
    bool loadPuzzle() {
	//make a file stream
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }
	//change -> intialised unitiliased zero	
	//explicitly zero initialising this is a good idea
	//so i did it
        char cell = 0;
		
        for (int row = 0; row < SIZE; row++) {
            for (int col= 0; col < SIZE; col++) {
		//copies file data into cell char by char
                if (!(file >> cell)) {
			//means we can't pull any new chars which means we're short in characters
                    std::cerr << "Error: Invalid or incomplete puzzle format in " << filename << std::endl;
                    return false;
                }
		//here we check to see the data return type
		//in this case, we're seeing if it's empty
		//or some char 1-9 or a bad char
                if (cell == '_') { 
                    grid[row][col] = 0;
                } else if (cell >= '1' && cell <= '9') {
                    grid[row][col] = cell - '0';
                } else {
                    std::cerr << "Error: Invalid character '" << cell << "' in " << filename << std::endl;
                    return false;
                }
            }
        }
	//load puzzle returns a boolean, which is kinda weird but not 
	//too bad of a choice. it makes sense to do the init method
        return true;
    }

    // Prints the grid
    void printGrid() const {
	//for all the rows, columns, display either
	//the empty _ char to represent a zero possibility
	//or displsy a number 1-9 to output what the value
	//ought to be when displaying
        for (int row= 0; row < SIZE; row++) {
            for (int col = 0; col < SIZE; col++) {
                if (grid[row][col] == 0) {
                    std::cout << "_ ";
                } else {
                    std::cout << grid[row][col] << " ";
                }
            }
            std::cout << "\n";
        }
    }

    // Primary object method to run solving routine and output formatted results
    void solve() {
	//display the filename and the puzzle
        std::cout << "========================================\n";
        std::cout << "File: " << filename << "\n";
        std::cout << "----------------------------------------\n";
        std::cout << "Initial Puzzle:\n";
        printGrid();
	//here we print all solutions by just recursively
	//going and display stuff. solutionsFound is a public
	//var so we have space for it.
        solutionsFound = 0;
        solveRecursive(0, 0);
	

	//display solutions found as a either a no solutions
	//or as a count, actual printing is handled later
        if (solutionsFound == 0) {
            std::cout << "\nNo solution found\n";
        } else {
            std::cout << "\nSolutions found: " << solutionsFound << "\n";
        }
        std::cout << "========================================\n\n";
    }
};

int main() {
   ///get all the puzzles
    std::vector<std::string> puzzleFiles = {
        "puzzle1.txt",
        "puzzle2.txt",
        "puzzle3.txt",
        "puzzle4.txt",
        "puzzle5.txt"
    };
    //use dynamic iteration if we wanna add more puzzles
    for (const auto& filename : puzzleFiles) {
	//make a sudoku instance
        SudokuSolver solver(filename);
	//can we load it?
        if (solver.loadPuzzle()) {
            solver.solve();//then solve it, which prints everything
        }
    }
    //return the POSIX return code
    return 0;
}
