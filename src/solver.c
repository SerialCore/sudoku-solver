// SPDX-License-Identifier: MIT License
/* solver.c -- various methods for solving sudoku
 *
 * Copyright (C) 2025 Wen-Xuan Zhang <serialcore@outlook.com>
 */

#include <solver.h>
#include <puzzle.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct note {
    int count; /* the count of numbers */
    int *nums; /* the arrow of numbers */
}note_t;

typedef struct fill {
    int row; /* the row of location */
    int col; /* the col of location */
    int num; /* the number to fill */
}fill_t;

typedef struct state {
    int totalvoid; /* the total amount of voids */
    int totalfill; /* the total amount of filled voids */
    int oncefill; /* amount of filled voids for one run */
    int backpoint; /* the point of drawback/fork */
    int guessed; /* the index of guessed number in note */
    int error; /* if there is an error after guessing */
}state_t;

/* stage 1: scan every void in puzzle map and check what can put in it */
void update_note_void(puzzle_t *puzzle, note_t *notes, state_t *states);

/* stage 2: scan every number in puzzle scale and check where can put it in */
void update_note_number(puzzle_t *puzzle, note_t *notes);

/* fill the logically available numbers; return filled */
void solver_fill(puzzle_t *puzzle, note_t *notes, fill_t *history, state_t *states);

/* guess a number and proceed */
void solver_guess(puzzle_t *puzzle, note_t *notes, fill_t *history, state_t *states);

/* wrong guess, drawback */
void solver_drawback(puzzle_t *puzzle, note_t *notes, fill_t *history, state_t *states);

/* main procedure of solving method */
void solver_main(puzzle_t *puzzle)
{
    int puzzle_scale = puzzle->scale;
    int puzzle_size = puzzle->size;
    int *puzzle_map = puzzle->map;
    puzzle_print_console(puzzle);

    /* initialize state information */
    state_t *states = malloc(sizeof(state_t));
    states->totalvoid = 0;
    states->totalfill = 0;
    states->oncefill = 0;
    states->backpoint = 0;
    states->guessed = 0;
    states->error = 0;

    /* initialize notepad */
    note_t *notes = malloc(sizeof(note_t)*puzzle_size);
    /* create a mirror map of notes */
    for (int i = 0; i < puzzle_scale; i++) {
        for (int j = 0; j < puzzle_scale; j++) {
            if (puzzle_map[puzzle_scale*i+j] == 0) {
                states->totalvoid++;
                note_t realnote = {
                    .count = 0,
                    .nums = malloc(sizeof(int)*puzzle_scale)
                };
                notes[puzzle_scale*i+j] = realnote;
            }
            else {
                note_t fakenote = {
                    .count = -1,
                    .nums = NULL
                };
                notes[puzzle_scale*i+j] = fakenote;
            }
        }
    }
    fill_t *history = malloc(sizeof(fill_t)*states->totalvoid);
    printf("[okey] get %d voids to fill\n\n", states->totalvoid);

    /* run the solver */
    while (states->totalfill < states->totalvoid) {
        /* stage 1: update note */
        update_note_void(puzzle, notes, states);
        if (states->error) {
            /* wrong guess, drawback */
            solver_drawback(puzzle, notes, history, states);
            /* another guess */
            solver_guess(puzzle, notes, history, states);
            continue;
        }
        else {
            /* no error, no need of another guess */
            states->guessed = 0;
        }
        /* stage 2: update note more precisely */
        update_note_number(puzzle, notes);
        /* fill in numbers avialable */
        solver_fill(puzzle, notes, history, states);
        if (states->oncefill == 0) {
            /* dead end, guess a number */
            solver_guess(puzzle, notes, history, states);
        }
        puzzle_print_console(puzzle);
    }
    printf("[okey] sudoku solved!\n");

    /* free memory buffer */
    for (int i = 0; i < puzzle_size; i++) {
        if (notes[i].nums != NULL) {
            free(notes[i].nums);
        }
    }
    free(notes);
    free(states);
    free(history);
}

