
/*
 * Queued I/O module.  This code handles reading and writing to large
 * numbers of nonblocking sockets.
 *
 * Copyright (c) 2004-2005, Steven Grimm.
 */
#include "qio.h"
#include "gfuncs.h"
#include <event.h>

/*
 * Output buffers smaller than this will be retained even after they're
 * drained, so we don't spend a lot of time allocating and freeing small
 * buffers. Set this to 0 to always free buffers as soon as they're flushed.
 */
#define MAX_DRAINED_BUFFER_SIZE  100000

/*
 * Each file descriptor we're handling has one of these structures.  They're
 * stored in an array indexed by file descriptor.
 */
typedef struct {
  int    fd;    /* Which file descriptor is this? */
  QIOReadCBF  readCBF;  /* Function to call on data arrival. */
  QIOCloseCBF  closeCBF;  /* Function to call on error. */
  QIOWriteCBF  writeCBF;  /* Function to call on writeability. */
  QIOFlushCBF  flushCBF;  /* Function to call when done writing. */
  void    *args;    /* Auxiliary data to pass to callbacks. */
  char    *output;  /* Data queued for output. */
  int    outputCount;  /* How many bytes are queued. */
  int    outputSize;  /* How big the output buffer is. */
  int    outputOffset;  /* How far into the output buffer we are. */
  int    immediateWrite; /* Nonzero to try writing in qio_write(). */
  struct event  *readEvent;
  struct event  *writeEvent;
  int    watchingWrite;
} QIO;

/*
 * Each scheduled event is described by an entry in this list.  The list
 * is sorted in order of firing time, i.e., the next event is always the
 * first one in the list.
 */
typedef struct _qio_event QIO_EVENT;
struct _qio_event {
  struct event  ev;
  QIOTimerCBF  cbf;    /* Callback function to call. */
  void    *args;    /* Arguments to pass to callback. */
  int    interval;  /* Interval to next event, or 0 for none. */
  int    done;    /* 1 if the event is finished. */
  QIO_EVENT  *next;    /* Next event in schedule. */
};

static QIO *qios = NULL;
static int qios_size = 0;
static int qio_init_count = 0;

/* Initial array size.  The array is grown as needed. */
#define QIO_INITIAL_SIZE  32

/* Amount of extra headroom to allocate when we grow the array. */
#define QIO_HEADROOM    7

/* Maximum file descriptor allowed, for sanity checking. */
#define QIO_MAX_LEGAL_FD  65535

/* Size of event hashtable. */
#define QIO_EVENT_HASH_SIZE  23


static QIO_EVENT *qio_event_hash[QIO_EVENT_HASH_SIZE];

static void qio_readable(int fd, short event, void *arg);
static void qio_writable(int fd, short event, void *arg);


/*
 * Resets after a fork() call.
 */
static int
qio_poll_forked()
{
  int  i;

  for (i = 0; i < qios_size; i++)
  {
    if (qios[i].fd >= 0)
    {
      /* Let the parent write any pending output. */
      qios[i].outputCount = qios[i].outputOffset = 0;
    }
  }

  return 0;
}

/*
 * Initializes the queued I/O module.  Must be called before any other QIO
 * functions are called.
 */
