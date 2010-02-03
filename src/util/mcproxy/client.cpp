
#include "proxy.h"
#include "server.h"
#include "stats.h"
#include "cluster.h"
#include "qio.h"
#include "qlio.h"
#include "gfuncs.h"

#define MISSING_ARG "CLIENT_ERROR Missing argument"
#define INTERNAL_ERROR "SERVER_ERROR Internal error"
#define CANNOT_SEND "SERVER_ERROR Can't send request to backing server"
#define KEY_TOO_LONG "CLIENT_ERROR key too long"
#define DEFERRED_SEND "NOT_FOUND"

#define NEXT_TOKEN() strtok(NULL, " \t\r\n")

#define MAX_KEY_LENGTH 250  /* Maximum allowed length of a key */
#define KEYS_AT_A_TIME 10  /* Typical maximum size of a multi-get */

#define LAST_ACTION_UNSET 2147483647

typedef struct _CLIENT CLIENT;
struct _CLIENT {
  int    fd;

  /*
   * Set to true if we have generated a response to the request that
   * is currently being parsed. (Gets reset on each new incoming request.)
   */
  int    responded;

  /*
   * The following are used to store the initial parameters from the
   * set/replace/add commands while waiting for binary data.
   */
  int    set_type;
  char    *key;
  int    flags;
  int    bytes;
  uint32_t  exptime;

  /*
   * Lists of requests we've sent out to the server but haven't
   * gotten responses for yet. This is in order of receipt from the
   * client.
   */
  REQUEST_LIST  *requests;
};

/*
 * CLIENT->set_type is one of these values if we're waiting for binary data
 * for a data-setting command.
 */
#define SET_TYPE_SET   1
#define SET_TYPE_ADD   2
#define SET_TYPE_REPLACE 3

int last_action = LAST_ACTION_UNSET; // will be rest
int parent_pipe_fd = -1;
int async_deletes = 0;

static void cmd_done(REQUEST *req, void *arg);


/*
 * Write a response string out to the client.
 */
static void send_response(CLIENT *client, char *str)
{
  int length = strlen(str);
  qio_enable_immediate(client->fd, 0);
  qio_write(client->fd, str, length);
  qio_write(client->fd, "\r\n", 2);
  qio_enable_immediate(client->fd, 1);
  client->responded++;
  stats_add_bytes_written(length + 2);
  HPHP::Logger::Verbose("%d -> %s", client->fd, str);
}

/*
 * Queue a response to send to the client. If there are no other requests
 * outstanding, send it immediately.
 */
static void respond(CLIENT *client, char *str)
{
  if (! client->responded)
  {
    REQUEST *req = request_new_error_marker(cmd_done, str, client);
    request_list_add(client->requests, req);
    if (request_list_peek(client->requests) == req)
    {
      cmd_done(req, client);
    }

    client->responded++;
  }
}

/*
 * Check if a set of keys is from a status request.
 * Just check first key, we treat entire request
 * as status.
 */

static int check_status_key(const char **keys, uint32_t nkeys)
{
  static const int status_key_prefix_len = sizeof(STATUS_KEY_PREFIX) - 1;

  if (nkeys == 0)
    return 0;

  return strncmp(keys[0], STATUS_KEY_PREFIX, status_key_prefix_len) == 0;
}

/*
 * Respond to a get with status data. Fill in the get data and
 * add it to the pending list, and call cmd_done.
 */
static void respond_status_get(CLIENT *client, const char **keys, uint32_t nkeys)
{
  REQUEST *req;

  req = request_new_status_get(keys, nkeys);

  request_list_add(client->requests, req);
  if (request_list_peek(client->requests) == req)
  {
    cmd_done(req, client);
  }
}

/*
 * Write an "internal error" response to the client.
 */
static void internal_error(CLIENT *client)
{
  stats_add_bytes_written(strlen(INTERNAL_ERROR));
  respond(client, INTERNAL_ERROR);
}


/*
 * Handle a server response. Since the client may pipeline requests to us and
 * the memcached protocol doesn't provide any means of matching requests with
 * responses, we will only send back responses in the order that the requests
 * were received. If we get a response for a request that's not the first one
 * in the list, we sit on it until we get a response to the first request.
 */
