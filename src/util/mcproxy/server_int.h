/*
 * Internal declarations for server management module.
 */
#include "include.h"
#include "prefix.h"

#define CONNECT_RETRY_INTERVAL 10

/*
 * Server status values.
 */
typedef enum {
    Uninitialized,
    Connecting,
    Connected,
    SleepingBetweenRetries,
    Dummy,
} MEMCACHED_STATUS;

/*
 * Memcached server description.
 */
typedef struct {
    /* List of requests that have been sent to the server.  */
    REQUEST_LIST    *request_list;

    /* Total size of outstanding async request bodies. */
    uint32_t    queued_async_size;

    /* Total number of outstanding async requests we've dropped. */
    uint32_t    pending_async_drop_count;

    /* Which cluster this server is a member of. */
    int     cluster;

    /* Hostname as supplied in configuration. */
    char        *hostname;

    /* Host address in string format. */
    char        *hostaddr;

    /* Description (hostname and port). */
    char        *description;

    /* Port number. */
    int     port;

    /* File descriptor of socket to server. */
    int     fd;

    /* Path of asynchronous request store file. */
    char        *store_path;

    /* File descriptor of persistent asynchronous request store. */
    int     store_fd;

    /* Current offset in the asynchronous request store. */
    uint32_t    store_offset;

    /* Server status. */
    MEMCACHED_STATUS status;

    /*
     * 0 if server is up, otherwise the most recent time it was
     * reported (or observed) to be down.
     */
    time_t      last_down_time;
    time_t      last_connect_time;

    /* List of prefixes this server handles. */
    PREFIX      **prefixes;
    uint32_t    nprefixes;

    /* Did we set a timeout in qio for this server */
    int timeout_set;
} MEMCACHED_IMPL;

void server_requeue_requests(MEMCACHED_IMPL *mcd);
void server_asyncstore_init(MEMCACHED_IMPL *mcd);

// indicates whether or not the server belongs to a pool using consistent hashing.
static inline int is_consistent_server(MEMCACHED *_server)
{
    MEMCACHED_IMPL *server = (MEMCACHED_IMPL *) _server;

    return (server->port % 2 == 1);
}