void
qio_init()
{
  int      i;

  /*
   * Keep count of the number of times we're called so we don't clean
   * up until qio_cleanup() has been called the same number of times.
   */
  if (qio_init_count++ > 0)
    return;

  /* Create an initial set of qio structures. */
  qios_size = QIO_INITIAL_SIZE;
  qios = (QIO*)gcalloc(sizeof(QIO) * qios_size);
  for (i = 0; i < qios_size; i++)
    qios[i].fd = -1;

  event_init();
}

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
int
qio_add(int fd, QIOReadCBF read_cbf, QIOCloseCBF close_cbf, void *args)
{
  int      i;

  if (fd < 0 || fd > QIO_MAX_LEGAL_FD)
    return EBADF;

  /* We require nonblocking output, so force it here. */
  if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
  {
    HPHP::Logger::Error("Can't set nonblocking mode on fd %d", fd);
    return EBADF;
  }

  /* Is this file descriptor too big for our array?  Expand it if so. */
  if (fd >= qios_size)
  {
    QIO    *new_qios;
    int    new_size = fd + QIO_HEADROOM + 1;

    new_qios = (QIO*)grealloc(qios, new_size * sizeof(QIO));
    for (i = fd; i < new_size; i++)
      new_qios[i].fd = -1;

    qios = new_qios;
    qios_size = fd + QIO_HEADROOM + 1;
  }

  /*
   * If this is a new file descriptor, set up its I/O elements. Reset
   * its output buffer just to be safe.
   */
  if (qios[fd].fd < 0)
  {
    qios[fd].fd = fd;
    qios[fd].output = NULL;
    qios[fd].outputCount = 0;
    qios[fd].outputSize = 0;
    qios[fd].immediateWrite = 1;
    qios[fd].watchingWrite = 0;
    qios[fd].readEvent = (event*)gmalloc(sizeof(struct event));
    qios[fd].writeEvent = (event*)gmalloc(sizeof(struct event));

    event_set(qios[fd].readEvent, fd, EV_READ | EV_PERSIST,
        qio_readable, (void *)(intptr_t)fd);
    event_set(qios[fd].writeEvent, fd, EV_WRITE | EV_PERSIST,
        qio_writable, (void *)(intptr_t)fd);

    event_add(qios[fd].readEvent, NULL);
  }

  qios[fd].readCBF = read_cbf;
  qios[fd].closeCBF = close_cbf;
  qios[fd].writeCBF = NULL;
  qios[fd].flushCBF = NULL;
  qios[fd].args = args;

  if (qios[fd].watchingWrite)
  {
    event_del(qios[fd].writeEvent);
    qios[fd].watchingWrite = 0;
  }

  return 0;
}


/*
 * Watches for writes on a file descriptor.
 */
static void
qio_watch_write(int fd)
{
  if (! qios[fd].watchingWrite)
  {
    event_add(qios[fd].writeEvent, NULL);
    qios[fd].watchingWrite = 1;
  }
}


/*
 * Registers a file descriptor with the queued I/O module, but lets the
 * application handle writes.
 *
 * Parameters:
 *  fd      File descriptor to register.
 *  read_cbf  Function to call when data arrives.
 *  write_cbf  Function to call when the descriptor is writable.
 *  close_cbf  Function to call when socket shuts down.
 *  args    Additional information to pass to callback function.
 *
 * Return value:
 *  0      File descriptor registered successfully.
 *  EBADF    Bad file descriptor.
 */
int
qio_add_write(int fd, QIOReadCBF read_cbf, QIOWriteCBF write_cbf,
      QIOCloseCBF close_cbf, void *args)
{
  if (qio_add(fd, read_cbf, close_cbf, args) < 0)
    return EBADF;
  qios[fd].writeCBF = write_cbf;
  qio_watch_write(fd);

  return 0;
}


/*
 * Removes a file descriptor from the queued I/O module.  Note that this
 * does not affect the file descriptor; it's up to the caller to close it
 * if needed.
 *
 * Parameters:
 *  fd      File descriptor to unregister.
 */
void qio_remove(int fd)
{
  if (fd < 0 || fd >= qios_size)
    return;

  /* Not monitoring this FD? */
  if (qios[fd].fd < 0)
    return;

  event_del(qios[fd].readEvent);
  event_del(qios[fd].writeEvent);
  qios[fd].watchingWrite = 0;
  gfree(qios[fd].readEvent);
  gfree(qios[fd].writeEvent);

  /* Let go of any pending output. */
  if (qios[fd].output != NULL)
  {
    gfree(qios[fd].output);
    qios[fd].output = NULL;
    qios[fd].outputCount = 0;
    qios[fd].outputSize = 0;
  }

  qios[fd].fd = -1;
}


/*
 * Closes a file descriptor and removes it from the queued I/O module.
 * The descriptor's close callback, if any, is called.
 *
 * Parameters:
 *  fd      File descriptor to close.
 */
void qio_close(int fd)
{
  /* Call close callback if we're monitoring this FD. */
  if (fd >= 0 && fd < qios_size && qios[fd].fd >= 0 &&
      qios[fd].closeCBF != NULL)
  {
    (qios[fd].closeCBF)(fd, QIO_CLOSE_LOCAL, qios[fd].args);
  }

  qio_remove(fd);
  close(fd);
}


