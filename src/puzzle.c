// SPDX-License-Identifier: MIT License
/* puzzle.c -- sudoku make and io methods
 * makes N^2-order sudoku puzzle where N can be 2, 3, 4, ..., 9
 * N can be larger than 9, but the program doesn't guarantee the performance.
 *
 * Copyright (C) 2025 Wen-Xuan Zhang <serialcore@outlook.com>
 */

#include <puzzle.h>
#include <fileio.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

puzzle_t *puzzle_make_swap(int order)
{
    if (order < 2 && order > 9) {
        return NULL;
    }

    int puzzle_order = order;
    int puzzle_scale = pow(order, 2);
    int puzzle_size = pow(order, 4);

    /* initialize puzzle map */
    int *puzzle_map = malloc(sizeof(int)*puzzle_size);
    /* scan rows */
    for (int i = 0; i < puzzle_scale; i++) {
        /* scan cols */
        for (int j = 0; j < puzzle_scale; j++) {
            puzzle_map[puzzle_scale*i+j] = (puzzle_order * i + i / puzzle_order + j) % puzzle_scale + 1;
        }
    }

    puzzle_t *puzzle = malloc(sizeof(puzzle_t));
    puzzle->order = puzzle_order;
    puzzle->scale = puzzle_scale;
    puzzle->size = puzzle_size;
    puzzle->map = puzzle_map;
    puzzle_print_console(puzzle);
    printf("[okey] standard form initialized for %dx%d sudoku\n\n", puzzle_scale, puzzle_scale);

    srand(time(0));
    int an, am, found;
    int *used_rand = malloc(sizeof(int)*floor(puzzle_order/2)*2);
    int *temp_arrow = malloc(sizeof(int)*puzzle_scale);
    /* scan row chunks */
    for (int r = 0; r < puzzle_order; r++) {
        /* reset recording */
        for (int u = 0; u < floor(puzzle_order/2)*2; u++) {
            used_rand[u] = 0;
        }
        /* swap rows for order/2 times */
        for (int i = 0; i < floor(puzzle_order/2); i++) {
            /* roll the dice for row n */
            found = 1;
            while (found) {
                an = rand() % puzzle_order + puzzle_order * r + 1;
                for (int u = 0; u < floor(puzzle_order/2)*2; u++) {
                    if (used_rand[u] == an) {
                        found = 1;
                        break;
                    }
                    else if (used_rand[u] == 0) {
                        used_rand[u] = an;
                        found = 0;
                        break;
                    }
                }
            }
            an--;
            /* roll the dice for row m */
            found = 1;
            while (found) {
                am = rand() % puzzle_order + puzzle_order * r + 1;
                for (int u = 0; u < floor(puzzle_order/2)*2; u++) {
                    if (used_rand[u] == am) {
                        found = 1;
                        break;
                    }
                    else if (used_rand[u] == 0) {
                        used_rand[u] = am;
                        found = 0;
                    break;
                    }
                }
            }
            am--;
            /* swap the rows n and m */
            for (int j = 0; j < puzzle_scale; j++) {
                temp_arrow[j] = puzzle_map[puzzle_scale*an+j];
                puzzle_map[puzzle_scale*an+j] = puzzle_map[puzzle_scale*am+j];
                puzzle_map[puzzle_scale*am+j] = temp_arrow[j];
            }
            printf("[log] swap rows %d and %d\n", an, am);
        }
    }
    
    /* scan col chunks */
    for (int c = 0; c < puzzle_order; c++) {
        /* reset recording */
        for (int u = 0; u < floor(puzzle_order/2)*2; u++) {
            
            used_rand[u] = 0;
        }
        /* swap cols for order/2 times */
        for (int j = 0; j < floor(puzzle_order/2); j++) {
            /* roll the dice for col n */
            found = 1;
            while (found) {
                an = rand() % puzzle_order + puzzle_order * c + 1;
                for (int u = 0; u < floor(puzzle_order/2)*2; u++) {
                    if (used_rand[u] == an) {
                        found = 1;
                        break;
                    }
                    else if (used_rand[u] == 0) {
                        used_rand[u] = an;
                        found = 0;
                        break;
                    }
                }
            }
            an--;
            /* roll the dice for col m */
            found = 1;
            while (found) {
                am = rand() % puzzle_order + puzzle_order * c + 1;
                for (int u = 0; u < floor(puzzle_order/2)*2; u++) {
                    if (used_rand[u] == am) {
                        found = 1;
                        break;
                    }
                    else if (used_rand[u] == 0) {
                        used_rand[u] = am;
                        found = 0;
                        break;
                    }
                }
            }
            am--;
            /* swap the cols n and m */
            for (int i = 0; i < puzzle_scale; i++) {
                temp_arrow[i] = puzzle_map[puzzle_scale*i+an];
                puzzle_map[puzzle_scale*i+an] = puzzle_map[puzzle_scale*i+am];
                puzzle_map[puzzle_scale*i+am] = temp_arrow[i];
            }
            printf("[log] swap cols %d and %d\n", an, am);
        }
    }
    printf("\n");
    puzzle_print_console(puzzle);

    free(used_rand);
    free(temp_arrow);

    /* trim puzzle map */
    int randr, randc;
    int trimcount = rand() % (puzzle_size / 5) + 4 * puzzle_size / 5;
    for (int i = 0; i < trimcount; i++) {
        randr = rand() % puzzle_scale;
        randc = rand() % puzzle_scale;
        puzzle_map[puzzle_scale*randr+randc] = 0;
        printf("[log] trim location {%d, %d}\n", randr, randc);
    }
    printf("\n");
    puzzle_print_console(puzzle);

    return puzzle;
}

