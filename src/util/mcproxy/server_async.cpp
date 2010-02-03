/*
 * Asynchronous request queue management.
 */
#include "proxy.h"
#include "server_int.h"
#include "stats.h"
#include "gfuncs.h"

char *async_dir = "/tmp";
uint32_t max_asyncstore_len = 1048576; // 1MB

static void server_async_cbf(REQUEST *req, void *arg);


/*
 * Opens the asynchronous request store for a server.
 *
 * If for_adding is nonzero, creates the file if it doesn't exist and sets
 * the current position to the end of the file.
 */
static void
server_asyncstore_open(MEMCACHED_IMPL *mcd, int for_adding)
{
  int create_flag = for_adding ? O_CREAT : 0;

  if (mcd->store_fd > -1)
    return;      // already open
  mcd->store_fd = open(mcd->store_path, O_RDWR | create_flag, 0666);
  if (mcd->store_fd < 0)
  {
    if (errno == ENOENT && ! for_adding)
      return;
    HPHP::Logger::Error("Can't open async store %s", mcd->store_path);
    return;
  }

  if (for_adding)
  {
    struct stat st;
    if (fstat(mcd->store_fd, &st))
    {
      HPHP::Logger::Error("Can't stat async store %s", mcd->store_path);
      return;
    }

    mcd->store_offset = st.st_size;
  }

  HPHP::Logger::Verbose("Opened async store for %s", mcd->description);
}


/*
 * Loads all the entries from a server's asynchronous request store.
 */
void
server_asyncstore_init(MEMCACHED_IMPL *mcd)
{
  REQUEST *req;
  struct stat st;
  char *lines;
  char *cmd;

  mcd->store_path = (char*)gmalloc(strlen(async_dir) + 30);
  sprintf(mcd->store_path, "%s/async.%s.%d",
    async_dir, mcd->hostaddr, mcd->port);

  server_asyncstore_open(mcd, 0);
  if (mcd->store_fd > -1)
  {
    if (fstat(mcd->store_fd, &st) < 0)
    {
      HPHP::Logger::Error("Can't get file length for async store");
      return;
    }

    lines = (char*)gmalloc_data(st.st_size + 1);
    if (read(mcd->store_fd, lines, st.st_size) < st.st_size)
    {
      HPHP::Logger::Error("Can't read async store");
      return;
    }

    lines[st.st_size] = '\0';

    /*
     * The store is a series of \0 <command> \0 sequences,
     * with already-acknowledged commands marked by a "!" as the
     * first character. Add all the commands to the server's
     * outgoing request list.
     */
    cmd = lines;
    while (cmd < lines + st.st_size - 1)
    {
      if (cmd[0] != '\0')
      {
        /*
         * This is an already-acknowledged entry or the
         * store has wrapped around and this is the end
         * of an old entry. Either way, skip it.
         */
        cmd += strlen(cmd) + 1;
        if (cmd >= lines + st.st_size - 1)
          break;
      }

      /* Skip past the opening null */
      cmd++;

      if (cmd[0] != '!')
      {
        req = request_new_async(server_async_cbf, cmd,
              cmd - lines, 1);
        request_set_server(req, (MEMCACHED *)mcd);
        request_list_add(mcd->request_list, req);
        mcd->queued_async_size += strlen(cmd);
      }

      cmd += strlen(cmd) + 1;
    }

    gfree(lines);

    // Leave the file open since we'll be marking requests as
    // complete once we connect to the server.

    mcd->store_offset = st.st_size;
  }
  else
  {
    mcd->store_offset = 0;
  }
}

/*
 * Empties out a server's asynchronous request store.
 */
static void
server_asyncstore_empty(MEMCACHED_IMPL *mcd)
{
  if (mcd->store_fd > -1)
  {
    close(mcd->store_fd);
    unlink(mcd->store_path);
    mcd->store_fd = -1;
  }
}

/*
 * Removes an asynchronous request from the persistent store if it's there.
 * We don't actually remove the entry as such; we just mark it as invalid
 * by tweaking the first character. Once all the stored requests have been
 * sent, the file will be truncated.
 */
static void
server_asyncstore_remove(MEMCACHED_IMPL *mcd, REQUEST *req)
{
  uint32_t  offset;

  if (request_get_type(req) != Async)
    return;
  offset = request_get_offset(req);
  if (offset == OFFSET_NONE)
    return;

  server_asyncstore_open(mcd, 0);
  if (mcd->store_fd > -1)
  {
    if (lseek(mcd->store_fd, (off_t) offset, SEEK_SET) != offset)
    {
      HPHP::Logger::Error("Can't seek in async store");
      return;
    }

    if (write(mcd->store_fd, "!", 1) != 1)
    {
      HPHP::Logger::Error("Can't mark record as complete");
      return;
    }
  }
}

/*
 * Adds an asynchronous request to the persistent store.
 */