/*
 * Adds bytes to a file descriptor's output queue and ensures we're watching
 * for writeability on the socket.
 *
 * Parameters:
 *  fd      File descriptor to add to.
 *  data    Data to add to queue.
 *  length    Length of data to add.
 */
static void
qio_add_to_output(int fd, void *data, int length)
{
  int    desiredSize;

  /* Make sure we're watching for writeability. */
  if (qios[fd].immediateWrite)
    qio_watch_write(fd);

  /* If this is the first data queued up, set up the queue. */
  if (qios[fd].output == NULL)
  {
    qios[fd].output = (char*)gmalloc_data(length * 2);
    qios[fd].outputSize = length * 2;
    qios[fd].outputOffset = 0;
    qios[fd].outputCount = 0;
  }

  /* Ensure there's enough additional space for this data. */
  desiredSize = qios[fd].outputCount + qios[fd].outputOffset + length;
  if (desiredSize > qios[fd].outputSize)
  {
    char    *newOutput;

    newOutput = (char*)grealloc(qios[fd].output, desiredSize);
    qios[fd].output = newOutput;
    qios[fd].outputSize = desiredSize;
  }

  if (qios[fd].outputCount == 0)
    qios[fd].outputOffset = 0;
  memcpy(qios[fd].output + qios[fd].outputOffset + qios[fd].outputCount,
      data, length);
  qios[fd].outputCount += length;
}


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
void qio_write(int fd, const void *data, int length)
{
  int      bytes_written;

  if (fd < 0 || fd >= qios_size || qios[fd].fd < 0)
    return;

  /* If there's already something queued up, just add to the queue. */
  if (qios[fd].outputCount > 0 || ! qios[fd].immediateWrite)
  {
    qio_add_to_output(fd, (void*)data, length);
    return;
  }

  /* Try to write immediately; no sense queuing unnecessarily. */
  bytes_written = write(fd, data, length);
  if (bytes_written == length)
    return;

  /*
   * If there was an error other than EWOULDBLOCK, we'll hit it again from
   * within the main loop.  Inefficient, maybe, but a cheap way to make
   * sure the close callback isn't called from within another callback.
   */
  if (bytes_written < 0)
  {
    if (errno != EWOULDBLOCK && errno != ENOTCONN)
    {
      HPHP::Logger::Error("Bad write; deferring error handling");
    }

    bytes_written = 0;
  }

  qio_add_to_output(fd, ((char*)data) + bytes_written, length-bytes_written);
}

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
void qio_enable_immediate(int fd, int enable)
{
  if (fd < 0 || fd >= qios_size || qios[fd].fd < 0)
    return;

  qios[fd].immediateWrite = enable;

  if (enable && qios[fd].outputCount > 0)
    qio_watch_write(fd);
}


/*
 * Try writing queued data to a descriptor that just polled writable.
 *
 * Parameters:
 *  fd      File descriptor to write to.
 *
 * Return values:
 *  0      No errors occurred.
 *  1      Couldn't write to descriptor.
 */
static int
qio_write_queued(int fd)
{
  int      bytes_written;
  QIOFlushCBF    flush_cbf;

  if (qios[fd].outputCount == 0)
  {
    printf("Polled for write on fd %d with no data queued\n", fd);
    event_del(qios[fd].writeEvent);
    qios[fd].watchingWrite = 0;
    return 0;
  }

  bytes_written = write(fd, qios[fd].output + qios[fd].outputOffset,
            qios[fd].outputCount);
  if (bytes_written < 0)
  {
    HPHP::Logger::Error("write");
    return 1;
  }

  /* Advance the buffer cursor in case we did a partial write. */
  qios[fd].outputCount -= bytes_written;
  if (qios[fd].outputCount > 0)
    qios[fd].outputOffset += bytes_written;
  else
  {
    /* Buffer is empty; don't need to check for writeability any more. */
    qios[fd].outputOffset = 0;
    event_del(qios[fd].writeEvent);
    qios[fd].watchingWrite = 0;

    flush_cbf = qios[fd].flushCBF;
    qios[fd].flushCBF = NULL;
    if (flush_cbf != NULL)
      (flush_cbf)(fd, qios[fd].args);

    /*
     * If the buffer is still empty and it's bigger than the
     * maximum size we want to retain after draining, free it.
     */
    if (qios[fd].outputCount == 0 &&
        qios[fd].outputSize > MAX_DRAINED_BUFFER_SIZE &&
        qios[fd].output != NULL)
    {
      gfree(qios[fd].output);
      qios[fd].outputSize = 0;
      qios[fd].outputOffset = 0;
      qios[fd].output = NULL;
    }
  }

  return 0;
}