static void
cmd_done(REQUEST *req, void *arg)
{
  CLIENT  *client = (CLIENT *) arg;
  REQUEST *head;
  REQUEST_STATUS status;
  REQUEST_TYPE type;

  last_action = time(NULL);

  if (req != NULL)
  {
    type = request_get_type(req);

    /* If the client is gone, we're done. */
    if (request_is_client_closed(req))
    {
      request_free(req);
      return;
    }
  }

  while ((head = request_list_peek(client->requests)) != NULL &&
         (status = request_get_status(head)) != Pending)
  {
    if (status != PartialResult)
      request_list_next(client->requests);

    type = request_get_type(head);
    if (type == Get && status == FinalResult)
    {
      void *data = request_get_data(head);
      if (data != NULL)
      {
        qio_write(client->fd, data,
            request_get_total(head));
        stats_add_bytes_written(request_get_total(head));

        if (!strncmp("VALUE", (char *) data, 5))
        {
          stats_incr_get_hits();
        }
      }
    }
    else if (type == FlushAll)
    {
      char *resp = request_get_response(head);
      if (strcmp(resp, "OK"))
      {
        HPHP::Logger::Error("Got %s from flush_all", resp);
        send_response(client, resp);
      }
      // Otherwise wait for the marker
    }
    else if (type == Quit)
    {
      request_free(head);
      qio_close(client->fd);
      return;
    }
    else
    {
      send_response(client, request_get_response(head));
      client->responded = 0;
    }

    if (status != PartialResult)
      request_free(head);
  }
}


/*
 * Pulls all the keys out of the request, and figures out which server
 * each one belongs to. Orders the keys by server.
 */
static void
pull_keys_from_get_request(char *line, const char ***keys, MEMCACHED ***servers,
         uint32_t *nkeys)
{
  MEMCACHED  *mcd;
  const char  *key;
  int    i;

  *nkeys = 0;
  *servers = NULL;
  *keys = NULL;

  while ((key = NEXT_TOKEN()) != NULL)
  {
    mcd = server_for_key_uponly(key, cluster_id());

    /* Allocate more space for keys */
    if ((*nkeys % KEYS_AT_A_TIME) == 0)
    {
      *keys = (const char**)grealloc(*keys, sizeof(char *) *
                               (*nkeys + KEYS_AT_A_TIME));
      *servers = (MEMCACHED **)grealloc(*servers, sizeof(MEMCACHED *) *
                                        (*nkeys + KEYS_AT_A_TIME));
    }

    for (i = *nkeys; i > 0; i--)
    {
      if (mcd < (*servers)[i - 1])
        break;
      (*servers)[i] = (*servers)[i - 1];
      (*keys)[i] = (*keys)[i - 1];
    }

    (*servers)[i] = mcd;
    (*keys)[i] = key;

    (*nkeys)++;
  }
}


/*
 * Creates a group of "get" requests for a list of keys and servers.
 * Modifies the server list so that the n-th entry in the request list
 * is sent to the n-th entry in the server list.
 */
static REQUEST_LIST *
create_get_requests(MEMCACHED **servers, const char **keys, uint32_t nkeys,
        CLIENT *client)
{
  int    i;
  int    first_key = 0;
  int    cur_server = 0;
  REQUEST_LIST  *list = request_list_new();

  if (servers == NULL)
    return NULL;

  for (i = 0; i < (int)nkeys; i++)
  {
    if (servers[i] != servers[first_key])
    {
      request_list_add(list, request_new_get(keys + first_key,
                     i - first_key, cmd_done, client));
      servers[cur_server++] = servers[first_key];
      first_key = i;
    }
  }

  request_list_add(list, request_new_get(keys + first_key,
                 nkeys - first_key, cmd_done, client));
  servers[cur_server++] = servers[first_key];

  return list;
}


/*
 * Handle the "get" command.
 */
