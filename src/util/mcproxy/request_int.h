/*
 * Internal declarations for request parsing module.
 */
#include "include.h"
#include "request.h"

typedef int (*REQUEST_INPUT_CBF)(REQUEST *req, char *input);

/*
 * Outgoing request descriptor.
 */
typedef struct _request REQUEST_IMPL;
struct _request {
  /* Which server this request was sent to. */
  MEMCACHED  *server;

  REQUEST_TYPE  type;

  /* Callback to call to handle input for this request. */
  REQUEST_INPUT_CBF input_cbf;

  /* Command to send to server, possibly including binary data. */
  char    *command;
  uint32_t  command_len;

  /* Callback function and caller-supplied opaque argument. */
  REQUEST_CBF  cbf;
  void    *arg;

  /* List of keys associated with this request. */
  char    **keys;
  uint32_t  nkeys;
  uint32_t  nkeys_alloced;

  /* Data associated with this request. */
  void    *data;
  uint32_t  bytes;
  uint32_t  total;    // total length, for multiget responses
  uint32_t  flags;

  /* Most recent server response. */
  char    *response;

  /* Expiration time for this request (Delete, Set/Add/Replace). */
  uint32_t  expiration;

  /* Internal status. */
  REQUEST_STATUS  status;

  /* True if this request's response should be ignored. */
  int    ignore;

  /* True if this request's client is no longer connected. */
  int    client_closed;
};


/*
 * List entry for a request. Since a request can appear in multiple servers'
 * request lists, we can't just put a "next" link in REQUEST itself.
 */
typedef struct _request_entry REQUEST_ENTRY;
struct _request_entry {
  REQUEST_IMPL  *request;
  REQUEST_ENTRY  *next;
};

/*
 * List of requests.
 */
typedef struct _request_list REQUEST_LIST_IMPL;
struct _request_list {
  REQUEST_ENTRY  *head;
  REQUEST_ENTRY  *tail;
  uint32_t  count;
};

