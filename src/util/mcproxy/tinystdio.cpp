
/*
 * Tiny stdio replacement, because Solaris stdio can't handle more than 256
 * files at a time.
 */
#include "tinystdio.h"
#include "gfuncs.h"

typedef struct {
  char    *inbuf;
  int    fd;
  int    in_bytes;  // Number of bytes in buffer
  int    in_pos;    // Position in buffer (<= in_bytes)
} TINYFILE_IMPL;

#define INBUF_SIZE 8192

/*
 * Fills the input buffer of a stream.
 *
 * Returns number of bytes read, or -1 if an error occurred.
 */
static int fill_inbuf(TINYFILE_IMPL *fp)
{
  int  ret;

  // No-op if there are already bytes in the buffer
  if (fp->in_bytes > 0 && fp->in_pos < fp->in_bytes)
    return 0;

  // If this isn't an fd-based stream, can't read any bytes
  if (fp->fd < 0)
    return 0;

  fp->in_pos = 0;
  do {
    ret = read(fp->fd, fp->inbuf, sizeof(fp->inbuf));
  } while (ret == -1 && errno == EINTR);

  if (ret >= 0)
    fp->in_bytes = ret;
  else
    fp->in_bytes = 0;

  return ret;
}

/*
 * Returns the next character from a stream.
 */
int tiny_getc(TINYFILE *tfp)
{
  TINYFILE_IMPL  *fp = (TINYFILE_IMPL *) tfp;

  if (fp == NULL)
    return EOF;

  if (fp->in_bytes > 0 && fp->in_pos < fp->in_bytes)
    return fp->inbuf[fp->in_pos++];

  if (fill_inbuf(fp) <= 0)
    return EOF;
  return fp->inbuf[fp->in_pos++];
}

/*
 * Reads a chunk of data from a stream.
 */
int tiny_read(TINYFILE *tfp, char *buf, int len)
{
  TINYFILE_IMPL  *fp = (TINYFILE_IMPL *) tfp;
  int    bytes_from_buf;
  int    left = len;
  int    ret;

  if (fp == NULL || buf == NULL)
    return -1;
  if (len == 0)
    return 0;

  // If the buffer is empty and this is a big read, just fetch
  // directly to the user's buffer.
  if (fp->in_bytes == 0 || (fp->in_bytes <= fp->in_pos && fp->fd >= 0))
  {
    if (len >= (int)sizeof(fp->inbuf))
      return read(fp->fd, buf, len);

    // A small request; replenish our buffer.
    if (fill_inbuf(fp) < 0)
      return -1;
  }

  if (fp->in_bytes == 0 || fp->in_bytes <= fp->in_pos)
    return 0;

  // Satisfy as much of the request as possible from the buffer.
  bytes_from_buf = fp->in_bytes - fp->in_pos;
  if (bytes_from_buf > left)
    bytes_from_buf = left;

  if (bytes_from_buf > 0)
  {
    memcpy(buf, fp->inbuf + fp->in_pos, bytes_from_buf);
    fp->in_pos += bytes_from_buf;
    left -= bytes_from_buf;
    buf += bytes_from_buf;
  }

  // Satisfied the whole request?
  if (left == 0)
    return len;

  // If this was a big request, satisfy the rest of it directly from
  // the file descriptor.  Otherwise replenish our buffer.
  if (len >= (int)sizeof(fp->inbuf))
    ret = read(fp->fd, buf, left);
  else
    ret = tiny_read(tfp, buf, left);

  if (ret >= 0)
    return bytes_from_buf + ret;
  else
    return bytes_from_buf;
}

/*
 * Reads a newline-terminated string from a stream.
 */
char *tiny_gets(TINYFILE *tfp, char *buf, int len)
{
  int    c;
  int    pos = 0;

  if (tfp == NULL || buf == NULL || len < 2)
    return NULL;

  while ((c = tiny_getc(tfp)) != EOF)
  {
    buf[pos++] = c;
    if (pos == len - 1 || c == '\n')
      break;
  }

  buf[pos] = '\0';
  if (pos > 0)
    return buf;
  else
    return NULL;
}

/*
 * Rewinds a tiny-stdio stream.
 */
void tiny_rewind(TINYFILE *tfp)
{
  TINYFILE_IMPL  *fp = (TINYFILE_IMPL *)tfp;

  lseek(fp->fd, 0, SEEK_SET);
  fp->in_bytes = 0;
  fp->in_pos = 0;
}


/*
 * Attaches a tiny-stdio stream to an open file descriptor.
 */
TINYFILE *tiny_fdopen(int fd)
{
  TINYFILE_IMPL *fp =
    (TINYFILE_IMPL*)gmalloc_data(sizeof(TINYFILE_IMPL) + INBUF_SIZE);

  fp->fd = fd;
  fp->inbuf = (char *)&fp[1];
  fp->in_bytes = 0;
  fp->in_pos = 0;

  return (TINYFILE *) fp;
}


/*
 * Attaches a tiny-stdio stream to a buffer.
 */
TINYFILE *tiny_bufopen(char *buf, int len)
{
  TINYFILE_IMPL  *fp = (TINYFILE_IMPL*)gmalloc_data(sizeof(TINYFILE_IMPL));

  fp->fd = -1;
  fp->inbuf = buf;
  fp->in_bytes = len;
  fp->in_pos = 0;

  return (TINYFILE *) fp;
}


/*
 * Detaches a tiny-stdio stream from an open file descriptor.  The file
 * descriptor is not affected, but any data that was buffered in the
 * stream's input buffer is lost.
 */
void tiny_free(TINYFILE *tfp)
{
  gfree(tfp);
}