static void cmd_get(char *line, CLIENT *client)
{
  MEMCACHED  **servers = NULL;
  const char  **keys = NULL;
  uint32_t  nkeys = 0;
  REQUEST_LIST  *requests;
  REQUEST    *req;
  int    server_num = 0;

  pull_keys_from_get_request(line, &keys, &servers, &nkeys);
  /*
   * Wait to flush all data to client until all keys have been processed.
   * Otherwise TCP could delay the final END out.
   */
  qio_enable_immediate(client->fd, 0);
  if (check_status_key(keys, nkeys))
  {
    respond_status_get(client, keys, nkeys);

    gfree(servers);
    gfree(keys);
  }
  else if ((requests = create_get_requests(servers, keys, nkeys, client)) != NULL)
  {
    /*
     * Cycle through the list of requests, sending each one out.
     * If we get a failure sending a request, just ignore it; it will
     * be treated as a cache miss by the application, no harm done.
     */
    while ((req = request_list_next(requests)) != NULL)
      if (server_do_request(servers[server_num++], req) == 0)
      {
        request_list_add(client->requests, req);
      }
      else
      {
        HPHP::Logger::Verbose("discarding 'get' request after send failure");
        request_free(req);
      }

    gfree(servers);
    gfree(keys);
    request_list_free(requests);

  }

  /*
   * Add a dummy request that will cause us to send the "END" response
   * to the client once all the servers have responded.
   */
  request_list_add(client->requests,
       request_new_get_marker(cmd_done, client));

  /*
   * If we didn't actually send any requests (e.g. because there was
   * only one key and we got an error asking for it) send the "END"
   * right away.
   */
  cmd_done(NULL, client);
  qio_enable_immediate(client->fd, 1);
}

/*
 * Gets a mandatory token, spitting an error message out to the client
 * if the token isn't present.
 */
static char *get_token(CLIENT *client)
{
  char *token = NEXT_TOKEN();
  if (token == NULL && ! client->responded)
    respond(client, MISSING_ARG);

  return token;
}


/*
 * Sends a request which might be deferred
 * - send_now -- should we attempt to send the command before falling back to async
 * - replicate -- replicate this request?
 * - reliable -- should the request be retried. Applies both to replication and to the request itself
 */
static void send_deferrable(REQUEST *req, CLIENT *client, char *key, int send_now, int replicate, int reliable)
{
  int cluster = cluster_id();
  MEMCACHED *mcd = server_for_key_uponly(key, cluster);

  int send_error = 1;

  if (send_now)
  {
    send_error = server_do_request(mcd, req);
  }

  if (! send_error)
  {
    /*
     * Set quickack after sending data so the kernel doesn't
     * think we're an interactive connection and immediately
     * turn it off again.
     */
    server_set_quickack(mcd);
    request_list_add(client->requests, req);
  } else if (reliable || ! send_now)
  {
    /* do an async send if either 1) this is a reliable request and we failed
     * or 2) we didn't send the request at all
     */

    server_do_request_async(server_for_key(key, cluster), req, reliable);
    respond(client, DEFERRED_SEND);
  } else
  {
    respond(client, CANNOT_SEND);
  }

  if (replicate)
  {
    cluster_replicate(req, reliable);
  }


  if (send_error)
  {
    request_free(req);
  }
}

/*
 * Handle the "delete" command.
 */
static void cmd_delete(char *line, CLIENT *client)
{
  char *key = get_token(client);
  char *timer = NEXT_TOKEN();
  REQUEST *req;

  if (key == NULL)
    return;
  if (timer == NULL)
    timer = "0";

  req = request_new_delete(key, atoi(timer), cmd_done, client);
  stats_incr_cmddelete_count();
  send_deferrable(req, client, key, ! async_deletes, 1 /* replicate */, 1 /* reliable */);
}

/*
 * Do initial parsing and setup for the data-setting commands.
 */
static void setup_set(char *line, CLIENT *client, int set_type)
{
  char  *key = get_token(client);
  char  *flags = get_token(client);
  char  *exptime = get_token(client);
  char  *bytes = get_token(client);

  if (client->responded)  /* get_token spat out an error */
    return;

  if (gstrlen(key) > MAX_KEY_LENGTH)
  {
    respond(client, KEY_TOO_LONG);

    /*
     * We could eat the value here, but memcached doesn't, so
     * emulate its behavior. This means we may be interpreting
     * the value as commands.
     */
    return;
  }

  client->set_type = set_type;
  client->exptime = atoi(exptime);
  client->flags = atoi(flags);
  client->key = gstrdup(key);
  client->bytes = atoi(bytes);

  stats_incr_cmdset_count();

  /* Slurp up the binary data plus the \r\n at the end. */
  qlio_expect_binary(client->fd, client->bytes + 2);
}

