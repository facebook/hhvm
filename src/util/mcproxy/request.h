#ifndef __request_h__
#define __request_h__ 1

#include "include.h"

/*
 * Special keys
 */

#define STATUS_KEY_PREFIX "__mcproxy__."

#define STATUS_KEY_CONFIG "config"
#define STATUS_KEY_FILE "config-file"
#define STATUS_KEY_SERVER "config-server"
#define STATUS_KEY_CLUSTER "cluster-status."
#define STATUS_KEY_LAST_UPDATE "last-config-update"


/*
 * Request types.
 */
typedef enum {
  Get,
  Set,
  Add,
  Replace,
  Delete,
  Incr,
  Decr,
  Stats,
  FlushAll,
  Version,
  GetMarker,
  FlushMarker,
  Quit,
  ErrorMarker,
  Async
} REQUEST_TYPE;

/*
 * Request status codes (as supplied to REQUEST_CBF functions). This does
 * not indicate the status as returned by the remote server; it indicates
 * the progress of the request internally.
 */
typedef enum {
  Pending,
  PartialResult,
  FinalResult,
  Error
} REQUEST_STATUS;

/*
 * Incoming request descriptor. (Dummy for type safety.)
 */
typedef struct {
  char  dummy1[123];
  int  dummy2;
} REQUEST;

/*
 * List of requests. (Dummy for type safety.)
 */
typedef struct {
  long  dummy2;
  char  dummy1[93];
} REQUEST_LIST;


/*
 * Callbacks for request progress.
 */
typedef void (*REQUEST_CBF)(REQUEST *request, void *args);

/*
 * Request list management.
 */
extern REQUEST_LIST *request_list_new();
extern void request_list_free(REQUEST_LIST *list);
extern REQUEST *request_list_next(REQUEST_LIST *list);
extern REQUEST *request_list_peek(REQUEST_LIST *list);
extern REQUEST *request_list_peek_tail(REQUEST_LIST *list);
extern void request_list_add(REQUEST_LIST *list, REQUEST *request);
extern void request_list_delete(REQUEST_LIST *list, void *args);
extern void request_list_init();
/*
 * "Constructors" for different request types.
 */
extern REQUEST *request_new_get(const char **keys, uint32_t nkeys, REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_set(const char *key, void *data, uint32_t bytes, uint32_t expiration, uint32_t flags, REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_add(const char *key, void *data, uint32_t bytes, uint32_t expiration, uint32_t flags, REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_replace(const char *key, void *data, uint32_t bytes, uint32_t expiration, uint32_t flags, REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_delete(const char *key, uint32_t expiration, REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_incr(const char *key, uint32_t amount, REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_decr(const char *key, uint32_t amount, REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_stats(REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_flush_all(uint32_t timer, REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_version(REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_flush_marker(REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_get_marker(REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_quit(REQUEST_CBF cbf, void *args);
extern REQUEST *request_new_error_marker(REQUEST_CBF cbf, char *msg, void *arg);
extern REQUEST *request_new_async(REQUEST_CBF cbf, char *cmd, uint32_t offset, int reliable);
extern REQUEST *request_new_async_data(REQUEST_CBF cbf, char *cmd, uint32_t offset, void *data, uint32_t bytes, int reliable);
extern REQUEST *request_new_status_get(const char **keys, uint32_t nkeys);


extern void request_free(REQUEST *req);

/*
 * Getters for various details about a request.
 */
extern REQUEST_STATUS request_get_status(const REQUEST *req);
extern REQUEST_TYPE request_get_type(const REQUEST *req);
extern MEMCACHED *request_get_server(const REQUEST *req);
extern char *  request_get_response(const REQUEST *req);
extern void *  request_get_command(const REQUEST *req);
extern uint32_t  request_get_command_len(REQUEST *req);
extern void *  request_get_data(const REQUEST *req);
extern uint32_t  request_get_bytes(const REQUEST *req);
extern uint32_t  request_get_total(const REQUEST *req);
extern int  request_is_internal(const REQUEST *req);
extern int  request_is_client_closed(REQUEST *vreq);
extern void  request_client_closed(REQUEST *vreq);
extern char *  request_get_first_key(const REQUEST *req);
extern void  request_get_keys(const REQUEST *req, char **keys, uint32_t *nkeys);
extern uint32_t  request_get_offset(const REQUEST *req);
extern int      request_is_reliable_async(const REQUEST *vreq);

extern int  request_handle_input(REQUEST *req, char *line);
extern void  request_set_offset(REQUEST *req, uint32_t offset);

extern void  request_init();

#endif /* __request_h__ */
