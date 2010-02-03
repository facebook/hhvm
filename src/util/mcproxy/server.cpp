/*
 * Server management.
 */
#include "proxy.h"
#include "cluster.h"
#include "server_int.h"
#include "request_int.h"
#include "gfuncs.h"
#include "qio.h"
#include "qlio.h"
#include "crc32.h"

extern int last_action;

/* intentionally global, set from main */
int server_timeout_msec = 1000;

/* Global list of servers, sorted by IP address and port */
static uint32_t nservers;
static uint32_t nservers_real;
static uint32_t nservers_up;
static MEMCACHED_IMPL **servers = NULL;

static void server_try_connect(void *arg);
static void server_schedule_reconnect(MEMCACHED_IMPL *mcd);

/*
 * Returns true if a server is able to accept requests.
 */
int
server_is_up(MEMCACHED *mcd)
{
    return mcd != NULL && ((MEMCACHED_IMPL *)mcd)->status == Connected;
}


int
server_is_real(MEMCACHED *mcd)
{
    return (mcd != NULL &&
        ((MEMCACHED_IMPL *)mcd)->port >= 0 &&
        ((MEMCACHED_IMPL *)mcd)->port <= 65535);
}


/*
 * Compares a MEMCACHED with a host/port pair.
 */
static int
mcdcmp(MEMCACHED_IMPL *mcd, const char *hostaddr, const int port, const int cluster)
{
    int cmp;

    cmp = strcmp(mcd->hostaddr, hostaddr);
    if (cmp != 0)
        return cmp;
    if (mcd->port != port)
        return mcd->port - port;
    return mcd->cluster - cluster;
}

/*
 * Compares two MEMCACHEDs to see if they point to the same server.
 */
int
server_is_same(MEMCACHED *mc1, MEMCACHED *mc2)
{
    MEMCACHED_IMPL *mcd1 = (MEMCACHED_IMPL *) mc1;
    MEMCACHED_IMPL *mcd2 = (MEMCACHED_IMPL *) mc2;

    if (mcd1->port != mcd2->port)
        return 0;
    return strcmp(mcd1->hostaddr, mcd2->hostaddr) == 0;
}


/*
 * Looks up a server by textual IP address, port, and cluster. Finds the
 * position in the list where the server should be found. Returns nonzero
 * if we the server was in the list.
 */
static int
server_find_pos(const char *hostaddr, const int port, const int cluster,
                int *pos)
{
    /*
     * Since the number of servers is expected to remain fairly small,
     * this is a simple binary search. If the server count gets high,
     * change this to a hashtable.
     */
    int max = nservers - 1, min = 0, middle = 0;
    int cmp = 0;

    while (max >= min)
    {
        middle = (max + min) / 2;
        cmp = mcdcmp(servers[middle], hostaddr, port, cluster);

        if (! cmp)
        {
            *pos = middle;
            return 1;
        }
        else if (cmp < 0)
            min = middle + 1;
        else
            max = middle - 1;
    }

    /*
     * If the current entry is less than the one we're looking for,
     * we'd want to find the server in question in the next list entry.
     */
    if (cmp < 0)
        *pos = middle + 1;
    else
        *pos = middle;
    return 0;
}

/*
 * Looks up a server by textual IP address, port, and cluster.
 */
MEMCACHED *
server_find(const char *hostaddr, const int port, const int cluster)
{
    int pos;

    if (server_find_pos(hostaddr, port, cluster, &pos))
        return (MEMCACHED *) servers[pos];
    return NULL;
}


/*
 * Adds a new server to the list.
 */
static void
server_add(MEMCACHED_IMPL *mcd)
{
    int pos;

    if (server_find_pos(mcd->hostaddr, mcd->port, mcd->cluster, &pos))
        return;

    servers = (MEMCACHED_IMPL**)grealloc(servers, sizeof(MEMCACHED_IMPL *) * (nservers + 1));
    if (pos < (int)nservers)
        memmove(servers + pos + 1, servers + pos,
            sizeof(MEMCACHED_IMPL *) * (nservers - pos));

    servers[pos] = mcd;
    nservers++;
        if (server_is_real((MEMCACHED*) mcd))
                nservers_real++;
}

static void server_timeout(void* arg) {
  MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *)arg;
  mcd->timeout_set = 0;
  HPHP::Logger::Info("Server %s got a timeout while sending a request", mcd->description);
  qio_close(mcd->fd);
}

