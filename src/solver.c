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

typedef struct guess {
    int back; /* the back point in fills */
    int choice; /* the choice in note */
    int count; /* the count of noted number */
    int *nums; /* the exact note */
}guess_t;

typedef struct state {
    puzzle_t *puzzle; /* puzzle point */
    int totalvoid; /* the total amount of voids */
    int totalfill; /* the total amount of filled voids */
    int deadend; /* if no where to go for now */
    int guessed; /* times of guess attempts for now */
    int error; /* if there is an error after guessing */
}state_t;

/* stage 1: scan every void in puzzle map and check what can put in it */
void update_note_void(note_t *notes, state_t *states);

/* stage 2: scan every number in puzzle scale and check where can put it in */
void update_note_number(note_t *notes, state_t *states);

/* fill the logically available numbers; return filled */
void solver_fill(note_t *notes, fill_t *fills, state_t *states);

/* guess a number and proceed */
void solver_guess(note_t *notes, fill_t *fills, guess_t *guesses, state_t *states);

/* wrong guess, drawback */
void solver_drawback(fill_t *fills, guess_t *guesses, state_t *states);

/* main procedure of solving method */
void solver_main(puzzle_t *puzzle)
{
    int puzzle_scale = puzzle->scale;
    int puzzle_size = puzzle->size;
    int *puzzle_map = puzzle->map;
    puzzle_print_console(puzzle);

    /* initialize state information */
    state_t *states = malloc(sizeof(state_t));
    states->puzzle = puzzle;
    states->totalvoid = 0;
    states->totalfill = 0;
    states->deadend = 0;
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
    fill_t *fills = malloc(sizeof(fill_t)*states->totalvoid);
    guess_t *guesses = malloc(sizeof(guess_t)*states->totalvoid);
    printf("[okey] get %d voids to fill\n\n", states->totalvoid);

    /* run the solver */
    while (states->totalfill < states->totalvoid) {
        /* stage 1: update note */
        update_note_void(notes, states);
        if (states->error) {
            /* wrong guess, drawback */
            solver_drawback(fills, guesses, states);
            /* another guess */
            solver_guess(notes, fills, guesses, states);
            continue;
        }
        /* stage 2: update note more precisely */
        update_note_number(notes, states);
        /* fill in numbers avialable */
        solver_fill(notes, fills, states);
        if (states->deadend) {
            /* dead end, guess a number */
            solver_guess(notes, fills, guesses, states);
        }
        puzzle_print_console(puzzle);
    }
    printf("[okey] sudoku solved!\n\n");
    
    /* print fill history (no wrong guesses) */
    printf("[log] solving history:\n");
    for (int h = 0; h < states->totalfill; h++) {
        printf("[log] fill void {%d, %d} <- %d\n", fills[h].row, fills[h].col, fills[h].num);
    }

    /* free memory buffer */
    for (int i = 0; i < puzzle_size; i++) {
        if (notes[i].nums != NULL) {
            free(notes[i].nums);
        }
    }
    free(notes);
    for (int i = 0; i < states->totalvoid; i++) {
        if (guesses[i].nums != NULL) {
            free(guesses[i].nums);
        }
    }
    free(guesses);
    free(fills);
    free(states);
}

