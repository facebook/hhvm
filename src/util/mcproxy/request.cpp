/*
 * Request queue module.
 */
#include "proxy.h"
#include "request_int.h"
#include "stats.h"
#include "server.h"
#include "gfuncs.h"

/* Externs to return status request information */
extern char* latest_config;
extern int latest_config_len;
extern char* latest_config_file;
extern char* latest_config_server;
extern time_t last_config_update_time;

/* Freelist of REQUEST *, faster than reallocing even with GC enabled. */
static REQUEST_LIST *freelist;

/*
 * Disposes of a request.
 */
void
request_free(REQUEST *vreq)
{
  REQUEST_IMPL *req = (REQUEST_IMPL *) vreq;

  if (req != NULL)
  {
    uint32_t  i;

    for (i = 0; i < req->nkeys; i++)
      gfree(req->keys[i]);
    gfree(req->response);
    gfree(req->command);
    gfree(req->data);
    req->response = NULL;

    request_list_add(freelist, (REQUEST *) req);
  }
}


/*
 * Handle a response to a request that doesn't fetch any data.
 */
static int
request_done(REQUEST *vreq, char *line)
{
  REQUEST_IMPL *req = (REQUEST_IMPL *) vreq;

  if (req->response != NULL)
    gfree(req->response);
  req->response = gstrdup_const(line);
  req->status = FinalResult;
  (req->cbf)(vreq, req->arg);

  return 0;
}


/*
 * Handle a response to a data retrieval command.
 *
 * Returns 1 if we're still waiting for more input.
 */
static int
request_get_input(REQUEST *vreq, char *line)
{
  REQUEST_IMPL  *req = (REQUEST_IMPL *) vreq;
  int    response_len;
  int    need_more = 1;

  /* Is this an information line? */
  if (req->response == NULL)
  {
    response_len = strlen(line);
    req->response = gstrdup(line);

    /*
     * For internally generated responses like the SERVER_ERROR
     * we return when we lose connectivity to memcached,
     * "line" can be a string constant in read-only memory.
     * So we use strncmp() to figure out the response code rather
     * than splitting line up using strtok().
     */
    if (! strncmp(line, "END", 3))
    {
      req->status = FinalResult;
      (req->cbf)(vreq, req->arg);
      need_more = 0;
    }
    else if (! strncmp(line, "VALUE ", 6))
    {
      char *key, *flags = NULL, *bytes = NULL;

      key = strtok(line + 6, " ");
      if (key != NULL)
        flags = strtok(NULL, " ");
      if (flags != NULL)
        bytes = strtok(NULL, " ");

      if (bytes != NULL)
      {
        req->bytes = atoi(bytes);
        req->data = grealloc(req->data, req->total +
              req->bytes +
              response_len + 4);
        memcpy((char*)req->data + req->total, req->response,
          response_len);
        req->total += response_len;
        memcpy((char*)req->data + req->total, "\r\n", 2);
        req->total += 2;

        server_expect_bytes(req->server,
            req->bytes + 2);
      }
      else
      {
        HPHP::Logger::Error("Malformed server response %s",
          req->response);
        req->status = Error;
        (req->cbf)(vreq, req->arg);
        req->ignore = 1;

        gfree(req->response);
        req->response = NULL;

        need_more = 0;
      }
    }
    else if (!strncmp("SERVER_ERROR", line, 12))
    {
      req->status = Error;
      req->data = grealloc(req->data, response_len + 4);

      memcpy(req->data, line, response_len);
            req->total = response_len;
            memcpy((char*)req->data + req->total, "\r\n", 2);
            req->total += 2;

      (req->cbf)(vreq, req->arg);
      req->ignore = 1;

      gfree(req->response);
      req->response = NULL;

      need_more = 0;
    }
    else
    {
      HPHP::Logger::Error("Unrecognized server response %s", req->response);
      req->status = Error;
      (req->cbf)(vreq, req->arg);
      req->ignore = 1;

      gfree(req->response);
      req->response = NULL;

      need_more = 0;
    }
  }
  else
  {
    /*
     * If the request is resulting an error, we would never have
     * allocated req->data.  We'll be ignoring the request anyway,
     * so there's no need to attempt to copy the data over.
     */
    if (req->status != Error) {
      /*
       * It's the data for a get request. We've already allocated
       * buffer space. The "+2" is for the trailing \r\n.
       */
      memcpy((char*)req->data + req->total, line, req->bytes + 2);
      req->total += req->bytes + 2;
    }

    gfree(req->response);
    req->response = NULL;
  }

  return need_more;
}


