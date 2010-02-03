/*
 * Declarations and definitions for line-buffered queued I/O module.
 *
 * Copyright (c) 2004-2005, Steven Grimm.
 */
#ifndef _QLIO_H_
#define _QLIO_H_ 1

#include "include.h"

/*
 * When a line has been read from a monitored socket, a callback function for
 * that socket is called.
 *
 * Parameters:
 *  fd  File descriptor to read.
 *  line  Null-terminated input line.
 *  args  Caller-defined opaque pointer.
 */
typedef void (*QLIOReadCBF)(int fd, char *line, void *args);


/*
 * When an error occurs on a monitored socket, a callback function for
 * that socket is called. The socket is closed automatically.
 *
 * Parameters:
 *  fd  File descriptor to read.
 *  code  QIO close code.
 *  line  Partial input line at close time, or NULL if none.
 *  args  Caller-defined opaque pointer.
 */
typedef void (*QLIOCloseCBF)(int fd, int code, char *line, void *args);

/*
 * Registers a file descriptor with the line-buffered queued I/O module.
 *
 * Parameters:
 *  fd    File descriptor to register.
 *  read_cbf  Function to call when data arrives.
 *  close_cbf  Function to call when socket shuts down.
 *  args    Additional information to pass to callback function.
 *
 * Return value:
 *  0    File descriptor registered successfully.
 *  EBADF    Bad file descriptor.
 *  ENOMEM    Out of memory.
 */
int qlio_add(int fd, QLIOReadCBF read_cbf, QLIOCloseCBF close_cbf, void *args);

/*
 * Waits for a certain amount of binary data to accumulate, then calls the
 * read callback function with that data.
 *
 * Parameters:
 *  fd    File descriptor to update.
 *  bytes    Number of bytes of data to wait for.
 */
int qlio_expect_binary(int fd, int bytes);

#endif
