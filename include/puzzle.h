// SPDX-License-Identifier: MIT License
/* puzzle.h -- header of sudoku make and io methods
 *
 * Copyright (C) 2025 Wen-Xuan Zhang <serialcore@outlook.com>
 */

#ifndef PUZZLE_H
#define PUZZLE_H
 
typedef struct puzzle {
   int order; /* order N can be 2, 3, 4, ...， 10 */
   int scale; /* scale of number can be 2^2=4, 3^2=9, 4^2=16, ...， 10^2=100 */
   int size; /* size of puzzle can be 2^4=16, 3^4=81, 4^4=256, ..., 10^4=10000 */
   int *map;
}puzzle_t;

/* make the world's hardest sudoku */
puzzle_t *puzzle_make_default();

/* make puzzle map by swapping rows and cols */
puzzle_t *puzzle_make_swap(int order);

/* read puzzle map from data file */
puzzle_t *puzzle_read_data(char *path);

/* write puzzle map to data file */
void puzzle_write_data(char *path, puzzle_t *puzzle);

/* print puzzle map to the console */
void puzzle_print_console(puzzle_t *puzzle);
 
#endif
 