/*
 * Allocates a new request.
 */
static REQUEST_IMPL *
request_new(REQUEST_TYPE type,
    const char **keys,
    uint32_t nkeys,
    void *data,
    uint32_t bytes,
    uint32_t expiration,
    REQUEST_CBF cbf,
    void *arg)
{
  REQUEST_IMPL  *req;
  uint32_t  i;

  req = (REQUEST_IMPL *) request_list_next(freelist);
  if (req == NULL)
    req = (REQUEST_IMPL*)gcalloc(sizeof(REQUEST_IMPL));

  if (keys != NULL)
  {
    /* Reuse the keys array from the freelist if we can. */
    if (nkeys > req->nkeys_alloced)
    {
      req->keys = (char**)grealloc(req->keys, sizeof(char *) * nkeys);
      req->nkeys_alloced = nkeys;
    }
    for (i = 0; i < nkeys; i++)
      req->keys[i] = gstrdup_const(keys[i]);
    for (; i < req->nkeys_alloced; i++)
      req->keys[i] = NULL;
  }

  req->response = NULL;
  req->server = NULL;
  req->type = type;
  req->command = NULL;
  req->command_len = 0;
  req->cbf = cbf;
  req->arg = arg;
  req->nkeys = nkeys;
  req->data = data;
  req->bytes = bytes;
  req->total = 0;
  req->flags = 0;
  req->expiration = expiration;
  req->status = Pending;
  req->input_cbf = request_done;
  req->ignore = 0;
  req->client_closed = 0;

  return req;
}


/*
 * Constructor for all the data-storage requests, which look the same except
 * for the command name.
 */
static REQUEST *
request_new_storage(REQUEST_TYPE type, const char *cmd, const char *key,
    void *data, uint32_t bytes, uint32_t expiration, uint32_t flags,
    REQUEST_CBF cbf, void *arg)
{
  REQUEST_IMPL *req = request_new(type, &key, 1, data, bytes,
          expiration, cbf, arg);
  char *format = "%s %s %u %u %u\r\n";

  // allocate enough for the maximum lengths of all the fields,
  // plus the trailing \r\n
  req->command = (char*)gmalloc_data(strlen(key) + strlen(format) + 38);

  sprintf(req->command, format, cmd, key, flags, expiration, bytes);
  req->command_len = strlen(req->command);

  req->data = data;
  req->bytes = bytes;

  return (REQUEST *) req;
}


/*
 * "Constructors" for different request types.
 */
REQUEST *
request_new_get(const char **keys, uint32_t nkeys, REQUEST_CBF cbf, void *arg)
{
  REQUEST_IMPL  *req;
  int    i;

  req = request_new(Get, keys, nkeys, NULL, 0, 0, cbf, arg);
  req->command_len = 5; // "get" + "\r\n"
  for (i = 0; i < (int)nkeys; i++)
    req->command_len += strlen(keys[i]) + 1;

  req->command = (char*)gmalloc_data(req->command_len);
  strcpy(req->command, "get");
  req->command_len = 3;
  for (i = 0; i < (int)nkeys; i++)
  {
    req->command[req->command_len++] = ' ';
    strcpy(req->command + req->command_len, keys[i]);
    req->command_len += strlen(keys[i]);
  }
  req->command[req->command_len++] = '\r';
  req->command[req->command_len++] = '\n';

  req->input_cbf = request_get_input;
  req->data = NULL;
  req->response = NULL;

  stats_incr_cmdget_count();
  return (REQUEST *) req;
}


REQUEST *
request_new_set(const char *key, void *data, uint32_t bytes,
    uint32_t expiration, uint32_t flags, REQUEST_CBF cbf, void *arg)
{
  return request_new_storage(Set, "set", key, data, bytes,
        expiration, flags, cbf, arg);
}

REQUEST *
request_new_add(const char *key, void *data, uint32_t bytes,
    uint32_t expiration, uint32_t flags, REQUEST_CBF cbf, void *arg)
{
  return request_new_storage(Add, "add", key, data, bytes,
        expiration, flags, cbf, arg);
}

