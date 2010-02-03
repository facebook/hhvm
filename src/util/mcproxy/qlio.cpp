
/*
 * Line-buffered queued I/O module.  A layer on top of the qio module that
 * provides line-at-a-time input.
 *
 * Copyright (c) 2004-2005, Steven Grimm.
 */
#include "qio.h"
#include "qlio.h"
#include "gfuncs.h"

/*
 * Each file descriptor we're handling has one of these structures to hold
 * its metadata and partial input.
 */
typedef struct {
  int    fd;    /* Which file descriptor is this? */
  QLIOReadCBF  readCBF;  /* Function to call on line arrival. */
  QLIOCloseCBF  closeCBF;  /* Function to call on error/EOF. */
  void    *args;    /* Client-supplied context. */

  char    *inbuf;    /* Buffer for partial input. */
  int    inbufSize;  /* Size of input buffer. */
  int    inbufCount;  /* Number of bytes received so far. */

  int    binarySize;  /* Number of binary bytes required. */
  int    inReadCBF;  /* Are we calling a read callback? */
  int    closed;
} QLIO;


static void qlio_cleanup(QLIO *qlio);

/*
 * Ensures that we have a minimum number of bytes of free space in an input
 * buffer.
 */
static int
ensure_buffer(QLIO *qlio, int bytes)
{
  char  *buf;

  if (qlio->inbufSize - qlio->inbufCount >= bytes)
    return 0;
  if (qlio->inbuf == NULL)
    buf = (char*)gmalloc_data(bytes);
  else
    buf = (char*)grealloc(qlio->inbuf, bytes + qlio->inbufSize);

  qlio->inbuf = buf;
  qlio->inbufSize += bytes;
  return 0;
}


/*
 * Reads input from a ready descriptor. If the input contains one or more
 * complete lines, pass them to the application.
 */
static void
qlio_read(int fd, void *args)
{
  QLIO  *qlio = (QLIO *) args;
  int  bytesread;
  int  pos = 0;
  char  *c;

  if (ensure_buffer(qlio, 512))
    return;
  bytesread = read(fd, qlio->inbuf + qlio->inbufCount,
      qlio->inbufSize - qlio->inbufCount - 1);
  if (bytesread <= 0)
  {
    qlio->inbuf[qlio->inbufCount] = '\0';
    qio_close(fd);
    return;
  }

  qlio->inbufCount += bytesread;
  qlio->inbuf[qlio->inbufCount] = '\0';

  do {
    /*
     * If we're waiting for binary data, pass it to the caller when
     * the requested number of bytes have accumulated.
     */
    while (qlio->binarySize > 0)
    {
      if (qlio->inbufCount - pos >= qlio->binarySize)
      {
        int startPos = pos;
        pos += qlio->binarySize;
        qlio->binarySize = 0;
        qlio->inReadCBF++;
        (qlio->readCBF)(fd, qlio->inbuf + startPos, qlio->args);
        qlio->inReadCBF--;

        if (qlio->closed)
          break;
      }
      else
        break;
    }

    if (qlio->binarySize > 0 || qlio->closed)
      break;

    c = strchr(qlio->inbuf + pos, '\n');
    if (c != NULL)
    {
      *c = '\0';

      /* If this line was \r\n terminated, lose the \r too. */
      if (c > (qlio->inbuf + pos) && *(c-1) == '\r')
        *(c-1) = '\0';

      if (qlio->readCBF != NULL)
      {
        qlio->inReadCBF++;
        (qlio->readCBF)(fd, qlio->inbuf + pos,
            qlio->args);
        qlio->inReadCBF--;

        if (qlio->closed)
          break;
      }

      pos = c + 1 - qlio->inbuf;
    }
    else
    {
      break;
    }
  } while (pos < qlio->inbufCount);

  if (qlio->closed)
    qlio_cleanup(qlio);
  else
  {
    if (qlio->inbufCount != pos)
    {
      memmove(qlio->inbuf, qlio->inbuf + pos,
        qlio->inbufCount - pos);
    }
    qlio->inbufCount -= pos;
  }
}


/*
 * Handles closure of a descriptor.
 */
static void
qlio_close(int fd, int code, void *args)
{
  QLIO  *qlio = (QLIO *) args;

  if (code != QIO_CLOSE_LOCAL)
    close(fd);
  if (qlio->closeCBF != NULL)
  {
    if (qlio->inbufCount > 0)
    {
      ensure_buffer(qlio, 1);
      qlio->inbuf[qlio->inbufCount] = '\0';
      (qlio->closeCBF)(fd, code, qlio->inbuf, qlio->args);
    }
    else
    {
      (qlio->closeCBF)(fd, code, NULL, qlio->args);
    }
  }

  if (qlio->inReadCBF)
    qlio->closed = 1;
  else
    qlio_cleanup(qlio);
}


/*
 * Cleans up after closure of a descriptor. If the application calls qio_close()
 * in one of our read callbacks, this is deferred until after the callback
 * returns.
 */
static void
qlio_cleanup(QLIO *qlio)
{
  if (qlio->inbuf != NULL)
    gfree(qlio->inbuf);
  gfree(qlio);
}


/*
 * Registers a file descriptor with the qlio module.
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
 *  ENOMEM    Out of memory.
 */
int
qlio_add(int fd, QLIOReadCBF read_cbf, QLIOCloseCBF close_cbf, void *args)
{
  QLIO  *qlio;
  int  status;

  qlio = (QLIO*)gcalloc(sizeof(QLIO));
  qlio->fd = fd;
  qlio->readCBF = read_cbf;
  qlio->closeCBF = close_cbf;
  qlio->args = args;

  /* Register this FD with the QIO module. */
  status = qio_add(fd, qlio_read, qlio_close, qlio);
  if (status)
  {
    gfree(qlio);
    return status;
  }

  return 0;
}

/*
 * Waits for a certain amount of binary data to accumulate, then calls the
 * read callback function with that data.
 *
 * Parameters:
 *  fd    File descriptor to update.
 *  bytes    Number of bytes of data to wait for.
 */
int
qlio_expect_binary(int fd, int bytes)
{
  QLIO  *qlio;

  qlio = (QLIO *) qio_get_opaque(fd);
  if (qlio != NULL)
    qlio->binarySize = bytes;
  return 0;
}