/*
 * Returns the opaque argument pointer for a particular file descriptor.
 *
 * Parameters:
 *  fd      File descriptor to query.
 */
void *qio_get_opaque(int fd)
{
  if (fd < 0 || fd >= qios_size)
    return NULL;

  return qios[fd].args;
}


/*
 * Flushes pending output on a file descriptor.  When the flush is complete,
 * the specified callback function is called.
 *
 * Parameters:
 *  fd      File descriptor to write to.
 *  flush_cbf    Callback to call when flush finishes.
 */
void qio_flush(int fd, QIOFlushCBF flush_cbf)
{
  if (fd < 0 || fd >= qios_size)
    return;

  /* Already flushed?  Call the function immediately. */
  if (qios[fd].outputCount == 0)
    (flush_cbf)(fd, qios[fd].args);
  else
    qios[fd].flushCBF = flush_cbf;
}


/*
 * Removes and frees a timer event from the timer event list.
 *
 * Parameters:
 *  cbf    Callback function to remove.
 *  args    If non-null, (cbf, args) events are removed; otherwise
 *      (cbf, *) are removed.
 *  event    If non-null, only this event is removed even if there
 *      are others with the same cbf and args.
 */
static void
qio_event_remove(QIOTimerCBF cbf, void *args, QIO_EVENT *event)
{
  int  min, max;
  int  i;

  if (args != NULL)
  {
    min = (((unsigned int) (intptr_t) cbf) ^ ((unsigned int) (intptr_t) args)) % QIO_EVENT_HASH_SIZE;
    max = min;
  }
  else
  {
    min = 0;
    max = QIO_EVENT_HASH_SIZE - 1;
  }

  for (i = min; i <= max; i++)
  {
    QIO_EVENT  **ptr, *cur;

    for (ptr = &qio_event_hash[i]; *ptr != NULL; )
    {
      cur = *ptr;
      if ((cbf == NULL || cur->cbf == cbf) &&
          (args == NULL || cur->args == args) &&
          (event == NULL || cur == event))
      {
        *ptr = cur->next;
        if (event == NULL)
          event_del(&cur->ev);
        gfree(cur);
      }
      else
      {
        ptr = &cur->next;
      }
    }
  }
}


/*
 * A timer event has fired.
 */
static void
qio_event_timer(int fd, short code, void *arg)
{
  struct event  *event = (struct event *) arg;
  QIO_EVENT  *e = (QIO_EVENT *) event->ev_arg;
  QIOTimerCBF  cbf = e->cbf;
  void    *args = e->args;

  if (e->interval)
  {
    struct timeval wait;
    wait.tv_sec = e->interval;
    wait.tv_usec = 0;
    evtimer_add(event, &wait);
  }
  else
    qio_event_remove(e->cbf, e->args, e);

  cbf(args);
}


/*
 * Creates a new event and adds it to the schedule.
 *
 * Parameters:
 *  when    When the event should fire.  If this is in the past
 *        (e.g. 0) the timer will fire immediately.
 *  cbf      Callback to call when event fires.
 *  args    Arguments to pass to callback function.
 *  interval  How often the event should repeat, in seconds.  If 0,
 *        the event will only fire once.
 */
