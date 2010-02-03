/*
 * Declarations and definitions for queued I/O module.
 *
 * Copyright (c) 2004-2005, Steven Grimm.
 */
#ifndef _QIO_H_
#define _QIO_H_ 1

#include "include.h"

/*
 * When data can be read from a monitored socket, a callback function for
 * that socket is called.
 *
 * Parameters:
 *  fd    File descriptor to read.
 *  args  Caller-defined opaque pointer.
 *
 * The function may call qio_remove() or qio_add() if needed, e.g. if
 * an error occurs and the socket needs to be closed.
 */
typedef void (*QIOReadCBF)(int fd, void *args);


/*
 * When data can be written to a monitored socket, a callback function for
 * that socket is called.  This is generally only used for nonblocking
 * connects since usually qio will handle writing.
 *
 * Parameters:
 *  fd    File descriptor to write.
 *  args  Caller-defined opaque pointer.
 * Returns:
 *  0    No more data to write for now (call qio_add_write again to
 *      turn write checking back on).
 *  1    Call the callback again when the descriptor is writable.
 *
 * The function may call qio_remove() or qio_add() if needed, e.g. if
 * an error occurs and the socket needs to be closed.
 */
typedef int (*QIOWriteCBF)(int fd, void *args);


/*
 * When a scheduled timer event occurs, its callback function is called.
 *
 * Parameters:
 *  args  Caller-defined opaque pointer.
 */
typedef void (*QIOTimerCBF)(void *args);


/*
 * When all the data has been flushed to a file descriptor after a call
 * to qio_flush(), a callback function is called.
 *
 * Parameters:
 *  fd  File descriptor that has finished flushing.
 *  args  Caller-defined opaque pointer.
 */
typedef void (*QIOFlushCBF)(int fd, void *args);


/*
 * When an error occurs on a monitored socket, a callback function for
 * that socket is called.  The callback does *not* need to call qio_remove();
 * the socket is automatically removed.
 *
 * Parameters:
 *  fd    File descriptor to read.
 *  code  Close code.
 *  args  Caller-defined opaque pointer.
 */
typedef void (*QIOCloseCBF)(int fd, int code, void *args);

/* QIOCloseCBF error code: Too many bytes queued for output. */
#define QIO_CLOSE_FULL    1

/* QIOCloseCBF error code: Remote side closed connection. */
#define QIO_CLOSE_REMOTE  2

/* QIOCloseCBF error code: Local side closed connection. */
#define QIO_CLOSE_LOCAL    3


/*
 * Initializes the queued I/O module.  Must be called before any other QIO
 * functions are called.
 */
void qio_init();

/*
 * Registers a file descriptor with the queued I/O module.
 *
 * Parameters:
 *  fd      File descriptor to register.
 *  read_cbf  Function to call when data arrives.
 *  close_cbf  Function to call when socket shuts down.
 *  args    Additional information to pass to callback function.
 *
 * Return value:
 *  0      File descriptor registered successfully.
 *  EBADF    Bad file descriptor.
 */
int qio_add(int fd, QIOReadCBF read_cbf, QIOCloseCBF close_cbf, void *args);

/*
 * Registers a file descriptor with the queued I/O module, but lets the
 * application handle writing.
 *
 * Parameters:
 *  fd      File descriptor to register.
 *  read_cbf  Function to call when data arrives.
 *  write_cbf  Function to call when descriptor is writable.
 *  close_cbf  Function to call when socket shuts down.
 *  args    Additional information to pass to callback function.
 *
 * Return value:
 *  0      File descriptor registered successfully.
 *  EBADF    Bad file descriptor.
 */
int qio_add_write(int fd, QIOReadCBF read_cbf, QIOWriteCBF write_cbf,
        QIOCloseCBF close_cbf, void *args);

/*
 * Removes a file descriptor from the queued I/O module.  Note that this
 * does not affect the file descriptor; it's up to the caller to close it
 * if needed.
 *
 * Parameters:
 *  fd      File descriptor to unregister.
 */
void qio_remove(int fd);

/*
 * Closes a file descriptor and removes it from the queued I/O module.
 * This causes the descriptor's close callback, if any, to be called.
 *
 * Parameters:
 *  fd      File descriptor to close.
 */
void qio_close(int fd);

/*
 * Writes to a qio-controlled file descriptor.  Use this instead of writing
 * directly using system calls, or there's no point using QIO!
 *
 * The actual write may be deferred; if an error occurs, the close callback
 * function is called.
 *
 * Parameters:
 *  fd      File descriptor to write to.
 *  data    Address of buffer to write.
 *  length    Number of bytes to write.
 */
void qio_write(int fd, const void *data, int length);

/*
 * Flushes pending output on a file descriptor.  When the flush is complete,
 * the specified callback function is called.
 *
 * Parameters:
 *  fd      File descriptor to write to.
 *  flush_cbf    Callback to call when flush finishes.
 */
void qio_flush(int fd, QIOFlushCBF flush_cbf);

/*
 * Schedules a timer to fire at a particular time.
 *
 * Parameters:
 *  when    When the event should fire.  If this is in the past
 *      (e.g. 0) the timer will fire immediately.
 *  cbf    Callback to call when event fires.
 *  args    Arguments to pass to callback function.
 *  interval  How often the event should repeat, in seconds.  If 0,
 *      the event will only fire once.
 */
void
qio_schedule(time_t when, QIOTimerCBF cbf, void *args, int interval);

/*
 * Cancels a timer event.
 *
 * Parameters:
 *  cbf    Callback supplied to qio_schedule().
 *  args    Arguments supplied to qio_schedule().  If NULL,
 *      all callbacks associated with cbf are cancelled.
 */
void qio_cancel(QIOTimerCBF cbf, void *args);

/*
 * Enables or disables immediate write attempts on a qio file descriptor.
 * If you're writing lots of little buffers that don't need to be delivered
 * until control returns to qio_main(), you should disable immediate writing
 * since it can result in additional network overhead.
 *
 * Parameters:
 *  fd      File descriptor to control.
 *  enable    Nonzero to enable immediate writing.
 */
void qio_enable_immediate(int fd, int enable);

/*
 * Returns the opaque argument pointer for a particular file descriptor.
 *
 * Parameters:
 *  fd      File descriptor to query.
 */
void *qio_get_opaque(int fd);

/*
 * Main loop.  Waits for data on all registered file descriptors and
 * calls callback functions as needed.
 *
 * Return values:
 *  -1      qio_init() wasn't called.
 *  0      Not monitoring any file descriptors.
 *  1      qio_stop() was called by a callback function.
 */
int qio_main();

/*
 * Stops the main loop.  This may have a delayed effect if called from a
 * second thread.
 */
void qio_stop();

/*
 * Cleans up after the queued I/O module, freeing all resources.
 */
void qio_cleanup();

/*
 * Dumps QIO stats to the provided file descriptor.
 */

void qio_dump_stats(int fd);

void qio_timeout_msec(int msec, QIOTimerCBF cbf, void *args);
#endif