/*
 * Binary data for a set/replace/add command has arrived. Process it.
 */
static void do_set(CLIENT *client, char *data)
{
  REQUEST *req = NULL;
  int replicate = client->flags & FLAG_REPLICATE;

  /*
   * Strip off the flag in case we're talking to a remote mcproxy;
   * otherwise we will potentially get into infinite replication loops.
   */
  client->flags &= ~FLAG_REPLICATE;

  data = (char*)gmemdup_const(data, client->bytes);

  switch (client->set_type)
  {
    case SET_TYPE_SET:
      req = request_new_set(client->key, data, client->bytes,
                client->exptime, client->flags,
                cmd_done, client);
      break;

    case SET_TYPE_ADD:
      req = request_new_add(client->key, data, client->bytes,
                client->exptime, client->flags,
                cmd_done, client);
      break;

    case SET_TYPE_REPLACE:
      req = request_new_replace(client->key, data, client->bytes,
              client->exptime, client->flags,
              cmd_done, client);
      break;

    default:
      HPHP::Logger::Error("Bad state! Set type is %d", client->set_type);
      internal_error(client);
      break;
  }

  if (req != NULL)
  {
    /*
     * Sets are replicated unreliably, since the data
     * volume is potentially too high to be able to fit
     * many of them in the async store.
     *
     * NOTE: Don't just change the "0" to a "1" if you
     * want to make set replication reliable! The async
     * data store code will probably need to be changed
     * to more cleanly support large request sizes,
     * since it currently assumes requests are
     * relatively small.
     */
    int can_defer = client->set_type == SET_TYPE_SET && (client->flags & FLAG_ASYNC_SET);

    send_deferrable(req, client, client->key, !can_defer, replicate, 0 /* not reliable */);
  }


  gfree(client->key);
  client->key = NULL;
}


/*
 * Handle the "set" command.
 */
static void cmd_set(char *line, CLIENT *client)
{
  setup_set(line, client, SET_TYPE_SET);
}

/*
 * Handle the "replace" command.
 */
static void cmd_replace(char *line, CLIENT *client)
{
  setup_set(line, client, SET_TYPE_REPLACE);
}

/*
 * Handle the "add" command.
 */
static void cmd_add(char *line, CLIENT *client)
{
  setup_set(line, client, SET_TYPE_ADD);
}

/*
 * Handle the "incr" command.
 */
static void cmd_incr(char *line, CLIENT *client)
{
  REQUEST *req;
  char *key = get_token(client);
  char *value = get_token(client);
  int amount;

  if (key == NULL || value == NULL)
    return;

  amount = atoi(value);

  req = request_new_incr(key, amount, cmd_done, client);
  send_deferrable(req, client, key, ! async_deletes, 1 /* replicate */, 1 /* reliable */);
}

/*
 * Handle the "decr" command.
 */
static void cmd_decr(char *line, CLIENT *client)
{
  REQUEST *req;
  char *key = get_token(client);
  char *value = get_token(client);
  int amount;

  if (key == NULL || value == NULL)
    return;

  amount = atoi(value);

  req = request_new_decr(key, amount, cmd_done, client);
  send_deferrable(req, client, key, ! async_deletes, 1 /* replicate */, 1 /* reliable */);
}

/*
 * Handle the "stats" command.
 */
static void cmd_stats(char *line, CLIENT *client)
{
  stats_dump(client->fd);
}

/*
 * Handle the "which_server" command.
 */
static void cmd_which_server(char *line, CLIENT *client)
{
  char *key = get_token(client);
  MEMCACHED *mcd = server_for_key(key, cluster_id());
  respond(client, server_get_description(mcd));
}

/*
 * Handle the "flush_all" command.
 */
static void cmd_flush_all(char *line, CLIENT *client)
{
  char *timer_tok = NEXT_TOKEN();
  int timer = (timer_tok == NULL) ? 0 : atoi(timer_tok);
  MEMCACHED *mcd;
  REQUEST *req;
  int i;

  for (i = 0; (mcd = server_number(i)) != NULL; i++)
  {
    req = request_new_flush_all(timer, cmd_done, client);
    if (server_do_request(mcd, req) == 0)
      request_list_add(client->requests, req);
    else
    {
      respond(client, CANNOT_SEND);
      request_free(req);
    }
  }

  request_list_add(client->requests,
       request_new_flush_marker(cmd_done, client));
}