/*
 * Input is available from a server.
 */
static void
server_read(int fd, char *line, void *args)
{
    MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) args;

    /*
     * Is this a response to an asynchronous request that has fallen off
     * the end of our request queue? Ignore it if so.
     */
    if (mcd->pending_async_drop_count)
    {
        mcd->pending_async_drop_count--;
        goto handle_timeout;
    }

    /* Is this the end of the response to the request? */
    if (request_handle_input(request_list_peek(mcd->request_list),
                line) == 0)
    {
        /* Yes. Move on to the next one. */
        request_list_next(mcd->request_list);
    }

 handle_timeout:
    /*
     * While any request is pending to the server, we ensure that the
     * server sends us some sort of data at least once per second. In theory,
     * a server could be evil and spoon feed us one byte at a time. However,
     * we're assuming memcached isn't being completely evil.
     */
    if (mcd->timeout_set) {
        qio_cancel(server_timeout, mcd);
        mcd->timeout_set = 0;
        /* more requests are pending so we need another timeout. */
        if (request_list_peek(mcd->request_list)) {
            qio_timeout_msec(server_timeout_msec, server_timeout, mcd);
            mcd->timeout_set = 1;
        }
    }
}


/*
 * A server connection attempt has failed.
 */
static void
server_connect_failed(int fd, void *args)
{
    MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) args;

    if (mcd->status != Connecting)
    {
        HPHP::Logger::Error("Connect failed while not in Connecting state");
        throw HPHP::Exception("abort");
    }

    HPHP::Logger::Error("Server %s connection failed", mcd->description);
    qio_close(fd);

    mcd->fd = -1;
    server_schedule_reconnect(mcd);
}


/*
 * Fail all the requests from clients that are pending on a server. Any
 * pending asynchronous requests are requeued so they'll be sent when the
 * server comes back online.
 */
static void
server_fail_client_requests(MEMCACHED_IMPL *mcd)
{
    REQUEST     *tail = request_list_peek_tail(mcd->request_list);
    REQUEST     *req;

    if (tail != NULL)
    {
        do {
            req = request_list_next(mcd->request_list);
            if (NULL == req)
                break;
            if (request_is_reliable_async(req)) {
                /*
                 * Requeue the request in the async queue,
                 * since it will have been marked as handled
                 * in the persistent store.
                 */
                server_do_request_async(
                        (MEMCACHED *)mcd, req, 1);
                request_free(req);
            }
            else
                request_handle_input(req,
                    "SERVER_ERROR Backing server lost");
        } while (req != tail);
    }
}

/*
 * A server connection has closed. Fail all its pending requests and start
 * trying to reconnect. (QIO callback)
 */
static void
server_close(int fd, int code, char *line, void *args)
{
    MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) args;

    if (mcd->status != Connecting)
    {
        uint32_t i;

        for (i = 0; i < mcd->nprefixes; i++)
            prefix_server_down(mcd->prefixes[i], (MEMCACHED *) mcd);
        nservers_up--;
        HPHP::Logger::Info("Server %s closed (%d of %d up)", mcd->description,
            nservers_up, nservers);
    }

    if (code != QIO_CLOSE_LOCAL)
        close(fd);

    mcd->timeout_set = 0;
    qio_cancel(server_timeout, mcd);

    mcd->fd = -1;
    mcd->last_down_time = time(NULL);

    server_schedule_reconnect(mcd); // server status is reset here

    server_fail_client_requests(mcd);
}


/*
 * A socket connection attempt has finished.
 */
static int
server_connected(int fd, void *args)
{
    MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) args;
    uint32_t i;
    int v;

    for (i = 0; i < mcd->nprefixes; i++)
        prefix_server_up(mcd->prefixes[i], (MEMCACHED *) mcd);
    nservers_up++;
    mcd->status = Connected;
    set_nodelay(mcd->fd, 1);

    /* Make the connection time out aggressively so we don't leave
       clients waiting for dead servers. */

#ifdef SO_KEEPALIVE
    v = 1;
    if (setsockopt(mcd->fd, SOL_SOCKET, SO_KEEPALIVE, &v, sizeof(v)))
    {
        HPHP::Logger::Error("Can't set keepalive on socket");
    }