void
qio_schedule(time_t when, QIOTimerCBF cbf, void *args, int interval)
{
  QIO_EVENT    *e;
  struct timeval    now_tv;
  struct timeval    when_tv;
  struct timeval    in_tv;
  int      hash;

  hash = (((unsigned int) (intptr_t) cbf) ^ ((unsigned int) (intptr_t) args)) % QIO_EVENT_HASH_SIZE;

  e = (QIO_EVENT*)gmalloc(sizeof(QIO_EVENT));

  e->cbf = cbf;
  e->args = args;
  e->interval = interval;
  e->next = qio_event_hash[hash];
  qio_event_hash[hash] = e;

  /* Figure out how long to wait until the scheduled time. */
  gettimeofday(&now_tv, NULL);
  when_tv.tv_sec = when;
  when_tv.tv_usec = 0;
  timersub(&when_tv, &now_tv, &in_tv);

  evtimer_set(&e->ev, qio_event_timer, e);
  evtimer_add(&e->ev, &in_tv);
}

void qio_timeout_msec(int msec, QIOTimerCBF cbf, void *args) {
  QIO_EVENT *e;
  struct timeval in_tv;
  int hash;

  hash = (((unsigned int) (intptr_t) cbf) ^ ((unsigned int) (intptr_t) args)) % QIO_EVENT_HASH_SIZE;

  e = (QIO_EVENT*)gmalloc(sizeof(QIO_EVENT));

  e->cbf = cbf;
  e->args = args;
  e->interval = 0;
  e->next = qio_event_hash[hash];
  qio_event_hash[hash] = e;

  in_tv.tv_sec = msec / 1000;
  in_tv.tv_usec = (msec % 1000) * 1000;

  evtimer_set(&e->ev, qio_event_timer, e);
  evtimer_add(&e->ev, &in_tv);
}


/*
 * Cancels a scheduled or recurring timer event.
 *
 * Parameters:
 *  cbf    Callback supplied to qio_schedule() or
 *      qio_schedule_recurring().
 *  args    Arguments supplied to qio_schedule().  If NULL,
 *      all callbacks associated with cbf are cancelled.
 */
void qio_cancel(QIOTimerCBF cbf, void *args)
{
  qio_event_remove(cbf, args, NULL);
}


/*
 * Main loop.  Waits for data on all registered file descriptors and
 * calls callback functions as needed.
 *
 * Return values:
 *  -1      qio_init() wasn't called.
 *  0      Not monitoring any file descriptors.
 *  1      qio_stop() was called by a callback function.
 */
int
qio_main()
{
  if (qios == NULL)
    return -1;
  if (event_dispatch() < 0)
    return -1;
  return 0;
}


/*
 * A file descriptor has become readable. Call the application callback.
 */
static void
qio_readable(int fd, short event, void *arg)
{
  QIO *q = &qios[(intptr_t)arg];

  (q->readCBF)(fd, q->args);
}

/*
 * A file descriptor has become writable. If we're managing its writes (the
 * typical case), write any pending output. If not, call the application's
 * write callback.
 */
static void
qio_writable(int fd, short event, void *arg)
{
  QIO *q = &qios[(intptr_t)arg];

  if (q->writeCBF != NULL)
  {
    event_del(q->writeEvent);
    if ((q->writeCBF)(fd, q->args) == 0) {
      q->watchingWrite = 0;
          } else {
      // we do this little dance because the cbf might want to add
      // it's own write event. Therefore, we can't delete t after
      // the cb.
      event_add(q->writeEvent, NULL);
    }
  }
  else if (qio_write_queued(fd))
  {
    /* Tell the app we got a write error, and remove the fd. */
    (q->closeCBF)(fd, QIO_CLOSE_REMOTE, q->args);
    qio_remove(fd);
  }
}


/*
 * Adjusts after a fork() call.  Call this in the child process.
 */
void qio_forked()
{
  qio_poll_forked();
}


/*
 * Stops the main loop.  This may have a delayed effect if called from a
 * second thread.
 */
void qio_stop()
{
  int  fd;

  for (fd = 0; fd < qios_size; fd++)
  {
    if (qios[fd].fd > -1)
    {
      event_del(qios[fd].readEvent);
      event_del(qios[fd].writeEvent);
      qios[fd].watchingWrite = 0;
      gfree(qios[fd].readEvent);
      gfree(qios[fd].writeEvent);
    }
  }

  qio_event_remove(NULL, NULL, NULL);
}

/*
 * Cleans up after the queued I/O module, freeing all resources.
 */
void qio_cleanup()
{
  if (qio_init_count == 1)
  {
    /* Do any necessary cleanup here. */
    qio_init_count = 0;
  }
  else if (qio_init_count > 1)
    qio_init_count--;
}