/*
 * Handle the "quit" command.
 */
static void cmd_quit(char *line, CLIENT *client)
{
  request_list_add(client->requests,
       request_new_quit(cmd_done, client));
  cmd_done(NULL, client);
}

/*
 * Handle the "version" command.
 */
static void cmd_version(char *line, CLIENT *client)
{
  respond(client, "VERSION 1.0.2 (proxy)");
}


struct Command {
  char  *cmd;
  int  len;
  void  (*func)(char *, CLIENT *);
} commands[] = {
  {"get",      3, cmd_get},
  {"set",      3, cmd_set},
  {"add",      3, cmd_add},
  {"replace",    7, cmd_replace},
  {"delete",    6, cmd_delete},
  {"incr",    4, cmd_incr},
  {"decr",    4, cmd_decr},
  {"stats",    5, cmd_stats},
  {"flush_all",    9, cmd_flush_all},
  {"version",    7, cmd_version},
  {"which_server",  12, cmd_which_server},
  {"quit",    4, cmd_quit}
};

/*
 * New input has arrived from a client.
 */
void handle_input(int fd, char *line, void *args)
{
  CLIENT *client = (CLIENT *)args;
  int  i;

  last_action = time(NULL);
  if (client->set_type > 0)
  {
    HPHP::Logger::Verbose("binary input from %d", fd);
    do_set(client, line);
    client->set_type = 0;
  }
  else
  {
    HPHP::Logger::Verbose("input from %d: %s", fd, line);
    if (line == NULL)
      return;

    if (! strncmp(line, "terminate", 9))
    {
      if (parent_pipe_fd > -1)
        write(parent_pipe_fd, "q", 1);
      throw HPHP::Exception("exit with %d", 0);
    }

#ifdef FIND_LEAKS
    if (! strncmp(line, "leaks", 5))
    {
      GC_gcollect();
    }
#endif

    stats_add_bytes_read(strlen(line));
    strtok(line, " \r\n\t");
    for (i = 0; i < (int)(sizeof(commands) / sizeof(commands[0])); i++)
      if (! strcmp(line, commands[i].cmd))
      {
        client->responded = 0;
        (commands[i].func)(line, client);
        return;
      }

    respond(client, "ERROR");
    cmd_done(NULL, client);
  }
}

/*
 * A client connection has closed. Free up any resources allocated to it.
 */
void handle_close(int fd, int code, char *line, void *args)
{
  CLIENT *client = (CLIENT *)args;
  REQUEST *req;

  HPHP::Logger::Verbose("client fd %d closed, line %s", fd, line);

  /*
   * Make sure we don't try to reply to any of the client's pending
   * requests when the server answers.
   */
  while ((req = request_list_next(client->requests)) != NULL)
    request_client_closed(req);

  if (client->key != NULL)
    gfree(client->key);
  request_list_free(client->requests);
  gfree(client);

  stats_incr_closed_client_connections();
#ifdef FIND_LEAKS
  CHECK_LEAKS();
#endif
}


/*
 * A new connection request is coming in on the listen socket. Accept it
 * and listen for requests on the newly connected socket.
 */
void handle_connection(int fd, void *args)
{
  CLIENT *client;
  int  new_fd;

  new_fd = accept(fd, NULL, NULL);
  if (new_fd < 0)
  {
    HPHP::Logger::Error("Can't accept connection");
    stats_incr_failed_client_connections();
    return;
  }

  HPHP::Logger::Verbose("New connection on fd %d", new_fd);
  client = (CLIENT*)gcalloc(sizeof(CLIENT));
  if (client == NULL)
  {
    HPHP::Logger::Error("Can't allocate client descriptor");
    close(new_fd);
    stats_incr_failed_client_connections();
    return;
  }

  client->requests = request_list_new();
  client->fd = new_fd;
  if (set_nodelay(new_fd, 1))
    HPHP::Logger::Error("Can't set socket to NODELAY");
  stats_incr_successful_client_connections();
  qlio_add(new_fd, handle_input, handle_close, client);
}

/*
 * Adds the connection listen handler.
 */
void
client_add_handler(int fd)
{
  qio_add(fd, handle_connection, NULL, NULL);
}