# ifdef TCP_KEEPCNT
    /* 3 keepalive probes lost before dropping connection */
    v = 3;
    if (setsockopt(mcd->fd, SOL_TCP, TCP_KEEPCNT, &v, sizeof(v)))
    {
        HPHP::Logger::Error("Can't set keepalive count on socket");
    }
# endif

# ifdef TCP_KEEPIDLE
    /* Start sending keepalives after 30 seconds of idle time */
    v = 30;
    if (setsockopt(mcd->fd, SOL_TCP, TCP_KEEPIDLE, &v, sizeof(v)))
    {
        HPHP::Logger::Error("Can't set keepalive idle time on socket");
    }
# endif

# ifdef TCP_KEEPINTVL
    /* 10 seconds between keepalive probes */
    v = 10;
    if (setsockopt(mcd->fd, SOL_TCP, TCP_KEEPINTVL, &v, sizeof(v)))
    {
        HPHP::Logger::Error("Can't set keepalive interval on socket");
    }

# endif
#endif /* SO_KEEPALIVE */

    HPHP::Logger::Info("Server %s (%d of %d) connected", mcd->description,
        nservers_up, nservers_real);

    /* Let QIO handle write buffering for this connection. */
    qio_remove(fd);
    qlio_add(fd, server_read, server_close, args);

    /* New connection, so there are no pending requests yet. */
    mcd->pending_async_drop_count = 0;

    /* If there are queued commands for this server, send them. */
    server_requeue_requests(mcd);

    return 0;
}


/*
 * Tries to connect to a server.
 */
static void
server_try_connect(void *arg)
{
    int status;
    MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) arg;
    time_t now = time(NULL);

    // if we've spent at least 60 seconds without input, do not try to
    // reconnect.
    if (now - last_action >= 60)
    {
        server_schedule_reconnect(mcd);
        return;
    }

    if (mcd->fd >= 0)
    {
        // This should be unreachable. server_try_connect only called on
        // schedule and when status is Unitialized
        HPHP::Logger::Info("Connection attempt already pending to %s. Not connecting. mcd->status: %d",
              mcd->description, mcd->status);
        return;
    }

    mcd->last_connect_time = now;
    status = clientsock_nb(mcd->hostaddr, mcd->port, &mcd->fd);
    if (status > 0)
    {
        HPHP::Logger::Verbose("Connecting to %s", mcd->description);
        mcd->status = Connecting;
        qio_add_write(mcd->fd, server_connect_failed, server_connected,
                NULL, mcd);
    }
    else if (status == 0)
    {
        server_connected(mcd->fd, mcd);
    }
    else
    {
        mcd->fd = -1;
        HPHP::Logger::Error("Connect to %s failed to start", mcd->description);
        server_schedule_reconnect(mcd);
    }
}


/*
 * Schedules a reconnect attempt to a server after some amount of time.
 */
static void
server_schedule_reconnect(MEMCACHED_IMPL *mcd)
{
    qio_schedule(time(NULL) + CONNECT_RETRY_INTERVAL,
            server_try_connect, mcd, 0);
    mcd->status = SleepingBetweenRetries;
}


/*
 * Returns a server definition, allocating a new one if needed. Starts trying
 * to connect to the server.
 */
MEMCACHED *
server_new(const char *hostname, const int port, const int cluster)
{
    MEMCACHED_IMPL  *mcd;
    char        addr[100];      /* big enough for IPv6 */

    host_to_addr(hostname, addr);

    if (addr[0] == '\0')
    {
        HPHP::Logger::Error(hostname);
        return NULL;
    }

    mcd = (MEMCACHED_IMPL *)server_find(addr, port, cluster);
    if (mcd == NULL)
    {
        mcd = (MEMCACHED_IMPL*)gcalloc(sizeof(MEMCACHED_IMPL));
        mcd->hostaddr = gstrdup(addr);
        mcd->hostname = gstrdup(hostname);
        mcd->port = port;
        mcd->status = Uninitialized;
        mcd->fd = -1;
        mcd->store_fd = -1;
        mcd->request_list = request_list_new();
        mcd->prefixes = NULL;
        mcd->nprefixes = 0;
        mcd->cluster = cluster;
        mcd->queued_async_size = 0;
        mcd->pending_async_drop_count = 0;

        sprintf(addr, "%s(%s):%d", hostname, mcd->hostaddr, port);
        mcd->description = gstrdup(addr);

        server_add(mcd);

        HPHP::Logger::Verbose("Added new server %s", mcd->description);
        server_asyncstore_init(mcd);
    }

    if (! server_is_real((MEMCACHED*) mcd)) {
        mcd->status = Dummy;
    }

    return (MEMCACHED *) mcd;
}