/*
 * Dumps some statistics about the QIO subsystem.
 */
void qio_dump_stats(int fd)
{
  unsigned long long  total_buf = 0;
  unsigned long     total_fds = 0;
  unsigned long    i;
  char buf[300];
  for (i = 0; (int)i < qios_size; i++)
  {
    if (qios[i].fd != -1)
    {
      sprintf(buf, "STAT QIO: fd %ld count %d size %d\n",
        i, qios[i].outputCount, qios[i].outputSize);
      qio_write(fd, buf, strlen(buf));
      total_fds++;
    }
    total_buf += qios[i].outputSize;
  }

  sprintf(buf, "STAT QIO: total fds %lu total buf %llu\n", total_fds, total_buf);
  qio_write(fd, buf, strlen(buf));
}


#ifdef UNIT_TEST

#include <signal.h>

/*
 * Unit test opens a server socket, connects two client sockets to it, and
 * sends an outrageous amount of data in both directions on both connections.
 * The read callback reads a byte at a time to force the library to queue.
 *
 * TODO:
 *
 * Test closing a socket from inside a callback.
 * Test forcing a write error, e.g. by calling shutdown() on the sly.
 * Test overrunning the output buffer.
 */

static char dummy[1024];

static void
sigint(int dummy)
{
  qio_stop();
}

static void
read_cbf(int fd, void *args)
{
  char c;

  if (read(fd, &c, 1) < 1)
  {
    printf("Can't read 1 byte from fd %d\n", fd);
    throw HPHP::Exception("exit with %d", 1);
  }

  if (c > 0)
  {
    printf("Got stop byte %d on fd %d\n", c, fd);
    qio_stop();
  }
}

static void
close_cbf(int fd, int type, void *args)
{
  printf("close_cbf(%d, %d)\n", fd, type);
}

static void
server_read_cbf(int fd, void *args)
{
  int    newfd = accept(fd, NULL, NULL);
  int    i;

  printf("accepted new fd %d\n", newfd);
  if (newfd >= 0)
  {
    qio_add(newfd, read_cbf, close_cbf, NULL);
    for (i = 0; i < 256; i++)
      qio_write(newfd, dummy, sizeof(dummy));
  }
}

static int do_stop = 0;

static void
timer_cbf(void *args)
{
  printf("timer_cbf(%s)\n", args);
  if (((char *)args)[0] == '2')
    do_stop = 1;
  else if (do_stop)
  {
    printf("canceling\n");
    qio_cancel(timer_cbf, args);
  }
}


main(int argc, char **argv)
{
  int cfd1, cfd2, server_fd;
  int  i;
  char c = 1;
  int port = (getpid() % 5000) + 15000;

  signal(SIGINT, sigint);

  memset(dummy, 0, sizeof(dummy));
  qio_init();

  qio_schedule(0, timer_cbf, "1", 1);
  qio_schedule(time(NULL) + 5, timer_cbf, "2", 0);

  server_fd = serversock(port);
  if (server_fd < 0)
  {
    perror("server socket");
    throw HPHP::Exception("exit with %d", 1);
  }
  printf("Server fd %d\n", server_fd);
  qio_add(server_fd, server_read_cbf, close_cbf, NULL);

  cfd1 = clientsock("localhost", port);
  if (cfd1 < 0)
  {
    perror("client socket");
    throw HPHP::Exception("exit with %d", 1);
  }
  printf("Client fd1 %d\n", cfd1);
  qio_add(cfd1, read_cbf, close_cbf, NULL);

  cfd2 = clientsock("localhost", port);
  if (cfd2 < 0)
  {
    perror("client socket");
    throw HPHP::Exception("exit with %d", 1);
  }
  printf("Client fd2 %d\n", cfd2);
  qio_add(cfd2, read_cbf, close_cbf, NULL);

  for (i = 0; i < 256; i++)
  {
    qio_write(cfd1, dummy, sizeof(dummy));
    qio_write(cfd2, dummy, sizeof(dummy));
  }

  qio_write(cfd1, &c, 1);

  printf("qio_main() returned %d\n", qio_main());
  printf("Done\n");

  qio_cleanup();
}

#endif
