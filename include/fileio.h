// SPDX-License-Identifier: MIT License
/* fileio.h -- header of fileio functions
 *
 * Copyright (C) 2025 Wen-Xuan Zhang <serialcore@outlook.com>
 */

#ifndef FILEIO_H
#define FILEIO_H

/* returns the length of the file in chars */

long fileio_length(char *path);

/* returns the state of operation: 1 success or 0 error */

int fileio_read_text(char *path, char *text, long length);
int fileio_write_text(char *path, char *text);
int fileio_append_text(char *path, char *text);

int fileio_read_formate(char *path, char *format, int count, ...);
int fileio_write_formate(char *path, char *format, int count, ...);
int fileio_append_formate(char *path, char *format, int count, ...);

int fileio_read_data(char *path, void *data, int size, long length);
int fileio_write_data(char *path, void *data, int size, long length);
int fileio_append_data(char *path, void *data, int size, long length);

#endif