/*
 * Returns the number of known servers.
 */
int
server_count()
{
    return nservers;
}


/*
 * Returns a particular server from the list of servers.
 */
MEMCACHED *
server_number(int num)
{
  if (num < 0 || num >= (int)nservers)
        return NULL;
    return (MEMCACHED *) servers[num];
}


/**
 * Returns the server for a particular key.
 */
MEMCACHED *
server_for_key(const char *key, const int cluster)
{
  uint32_t hash;
  uint32_t nmcds;
  MEMCACHED_IMPL **mcds;
  uint32_t num;
  const CLUSTER_SERVERS * const cluster_servers = prefix_get_cluster_servers_for_key(key,
                         cluster);

        // We may not have any cluster servers yet if we are configuring/
  // re-configuring.
  if(cluster_servers == NULL) {
    return NULL;
  }

  /* Look up the servers that can serve the key. */
  prefix_get_servers_for_cluster_servers(cluster_servers,
                 (MEMCACHED***)&mcds, &nmcds);

  if (0 == nmcds)
    return NULL;

  if (cluster_servers->use_consistent_hashing)
  {
    // alas, we have to cast away const because the rest of the API
    // doesn't declare it as so.
    continuum_lookup_res_t retval;

    retval = continuum_lookup(key, strlen(key), nmcds);
    if (retval.error_code != 0)
    {
      return NULL;
    }
    num = retval.server_num;
  }
  else
  {
    hash = crc32(key, strlen(key)) & 0x7fffffff;
    num = hash % nmcds;
  }
  return (MEMCACHED *) mcds[num];
}


/**
 * Returns the server for a particular key if it's up.
 */
MEMCACHED *
server_for_key_uponly(const char *key, const int cluster)
{
    MEMCACHED *server = server_for_key(key, cluster);
    time_t now;
    time_t last_connect_time;

    if (server_is_up(server))
        return server;

    HPHP::Logger::Verbose("Server is down");

    last_connect_time = server_get_last_connect_time(server);

    now = time(NULL);

    if(last_connect_time != 0 &&
       now - last_connect_time > 60) {
        MEMCACHED_IMPL *mcd = (MEMCACHED_IMPL *)server;
        HPHP::Logger::Info("Server %s is down and hasn't attempted to reconnect in %ds.",
             mcd->description,
             now - last_connect_time);

        /* 0 last-connect-time so we don't keep logging this. */
        mcd->last_connect_time = 0;
    }
    return NULL;
}

/*
 * Sends a request to the server(s) that can serve it.
 *
 * Returns:
 *   0 Request has been sent.
 *  -1 Server wasn't able to accept request.
 */
int
server_do_request(MEMCACHED *vmcd, REQUEST *req)
{
    MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) vmcd;
    void        *data;

    if (vmcd && !server_is_real(vmcd)) {
        HPHP::Logger::Error("Executing request with a dummy server!");
        return -1;
    }

    if (server_is_up(vmcd))
    {
        request_set_server(req, vmcd);
        request_list_add(mcd->request_list, req);
        qio_enable_immediate(mcd->fd, 0);
        qio_write(mcd->fd, request_get_command(req),
            request_get_command_len(req));
        data = request_get_data(req);
        if (data != NULL)
        {
            qio_write(mcd->fd, data, request_get_bytes(req));
            qio_write(mcd->fd, "\r\n", 2);
        }
        qio_enable_immediate(mcd->fd, 1);

        if (!mcd->timeout_set) {
          qio_timeout_msec(server_timeout_msec, server_timeout, mcd);
          mcd->timeout_set = 1;
        }

        return 0;
    }

    return -1;
}


/*
 * Expects some number of bytes from a server connection.
 */
void
server_expect_bytes(MEMCACHED *vmcd, uint32_t bytes)
{
    MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) vmcd;

    if (mcd->fd != -1)
        qlio_expect_binary(mcd->fd, bytes);
}