REQUEST *
request_new_replace(const char *key, void *data, uint32_t bytes,
    uint32_t expiration, uint32_t flags, REQUEST_CBF cbf, void *arg)
{
  return request_new_storage(Replace, "replace", key, data, bytes,
        expiration, flags, cbf, arg);
}

REQUEST *
request_new_delete(const char *key, uint32_t expiration, REQUEST_CBF cbf,
    void *arg)
{
  REQUEST_IMPL *req = request_new(Delete, &key, 1, NULL, 0,
          expiration, cbf, arg);
  char *format = "delete %s %u\r\n";
  req->command = (char*)gmalloc_data(strlen(key) + strlen(format) + 10);

  sprintf(req->command, format, key, expiration);
  return (REQUEST *) req;
}

REQUEST *
request_new_incr(const char *key, uint32_t amount, REQUEST_CBF cbf, void *arg)
{
  REQUEST_IMPL *req = request_new(Incr, &key, 1, NULL, 0,
          amount, cbf, arg);
  char *format = "incr %s %u\r\n";
  req->command = (char*)gmalloc_data(strlen(key) + strlen(format) + 10);

  sprintf(req->command, format, key, amount);
  return (REQUEST *) req;
}

REQUEST *
request_new_decr(const char *key, uint32_t amount, REQUEST_CBF cbf, void *arg)
{
  REQUEST_IMPL *req = request_new(Decr, &key, 1, NULL, 0,
          amount, cbf, arg);
  char *format = "decr %s %u\r\n";
  req->command = (char*)gmalloc_data(strlen(key) + strlen(format) + 10);

  sprintf(req->command, format, key, amount);
  return (REQUEST *) req;
}

REQUEST *
request_new_stats(REQUEST_CBF cbf, void *arg)
{
  return (REQUEST *) request_new(Stats, NULL, 0, NULL, 0, 0, cbf, arg);
}

REQUEST *
request_new_flush_all(uint32_t timer, REQUEST_CBF cbf, void *arg)
{
  REQUEST_IMPL *req = request_new(FlushAll, NULL, 0, NULL, 0,
          timer, cbf, arg);
  req->command = (char*)gmalloc_data(24);  /* flush_all xxxxxxxxxx\r\n\0 */
  if (timer)
    sprintf(req->command, "flush_all %u\r\n", timer);
  else
    strcpy(req->command, "flush_all\r\n");

  return (REQUEST *) req;
}

REQUEST *
request_new_version(REQUEST_CBF cbf, void *arg)
{
  return (REQUEST *) request_new(Version, NULL, 0, NULL, 0, 0, cbf, arg);
}

/*
 * Returns a new "marker" request. This is used to mark the end of a related
 * group of "get" requests in a client's request queue.
 */
REQUEST *
request_new_get_marker(REQUEST_CBF cbf, void *arg)
{
  REQUEST_IMPL *req = request_new(GetMarker, NULL, 0, NULL, 0, 0, cbf,
          arg);

  req->response = gstrdup_const("END");
  req->status = FinalResult;

  return (REQUEST *) req;
}

/*
 * Returns a new "marker" request. This is used to mark the end of a related
 * group of "flush_all" requests in the client's request queue.
 */
REQUEST *
request_new_flush_marker(REQUEST_CBF cbf, void *arg)
{
  REQUEST_IMPL *req = request_new(FlushMarker, NULL, 0, NULL, 0, 0, cbf,
          arg);

  req->response = gstrdup_const("OK");
  req->status = FinalResult;

  return (REQUEST *) req;
}

/*
 * Returns a new "quit" request. Once this reaches the head of the client's
 * request queue, we close the connection.
 */
REQUEST *
request_new_quit(REQUEST_CBF cbf, void *arg)
{
  REQUEST_IMPL *req = request_new(Quit, NULL, 0, NULL, 0, 0, cbf, arg);
  req->status = FinalResult;

  return (REQUEST *) req;
}

/*
 * Returns a new "error" request, which spits out an error message to the client
 * once we've finished processing its other requests.
 */
REQUEST *
request_new_error_marker(REQUEST_CBF cbf, char *response, void *arg)
{
  REQUEST_IMPL *req = request_new(ErrorMarker, NULL, 0, NULL, 0, 0, cbf,
          arg);
  req->status = FinalResult;
  req->response = gstrdup_const(response);

  return (REQUEST *) req;
}