void update_note_void(puzzle_t *puzzle, note_t *notes, state_t *states)
{
    int puzzle_order = puzzle->order;
    int puzzle_scale = puzzle->scale;
    int *puzzle_map = puzzle->map;

    int count, found;
    int csrow, cscol; /* start of a chunk */
    note_t onenote;

    /* stage 1: scan every void in puzzle map and check what can put in it */
    for (int noterow = 0; noterow < puzzle_scale; noterow++) {
        for (int notecol = 0; notecol < puzzle_scale; notecol++) {
            /* get a note with row and col */
            onenote = notes[puzzle_scale*noterow+notecol];
            if (onenote.count == -1) {
                continue;
            }
            if (puzzle_map[puzzle_scale*noterow+notecol] != 0) {
                onenote.count = -1;
                notes[puzzle_scale*noterow+notecol] = onenote;
                continue;
            }

            /* empty notes */
            for (int c = 0; c < onenote.count; c++) {
                onenote.nums[c] = 0;
            }
            count = 0; /* count of "what" for a void */
            /* scan every number */
            for (int n = 1; n <= puzzle_scale; n++) {
                found = 0;
                /* check whole row */
                for (int j = 0; j < puzzle_scale; j++) {
                    if (puzzle_map[puzzle_scale*noterow+j] == n) {
                        found = 1;
                        break;
                    }
                }
                /* check whole col */
                for (int i = 0; i < puzzle_scale; i++) {
                    if (puzzle_map[puzzle_scale*i+notecol] == n) {
                        found = 1;
                        break;
                    }
                }
                /* chech whole chunk */
                csrow = floor(noterow/puzzle_order) * puzzle_order;
                cscol = floor(notecol/puzzle_order) * puzzle_order;
                for (int i = csrow; i < csrow + puzzle_order; i++) {
                    for (int j = cscol; j < cscol + puzzle_order; j++) {
                        if (puzzle_map[puzzle_scale*i+j] == n) {
                            found = 1;
                            break;
                        }
                    }
                }
                if (!found) {
                    /* the number is available */
                    onenote.nums[count++] = n;
                    onenote.count = count;
                    notes[puzzle_scale*noterow+notecol] = onenote;
                }
            }
            /* if error encountered */
            if (count == 0) {
                states->error = 1;
                printf("[log] error encountered\n\n");
                return;
            }
            /* all is well */
            printf("[log] scan void {%d, %d} <- ", noterow, notecol);
            for (int c = 0; c < count; c++) {
                printf("%d, ", onenote.nums[c]);
            }
            printf("\n");
        }
    }
    printf("\n");
}

void update_note_number(puzzle_t *puzzle, note_t *notes)
{
    int puzzle_order = puzzle->order;
    int puzzle_scale = puzzle->scale;
    int *puzzle_map = puzzle->map;

    int count;
    int srow, scol; /* special location */
    int csrow, cscol; /* start of a chunk */
    note_t onenote;

    /* stage 2: scan every number in puzzle scale and check where can put it in */
    for (int n = 1; n <= puzzle_scale; n++) {
        /* check every chunk in notes */
        for (int chrow = 0; chrow < puzzle_order; chrow++) {
            for (int chcol = 0; chcol < puzzle_order; chcol++) {
                count = 0; /* count of "where" in a chunk for a number */
                /* check every note in this chunk */
                csrow = chrow * puzzle_order;
                cscol = chcol * puzzle_order;
                for (int noterow = csrow; noterow < csrow + puzzle_order; noterow++) {
                    for (int notecol = cscol; notecol < cscol + puzzle_order; notecol++) {
                        onenote = notes[puzzle_scale*noterow+notecol];
                        if (onenote.count == -1) {
                            continue;
                        }
                        for (int c = 0; c < onenote.count; c++) {
                            if (onenote.nums[c] == n) {
                                /* record special location */
                                srow = noterow;
                                scol = notecol;
                                count++;
                                break;
                            }
                        }
                    }
                }
                if (count == 1) {
                    /* update note */
                    onenote = notes[puzzle_scale*srow+scol];
                    for (int c = 0; c < onenote.count; c++) {
                        onenote.nums[c] = 0;
                    }
                    onenote.count = 1;
                    onenote.nums[0] = n;
                    notes[puzzle_scale*srow+scol] = onenote;
                    printf("[log] scan number %d -> {%d, %d}\n", n, srow, scol);
                }
            }
        }

        /* check every row in notes */
        for (int noterow = 0; noterow < puzzle_scale; noterow++) {
            count = 0; /* count of "where" in a row for a number */
            for (int notecol = 0; notecol < puzzle_scale; notecol++) {
                onenote = notes[puzzle_scale*noterow+notecol];
                if (onenote.count == -1) {
                    continue;
                }
                for (int c = 0; c < onenote.count; c++) {
                    if (onenote.nums[c] == n) {
                        /* record special location */
                        srow = noterow;
                        scol = notecol;
                        count++;
                        break;
                    }
                }
            }
            if (count == 1) {
                /* update note */
                onenote = notes[puzzle_scale*srow+scol];
                for (int c = 0; c < onenote.count; c++) {
                    onenote.nums[c] = 0;
                }
                onenote.count = 1;
                onenote.nums[0] = n;
                notes[puzzle_scale*srow+scol] = onenote;
                printf("[log] scan number %d -> {%d, %d}\n", n, srow, scol);
            }
        }

        /* check every col in notes */
        for (int notecol = 0; notecol < puzzle_scale; notecol++) {
            count = 0; /* count of "where" in a col for a number */
            for (int noterow = 0; noterow < puzzle_scale; noterow++) {
                onenote = notes[puzzle_scale*noterow+notecol];
                if (onenote.count == -1) {
                    continue;
                }
                for (int c = 0; c < onenote.count; c++) {
                    if (onenote.nums[c] == n) {
                        /* record special location */
                        srow = noterow;
                        scol = notecol;
                        count++;
                        break;
                    }
                }
            }
            if (count == 1) {
                /* update note */
                onenote = notes[puzzle_scale*srow+scol];
                for (int c = 0; c < onenote.count; c++) {
                    onenote.nums[c] = 0;
                }
                onenote.count = 1;
                onenote.nums[0] = n;
                notes[puzzle_scale*srow+scol] = onenote;
                printf("[log] scan number %d -> {%d, %d}\n", n, srow, scol);
            }
        }
    }
    printf("\n");
}