/*
 * Adds a prefix to the server's list of prefixes.
 */
void
server_add_prefix(MEMCACHED *vmcd, PREFIX *prefix)
{
    MEMCACHED_IMPL  *mcd = (MEMCACHED_IMPL *) vmcd;

    mcd->prefixes = (PREFIX**)grealloc(mcd->prefixes,
                sizeof(PREFIX *) * (mcd->nprefixes + 1));
    mcd->prefixes[mcd->nprefixes++] = prefix;
}


/*
 * Clears the prefixes from all the servers in preparation for a config reload.
 */
void
server_clear_prefixes()
{
    unsigned int i;

    for (i = 0; i < nservers; i++)
    {
        gfree(servers[i]->prefixes);
        servers[i]->prefixes = NULL;
        servers[i]->nprefixes = 0;
    }
}


/*
 * Starts trying to connect to all the servers that don't already have
 * connections or pending connection attempts.
 */
void
server_connect_all(void)
{
    unsigned int i;

    for (i = 0; i < nservers; i++)
        if (servers[i]->status == Uninitialized)
            server_try_connect(servers[i]);
}

static inline char *server_get_status_string(MEMCACHED_STATUS status) {
   switch (status) {
    case Uninitialized:
      return "Uninitialized";
    case Connecting:
      return "Connecting";
    case Connected:
      return "Connected";
    case SleepingBetweenRetries:
      return "SleepingBetweenRetries";
    case Dummy:
      return "Dummy";
    default:
      return "Unknown";

  }
}


char *
server_get_status_summary(int cluster) {
  unsigned int i;

  char *str = NULL;
  char *status;
  int len;
  int size = 0;

  for (i=0; i < nservers; i++) {
    if (servers[i]->cluster == cluster) {
      len = strlen(servers[i]->description);
      status = server_get_status_string(servers[i]->status);
      len += strlen(status);

      // space and '\n' and '\0'
      len += 3;

      str = (char*)grealloc(str, size+len);

      size += sprintf(str + size, "%s %s\n", servers[i]->description, status);
    }
  }

  return str;
}


void
server_disconnect(MEMCACHED *vmcd)
{
    MEMCACHED_IMPL *mcd = (MEMCACHED_IMPL *) vmcd;

    if (mcd->fd >= 0)
        qio_close(mcd->fd);
}


void
server_disconnect_all(void)
{
    unsigned int     i;
    MEMCACHED_IMPL* mcd;
    for (i = 0; i < nservers; i++)
    {
        mcd = (MEMCACHED_IMPL*) servers[i];
        qio_close(mcd->fd);
    }
}

double
server_average_queue_length()
{
    unsigned int i;
    int totalLength = 0;
    for (i = 0; i < nservers; i++)
    {
        MEMCACHED_IMPL * mcdi = servers[i];
        REQUEST_LIST_IMPL * listi = (REQUEST_LIST_IMPL*) mcdi->request_list;

        totalLength += (int) listi->count;
    }
    return ((double) totalLength) / nservers;
}

int server_get_cluster(MEMCACHED *server)
{
    if (server == NULL)
        return -1;
    return ((MEMCACHED_IMPL *)server)->cluster;
}

char *server_get_description(MEMCACHED *server)
{
    if (server == NULL)
        return "<null>";
    return ((MEMCACHED_IMPL *)server)->description;
}

time_t server_get_last_down_time(MEMCACHED *server)
{
    if (server == NULL)
        return 0;
    return ((MEMCACHED_IMPL *)server)->last_down_time;
}

time_t server_get_last_connect_time(MEMCACHED *server)
{
    if (server == NULL)
        return 0;
    return ((MEMCACHED_IMPL *)server)->last_connect_time;
}

/*
 * Sets the TCP "quick ack" option on the server connection, if available.
 */
void server_set_quickack(MEMCACHED *vmcd)
{
#ifdef TCP_QUICKACK
    MEMCACHED_IMPL *mcd = (MEMCACHED_IMPL *)vmcd;
    if (mcd != NULL && mcd->fd >= 0)
    {
        int one = 1;
        if (setsockopt(mcd->fd, SOL_TCP, TCP_QUICKACK,
                       &one, sizeof(one)))
        {
            HPHP::Logger::Error("Can't set quickack on %s",
                server_get_description(vmcd));
        }
    }
#endif
}