puzzle_t *puzzle_read_data(char *path)
{
    int size = fileio_length(path) * sizeof(char) / sizeof(int);
    int puzzle_order = sqrt(sqrt(size));
    int puzzle_scale = sqrt(size);
    int puzzle_size = size;

    int *puzzle_map = malloc(sizeof(int)*puzzle_size);
    if (!fileio_read_data(path, puzzle_map, sizeof(int), puzzle_size)) {
        printf("[error] failed to read %s\n", path);
        return NULL;
    }

    puzzle_t *puzzle = malloc(sizeof(puzzle_t));
    puzzle->order = puzzle_order;
    puzzle->scale = puzzle_scale;
    puzzle->size = puzzle_size;
    puzzle->map = puzzle_map;

    printf("[okey] read %s successfully\n", path);
    return puzzle;
}

void puzzle_write_data(char *path, puzzle_t *puzzle)
{
    if (fileio_write_data(path, puzzle->map, sizeof(int), puzzle->size)) {
        printf("[okey] write %s successfully\n", path);
    }
    else {
        printf("[error] failed to write %s\n", path);
    }
}

void puzzle_print_console(puzzle_t *puzzle)
{
    int num;
    int puzzle_order = puzzle->order;
    int puzzle_scale = puzzle->scale;
    int *puzzle_map = puzzle->map;

    /* draw border */
    for (int c = 0; c < (3 * puzzle_scale + 1); c++) {
        printf("-");
    }
    printf("\n");

    /* scan rows */
    for (int i = 0; i < puzzle_scale; i++) {
        printf("|");
        /* scan cols */
        for (int j = 0; j < puzzle_scale; j++) {
            num = puzzle_map[puzzle_scale*i+j];
            /* draw border */
            if (((j + 1) % puzzle_order) == 0) {
                num != 0 ? printf("%2d", num) : printf("  ");
                printf("|");
            }
            else {
                num != 0 ? printf("%2d ", num) : printf("   ");
            }
        }
        printf("\n");
        /* draw border */
        if (((i + 1) % puzzle_order) == 0) {
            for (int c = 0; c < (3 * puzzle_scale + 1); c++) {
                printf("-");
            }
            printf("\n");
        }
    }
}
