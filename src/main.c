// SPDX-License-Identifier: MIT License
/* main.c -- main function and control system
 *
 * Copyright (C) 2025 Wen-Xuan Zhang <serialcore@outlook.com>
 */

#include <puzzle.h>
#include <solver.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_help();

int main(int argc, char **argv)
{
    if (argc == 2) {
        if (!strcmp(argv[1], "help")) {
            print_help();
        }
        else {
            return 1;
        }
    }
    else if (argc == 3) {
        if (!strcmp(argv[1], "solve")) {
            puzzle_t *puzzle = puzzle_read_data(argv[2]);
            if (puzzle != NULL) {
                solver_main(puzzle);
            }
        }
        else {
            return 1;
        }
    }
    else if (argc == 4) {
        if (!strcmp(argv[1], "make")) {
            puzzle_t *puzzle = puzzle_make_swap(atoi(argv[3]));
            if (puzzle != NULL) {
                puzzle_write_data(argv[2], puzzle);
            }
        }
        else {
            return 1;
        }
    }
    else {
        print_help();
    }

    return 0;
}

void print_help()
{
    printf("This is the help page, for now.\n");
    printf("usage: ./sudoku_solver [operate] [datafile] [order] > logfile\n\n");
    printf("operate: \n");
    printf("    make\tmake a new puzzle and write to file.\n");
    printf("    solve\tread a puzzle and solve it.\n");
    printf("    help\tshow this page.\n\n");
    printf("order N: \n");
    printf("    N can be 2, 3, 4, ..., 9\n\n");
    printf("example: \n");
    printf("    ./sudoku_solver make puzzle.dat 3 > make.log\n");
    printf("    ./sudoku_solver solve puzzle.dat > solve.log\n");
}