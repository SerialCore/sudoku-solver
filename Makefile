all:
	gcc src/main.c src/solver.c src/puzzle.c src/fileio.c -I include/ -lm -o sudoku_solver

clean:
	rm sudoku_solver