REQUEST *
request_new_status_get(const char **keys, uint32_t nkeys) {
  REQUEST_IMPL *req = (REQUEST_IMPL *)request_new_get(keys, nkeys, NULL, NULL);
  int size;
  int i;
  char *response_data;
  int response_len;
  int is_status_key = 1;
  int free_response = 0;

  int status_key_prefix_len = strlen(STATUS_KEY_PREFIX);

  int cluster;

  req->status = FinalResult;
  req->data = NULL;
  req->total = 0;
  req->response = NULL;

  for (i=0; i < (int)nkeys; i++) {
    /* 6 for "VALUE ", 2 for "0 " (flags), 10 for size, 2 for \r\n */
    size = 6 + strlen(keys[i]) + 2 + 10 + 2;
    free_response = 0;

    if (strcmp(keys[i] + status_key_prefix_len, STATUS_KEY_CONFIG) == 0) {
      response_data = latest_config;

      response_len = latest_config_len;
    }
    else if (strcmp(keys[i] + status_key_prefix_len, STATUS_KEY_FILE) == 0) {
      if (latest_config_file == NULL)
        response_data = "<none>";
      else
        response_data = latest_config_file;

      response_len = strlen(response_data);
    }
    else if (strcmp(keys[i] + status_key_prefix_len, STATUS_KEY_SERVER) == 0) {
      if (latest_config_server == NULL)
        response_data = "<none>";
      else
        response_data = latest_config_server;

      response_len = strlen(response_data);
    }
    else if (strncmp(keys[i] + status_key_prefix_len, STATUS_KEY_CLUSTER, strlen(STATUS_KEY_CLUSTER)) == 0) {
      cluster = atoi(keys[i] + status_key_prefix_len + strlen(STATUS_KEY_CLUSTER));
      HPHP::Logger::Info("Retrieving status key %s cluster %d\n", keys[i], cluster);

      response_data = server_get_status_summary(cluster);

      response_len = strlen(response_data);

      free_response = 1;
    }
    else if (strcmp(keys[i] + status_key_prefix_len, STATUS_KEY_LAST_UPDATE) == 0) {
      response_data = ctime(&last_config_update_time);

      response_len = strlen(response_data);
    }
    else {
      is_status_key = 0;
    }

    if (is_status_key) {
      size += response_len + 2;
      req->data = grealloc(req->data, req->total + size);

      req->total += sprintf((char*)req->data+req->total, "VALUE %s 0 %d\r\n",
                            keys[i], response_len);

      memcpy((char*)req->data + req->total, response_data, response_len);
      req->total += response_len;
      memcpy((char*)req->data + req->total, "\r\n", 2);
      req->total += 2;
    }

    if (free_response) {
      gfree(response_data);
    }
  }

  return (REQUEST *) req;
}

/*
 * Returns a new asynchronous request. This type of request is not associated
 * with a client, but does require a response from the server before it can
 * be disposed.
 */
REQUEST *
request_new_async(REQUEST_CBF cbf, char *command, uint32_t offset, int reliable)
{
  REQUEST_IMPL *req = request_new(Async, NULL, 0, NULL, 0, offset, cbf,
          NULL);
  req->command = gstrdup_const(command);
  req->command_len = strlen(req->command);
  req->flags = reliable;

  return (REQUEST *) req;
}

/*
 * Returns a new asynchronous request with a payload.
 */
REQUEST *
request_new_async_data(REQUEST_CBF cbf, char *command, uint32_t offset,
      void *data, uint32_t bytes, int reliable)
{
  REQUEST_IMPL *req = request_new(Async, NULL, 0, NULL, 0, offset, cbf,
          NULL);
  req->command = gstrdup_const(command);
  req->command_len = strlen(req->command);
  req->data = gmemdup_const((const char *)data, bytes);
  req->bytes = bytes;
  req->flags = reliable;

  return (REQUEST *) req;
}


REQUEST_STATUS
request_get_status(const REQUEST *vreq)
{
  return ((const REQUEST_IMPL *) vreq)->status;
}

REQUEST_TYPE
request_get_type(const REQUEST *vreq)
{
  return ((const REQUEST_IMPL *) vreq)->type;
}


