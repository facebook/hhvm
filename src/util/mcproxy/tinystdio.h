/*
 * Tiny stdio replacement, because Solaris stdio can't handle more than 256
 * files at a time.
 */
#ifndef __TINYSTDIO_H__ /* { */
#define __TINYSTDIO_H__ 1

#include "include.h"

/* Dummy structure for type safety. */
typedef struct {
  char  dummy1[26];
  char  dummy2[31];
  int  dummy3[3];
} TINYFILE;

/*
 * Returns the next character from a stream, or EOF (-1) if none.
 */
int tiny_getc(TINYFILE *tfp);

/*
 * Reads a chunk of data from a stream.  Returns number of bytes read, 0
 * on EOF, or -1 on error.
 */
int tiny_read(TINYFILE *tfp, char *buf, int len);

/*
 * Reads a newline-terminated string from a stream.  Returns the buf
 * argument, or NULL if no string could be read.
 */
char *tiny_gets(TINYFILE *tfp, char *buf, int len);

/*
 * Attaches a tiny-stdio stream to an open file descriptor.
 */
TINYFILE *tiny_fdopen(int fd);

/*
 * Attaches a tiny-stdio stream to a buffer.
 */
TINYFILE *tiny_bufopen(char *buf, int len);

/*
 * Detaches a tiny-stdio stream from an open file descriptor or buffer.
 * The file descriptor is not affected, but any data that was buffered in the
 * stream's input buffer is lost.
 */
void tiny_free(TINYFILE *tfp);

/*
 * Rewinds a tiny-stdio stream to the beginning.
 */
void tiny_rewind(TINYFILE *tfp);

#endif /* } */