void solver_fill(puzzle_t *puzzle, note_t *notes, fill_t *history, state_t *states)
{
    int puzzle_scale = puzzle->scale;
    int *puzzle_map = puzzle->map;

    int number;
    states->oncefill = 0;
    note_t onenote;

    for (int noterow = 0; noterow < puzzle_scale; noterow++) {
        for (int notecol = 0; notecol < puzzle_scale; notecol++) {
            /* get a note with row and col */
            onenote = notes[puzzle_scale*noterow+notecol];
            if (onenote.count == -1) {
                continue;
            }

            if (onenote.count == 1) {
                number = onenote.nums[0];
                puzzle_map[puzzle_scale*noterow+notecol] = number;
    
                fill_t newfill = {
                    .row = noterow,
                    .col = notecol,
                    .num = number
                };
                states->oncefill++;
                history[states->totalfill++] = newfill;
                printf("[log] fill void {%d, %d} <- %d\n", noterow, notecol, number);
            }
        }
    }
    
    if (states->oncefill != 0) {
        printf("[log] %d voids filled for now\n\n", states->totalfill);
    }
    else {
        printf("[log] dead end encountered\n\n");
    }
}

void solver_guess(puzzle_t *puzzle, note_t *notes, fill_t *history, state_t *states)
{
    int puzzle_scale = puzzle->scale;
    int *puzzle_map = puzzle->map;

    int choice = states->guessed; /* choose a number in note */
    states->backpoint = states->totalfill;
    note_t onenote;

    for (int noterow = 0; noterow < puzzle_scale; noterow++) {
        for (int notecol = 0; notecol < puzzle_scale; notecol++) {
            onenote = notes[puzzle_scale*noterow+notecol];
            if (onenote.count == -1) {
                continue;
            }

            fill_t guess = {
                .row = noterow,
                .col = notecol,
                .num = onenote.nums[choice]
            };
            history[states->totalfill++] = guess;
            puzzle_map[puzzle_scale*noterow+notecol] = guess.num;
            printf("[log] guess number %d -> {%d, %d}\n\n", guess.num, noterow, notecol);
            return;
        }
    }

}

void solver_drawback(puzzle_t *puzzle, note_t *notes, fill_t *history, state_t *states)
{
    int puzzle_scale = puzzle->scale;
    int *puzzle_map = puzzle->map;

    /* only withdraw filled number, no need to empty history */
    for (int i = states->backpoint; i < states->totalfill; i++) {
        puzzle_map[puzzle_scale*(history[i].row)+(history[i].col)] = 0;
    }
    states->totalfill = states->backpoint;
    states->error = 0;
    states->guessed++;

    fill_t guess = history[states->backpoint];
    printf("[log] withdraw number %d <- {%d, %d}\n\n", guess.num, guess.row, guess.col);
}