/*
 * Returns the raw response line for this request. For "get" requests, this
 * is just the initial information line; the data may be fetched using
 * request_get_data() and request_get_bytes().
 */
char *
request_get_response(const REQUEST *vreq)
{
  char * response = ((const REQUEST_IMPL *) vreq)->response;
  if (((REQUEST_IMPL *) vreq)->type == Get
    && !strncmp("VALUE", response, 5))
  {
    stats_incr_get_hits();
  }
  return response;
}


/*
 * Returns the binary data associated with this request or, in the case of
 * "get" requests, with the most recent response.
 */
void *
request_get_data(const REQUEST *vreq)
{
  return ((const REQUEST_IMPL *) vreq)->data;
}


/*
 * Returns the length of the binary data associated with this request or, in
 * the case of "get" requests, with the most recent response.
 */
uint32_t
request_get_bytes(const REQUEST *vreq)
{
  return ((const REQUEST_IMPL *) vreq)->bytes;
}

/*
 * Returns the total length of the response block (only meaningful for "get"
 * requests).
 */
uint32_t
request_get_total(const REQUEST *vreq)
{
  return ((const REQUEST_IMPL *) vreq)->total;
}


/*
 * Returns the command for this request. The command may include binary data,
 * so callers should also call request_get_command_len().
 */
void *
request_get_command(const REQUEST *vreq)
{
  return ((const REQUEST_IMPL *) vreq)->command;
}


/*
 * Returns the number of bytes in the command for this request.
 */
uint32_t
request_get_command_len(REQUEST *vreq)
{
  REQUEST_IMPL  *req = (REQUEST_IMPL *) vreq;

  if (req->command_len == 0)
    req->command_len = strlen((char *)request_get_command(vreq));
  return req->command_len;
}

/*
 * Returns the first key for this request.
 */
char *
request_get_first_key(const REQUEST *vreq)
{
  const REQUEST_IMPL  *req = (const REQUEST_IMPL *) vreq;

  if (req->nkeys == 0 || req->keys == NULL)
    return NULL;
  return req->keys[0];
}


/*
 * Returns the async store file offset for this request.
 */
uint32_t
request_get_offset(const REQUEST *vreq)
{
  const REQUEST_IMPL  *req = (const REQUEST_IMPL *) vreq;

  // We reuse the "expiration" field for this, since there is no
  // use for that field on an async request.
  if (req->type == Async)
    return req->expiration;
  return OFFSET_NONE;
}


/*
 * Returns the server for this request.
 */
MEMCACHED *
request_get_server(const REQUEST *vreq)
{
  return ((const REQUEST_IMPL *) vreq)->server;
}


/*
 * Returns true if this request's client is no longer connected.
 */
int
request_is_client_closed(REQUEST *vreq)
{
  return ((REQUEST_IMPL *) vreq)->client_closed;
}


/*
 * Marks this request as client-closed, so we won't try to send its response
 * back to a client. If there is no response (because it's an internal marker
 * request), frees the request.
 */
void
request_client_closed(REQUEST *vreq)
{
  if (request_is_internal(vreq))
    request_free(vreq);
  else
    ((REQUEST_IMPL *) vreq)->client_closed = 1;
}


/*
 * Returns true if this request is an internal placeholder.
 */
int
request_is_internal(const REQUEST *vreq)
{
  switch (request_get_type(vreq)) {
  case GetMarker:
  case FlushMarker:
  case Quit:
    return 1;

  default:
    return 0;
  }
}


/*
 * Sets the server this request has been sent to.
 */
void
request_set_server(REQUEST *vreq, MEMCACHED *server)
{
  ((REQUEST_IMPL *) vreq)->server = server;
}


/*
 * Returns true if an asynchronous request is reliable.
 */
int
request_is_reliable_async(const REQUEST *vreq)
{
  const REQUEST_IMPL *req = (REQUEST_IMPL *)vreq;

  if (request_get_type(vreq) == Async && req->flags)
    return 1;
  return 0;
}


/*
 * Handles a new input line from a server.
 */
int
request_handle_input(REQUEST *req, char *line)
{
  if (req != NULL)
    return (((REQUEST_IMPL *) req)->input_cbf)(req, line);
  return 0;
}


/*
 * Initializes the request-handling module.
 */
void
request_init()
{
  freelist = request_list_new();
}