void update_note_void(note_t *notes, state_t *states)
{
    /* situations:
     * 1. basically, every note contains as least one number
     * 2. unfortunately, any note contains no number which implies error
     */

    int puzzle_order = states->puzzle->order;
    int puzzle_scale = states->puzzle->scale;
    int *puzzle_map = states->puzzle->map;

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

void update_note_number(note_t *notes, state_t *states)
{
    /* situations:
     * 1. any number only exists in one note in one chunk which means that is the answer
     * 2. any number only exists in one note in one row which means that is the answer
     * 2. any number only exists in one note in one col which means that is the answer
     */

    int puzzle_order = states->puzzle->order;
    int puzzle_scale = states->puzzle->scale;
    int *puzzle_map = states->puzzle->map;

    int count; /* count of "where" */
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

void solver_fill(note_t *notes, fill_t *fills, state_t *states)
{
    /* situations:
     * 1. any note contains one number which means that is the answer
     * 2. every note constains as least two numbers, that is called deadend
     */

    int puzzle_scale = states->puzzle->scale;
    int *puzzle_map = states->puzzle->map;

    int filled = 0;
    note_t onenote;

    for (int noterow = 0; noterow < puzzle_scale; noterow++) {
        for (int notecol = 0; notecol < puzzle_scale; notecol++) {
            /* get a note with row and col */
            onenote = notes[puzzle_scale*noterow+notecol];
            if (onenote.count == -1) {
                continue;
            }
            if (onenote.count == 1) {
                puzzle_map[puzzle_scale*noterow+notecol] = onenote.nums[0];
                fill_t newfill = {
                    .row = noterow,
                    .col = notecol,
                    .num = onenote.nums[0]
                };
                filled++;
                fills[states->totalfill++] = newfill;
                printf("[log] fill void {%d, %d} <- %d\n", noterow, notecol, onenote.nums[0]);
            }
        }
    }
    
    if (filled != 0) {
        states->deadend = 0;
        printf("[log] %d voids filled for now\n\n", states->totalfill);
    }
    else {
        states->deadend = 1;
        printf("[log] dead end encountered\n\n");
    }
}

void solver_guess(note_t *notes, fill_t *fills, guess_t *guesses, state_t *states)
{
    /* situations:
     * 1. first deadend, first guess
     * 2. another deadend encountered, need a new guess
     * 3. drawback and choose next number for the last guess postion
     * 4. not the problem of last guess, rollback (already handled by solver_drawback)
     */

    int puzzle_scale = states->puzzle->scale;
    int *puzzle_map = states->puzzle->map;

    //int virgin; /* first or another */
    int grow = 0, gcol = 0; /* at the start of map as default */
    note_t onenote;
    guess_t oneguess;

    /* not the first guess, move on to next number or new guess */
    if (states->guessed != 0) {
        oneguess = guesses[states->guessed-1];
        grow = fills[oneguess.back].row;
        gcol = fills[oneguess.back].col;
        /* this is called by drawback/error not deadend */
        if (!states->deadend) {
            /* if choice hits the limit, the guess should have been wasted already */
            oneguess.choice++;
            guesses[states->guessed-1] = oneguess;
            fill_t newfill = {
                .row = grow,
                .col = gcol,
                .num = oneguess.nums[oneguess.choice]
            };
            fills[states->totalfill++] = newfill;
            puzzle_map[puzzle_scale*grow+gcol] = newfill.num;
            printf("[log] guess number %d -> {%d, %d}\n\n", newfill.num, grow, gcol);
        }
        /* another deadend, new guess */
        else {
            //virgin = 0;
        }
    }
    /* first deadend, first guess */
    else {
        //virgin = 1;
    }

    /* make a new guess and record */
    for (int noterow = grow; noterow < puzzle_scale; noterow++) {
        for (int notecol = gcol; notecol < puzzle_scale; notecol++) {
            onenote = notes[puzzle_scale*noterow+notecol];
            if (onenote.count == -1) {
                continue;
            }
            /* copy the note which could be lost later */
            oneguess.back = states->totalfill;
            oneguess.choice = 0; /* first guess */
            oneguess.count = onenote.count;
            oneguess.nums = malloc(sizeof(int)*onenote.count);
            for (int c = 0; c < onenote.count; c++) {
                oneguess.nums[c] = onenote.nums[c];
            }
            guesses[states->guessed++] = oneguess;
            /* fill the guess in map */
            states->deadend = 0;
            fill_t newfill = {
                .row = noterow,
                .col = notecol,
                .num = oneguess.nums[oneguess.choice]
            };
            fills[states->totalfill++] = newfill;
            puzzle_map[puzzle_scale*noterow+notecol] = newfill.num;
            printf("[log] guess number %d -> {%d, %d}\n\n", newfill.num, noterow, notecol);
            return;
        }
    }
}

void solver_drawback(fill_t *fills, guess_t *guesses, state_t *states)
{
    /* situations:
     * 1. the number of last guess was wrong
     * 2. the number of earlier guess was wrong
     */

    int puzzle_scale = states->puzzle->scale;
    int *puzzle_map = states->puzzle->map;

    int grow, gcol;
    guess_t oneguess;

    oneguess = guesses[states->guessed-1];
    /* not the problem of last guess, it's guessed up already */
    if (oneguess.choice == oneguess.count - 1) {
        /* throw last guess to the garbage */
        states->guessed--;
        oneguess = guesses[states->guessed-1];
    }
    grow = fills[oneguess.back].row;
    gcol = fills[oneguess.back].col;

    /* withdraw from guess backpoint to the last filled */
    for (int i = oneguess.back; i < states->totalfill; i++) {
        /* only withdraw filled number, no need to empty history */
        puzzle_map[puzzle_scale*(fills[i].row)+(fills[i].col)] = 0;
    }
    states->totalfill = oneguess.back;
    states->error = 0;

    printf("[log] withdraw guess %d -> {%d, %d} and later\n\n", oneguess.nums[oneguess.choice], grow, gcol);
}
