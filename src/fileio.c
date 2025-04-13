// SPDX-License-Identifier: MIT License
/* fileio.c -- file read and write functions
 * supports read, write and append operations for plain text, formatetd text and vector data respectively
 *
 * Copyright (C) 2025 Wen-Xuan Zhang <serialcore@outlook.com>
 */

#include <fileio.h>

#include <stdio.h>
#include <stdarg.h>

long fileio_length(char *path)
{
    FILE *pf;
    long length = 0;

    pf = fopen(path, "r");
    while (fgetc(pf) != EOF) {
        length++;
    }

    return length;
}

int fileio_read_text(char *path, char *text, long length)
{
    FILE *pf;
    int state = 0; 

    pf = fopen(path, "r");
    if (pf != NULL) {
        state = fgets(text, length, pf) != NULL ? 1 : 0;
        state *= fclose(pf) + 1;
    }

    return state;
}

int fileio_write_text(char *path, char *text)
{
    FILE *pf;
    int state = 0; 

    pf = fopen(path, "w");
    if (pf != NULL) {
        state = fputs(text, pf);
        state *= fclose(pf) + 1;
    }

    return state;
}

int fileio_append_text(char *path, char *text)
{
    FILE *pf;
    int state = 0; 

    pf = fopen(path, "a");
    if (pf != NULL) {
        state = fputs(text, pf);
        state *= fclose(pf) + 1;
    }

    return state;
}

int fileio_read_formate(char *path, char *format, int count, ...)
{
    FILE *pf;
    int state = 0; 
    va_list args;

    va_start(args, count);
    pf = fopen(path, "r");
    if (pf != NULL) {
        state = fscanf(pf, format, args) == count ? 1 : 0;
        state *= fclose(pf) + 1;
    }
    va_end(args);

    return state;
}

int fileio_write_formate(char *path, char *format, int count, ...)
{
    FILE *pf;
    int state = 0; 
    va_list args;

    va_start(args, count);
    pf = fopen(path, "w");
    if (pf != NULL) {
        state = fprintf(pf, format, args) == count ? 1 : 0;
        state *= fclose(pf) + 1;
    }
    va_end(args);

    return state;
}

int fileio_append_formate(char *path, char *format, int count, ...)
{
    FILE *pf;
    int state = 0; 
    va_list args;

    va_start(args, count);
    pf = fopen(path, "a");
    if (pf != NULL) {
        state = fprintf(pf, format, args) == count ? 1 : 0;
        state *= fclose(pf) + 1;
    }
    va_end(args);

    return state;
}

int fileio_read_data(char *path, void *data, int size, long length)
{
    FILE *pf;
    int state = 0; 

    pf = fopen(path, "rb");
    if (pf != NULL) {
        state = fread(data, size, length, pf) == length ? 1 : 0;
        state *= fclose(pf) + 1;
    }

    return state;
}

int fileio_write_data(char *path, void *data, int size, long length)
{
    FILE *pf;
    int state = 0; 

    pf = fopen(path, "wb");
    if (pf != NULL) {
        state = fwrite(data, size, length, pf) == length ? 1 : 0;
        state *= fclose(pf) + 1;
    }

    return state;
}

int fileio_append_data(char *path, void *data, int size, long length)
{
    FILE *pf;
    int state = 0; 

    pf = fopen(path, "ab");
    if (pf != NULL) {
        state = fwrite(data, size, length, pf) == length ? 1 : 0;
        state *= fclose(pf) + 1;
    }

    return state;
}