static void
server_asyncstore_add(MEMCACHED_IMPL *mcd, REQUEST *req)
{
  /* If this request is already in the store, don't store it again. */
  if (request_get_type(req) == Async &&
      request_get_offset(req) != OFFSET_NONE)
    return;

  server_asyncstore_open(mcd, 1);
  if (mcd->store_fd > -1)
  {
    int command_len = request_get_command_len(req);
    char *command = (char*)request_get_command(req);
    struct iovec iov[3];

    /* If we would push the file over its size limit, wrap to 0
       and truncate the file.  If we fail to truncate the file
       then close the file, unlink it and return.  Rely on
       external monitoring to alert us when this failure
       occurs. */
    if (command_len + mcd->store_offset > max_asyncstore_len)
    {
      HPHP::Logger::Warning("Request queue %s has reached its size limit",
                            mcd->store_path);
      mcd->store_offset = 0;
      if (ftruncate(mcd->store_fd, 0) < 0) {
        HPHP::Logger::Error("Can't ftruncate in async store");
        server_asyncstore_empty(mcd);
        return;
      }
    }

    if (lseek(mcd->store_fd, mcd->store_offset, SEEK_SET) < 0)
    {
      HPHP::Logger::Error("Can't seek in async store");
      return;
    }

    /* Write a null byte, the command, and another null. */
    iov[0].iov_base = iov[2].iov_base = (void*)"";
    iov[0].iov_len = iov[2].iov_len = 1;
    iov[1].iov_base = command;
    iov[1].iov_len = command_len;
    if (writev(mcd->store_fd, iov, 3) != command_len + 2)
    {
      HPHP::Logger::Error("Can't append entry to async store");
      return;
    }

    // XXX - If we want to be really paranoid, we can call fsync
    //       at this point. But that will generate a lot of disk
    //       I/O and it's not clear it's worthwhile.

    mcd->store_offset += command_len + 2;
  }
}


/*
 * Request handler callback for an asynchronous request. This is only called
 * when we actually get a response from the server; if the server connection
 * is closed, async requests are requeued by server_requeue_requests().
 */
static void
server_async_cbf(REQUEST *req, void *arg)
{
  MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) request_get_server(req);

  /*
   * Remove this request from the store if it was there. This
   * can only be true if the request was reliable (flags != 0).
   */
  if (request_is_reliable_async(req)) {
    server_asyncstore_remove(mcd, req);
  }

  /* If this was the last pending request, empty out the store. */
  if (request_list_peek_tail(mcd->request_list) == req)
    server_asyncstore_empty(mcd);

  if (request_is_reliable_async(req)) {
    mcd->queued_async_size -= request_get_command_len(req);
  }

  stats_remote_delete(req);
  request_free(req);
}

/*
 * Sends a request to a server asynchronously, optionally deferring it
 * if it can't be sent immediately.
 */
void
server_do_request_async(MEMCACHED *vmcd, const REQUEST *req, int reliable)
{
  MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) vmcd;
  REQUEST    *async;
  int    send_failed;

  if (request_get_data(req) != NULL)
    async = request_new_async_data(server_async_cbf,
                                   (char*)request_get_command(req),
                                   request_get_offset(req),
                                   request_get_data(req),
                                   request_get_bytes(req),
                                   reliable);
  else
    async = request_new_async(server_async_cbf,
                              (char*)request_get_command(req),
                              request_get_offset(req),
                              reliable);

  send_failed = server_do_request(vmcd, async);

  if (reliable)
  {
    if (send_failed)
    {
      /* might have failed because we are incorrectly sending
       * a request to a dummy server.  stop the request now. */
      if (! server_is_real(vmcd)) {
        return;
      }

      request_set_server(async, vmcd);
      request_list_add(mcd->request_list, async);
      server_asyncstore_add(mcd, async);
    }

    /*
     * Add it to the async queue even if it was successfully sent;
     * it will be removed when we get a response back from the
     * remote side. (Otherwise we would lose requests that were
     * pending at the time a remote server died.)
     */
    mcd->queued_async_size += request_get_command_len(async);

    int num_requests_dropped = 0;
    while (mcd->queued_async_size > max_asyncstore_len)
    {
      REQUEST  *old = request_list_next(mcd->request_list);

      /* The queue is full. Forget about an old entry. */
      if (server_is_up(vmcd))
      {
        mcd->pending_async_drop_count++;
      }
      request_handle_input(old, "SERVER_ERROR Queue full");
      num_requests_dropped++;
    }
    if (num_requests_dropped > 0) {
      HPHP::Logger::Error("Server %s async queue full, dropping %d requests.",
               mcd->description,
               num_requests_dropped);
    }
  }
  else
  {
    if (send_failed)
    {
      HPHP::Logger::Error("Server %s request dropped - server was down.",
            mcd->description);
      /* Request won't be freed by completion callback */
      request_free(async);
    }
  }
}

/*
 * Requeues all of the pending requests for a server that has just
 * reconnected.
 */
void
server_requeue_requests(MEMCACHED_IMPL *mcd)
{
  REQUEST *tail = request_list_peek_tail(mcd->request_list);
  REQUEST *cur;

  if (tail != NULL)
  {
    /*
     * Walk through the requests until we get to whatever was at the
     * tail of the list initially. (We will be adding each entry to
     * the end of the list as we pop it off the beginning.)
     */
    do {
      cur = request_list_next(mcd->request_list);
      mcd->queued_async_size -= request_get_command_len(cur);
      server_do_request_async((MEMCACHED *) mcd, cur, 1);
      request_free(cur);
    } while (cur != tail);
  }
}
