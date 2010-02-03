#ifndef __server_h__
#define __server_h__ 1

#include "include.h"
#include "request.h"

#define OFFSET_NONE 0xffffffff

extern MEMCACHED *  server_new(const char *hostname, const int port,
        const int cluster);
extern void    server_connect_all(void);
extern char *server_get_status_summary(int cluster);
extern void    server_disconnect(MEMCACHED *server);

extern MEMCACHED *server_for_key_uponly(const char *key, const int cluster);
extern MEMCACHED *server_for_key(const char *key, const int cluster);
extern MEMCACHED *server_number(int num);

extern int    server_is_same(MEMCACHED *server1, MEMCACHED *server2);
extern int    server_is_up(MEMCACHED *server);
extern int    server_is_real(MEMCACHED *server);
extern int    server_do_request(MEMCACHED *server, REQUEST *request);
extern void   server_do_request_async(MEMCACHED *server, const REQUEST *request, int reliable);
extern void   server_expect_bytes(MEMCACHED *server, uint32_t bytes);
extern int    server_get_cluster(MEMCACHED *server);
extern char * server_get_description(MEMCACHED *server);
extern void   server_set_quickack(MEMCACHED *server);
extern time_t server_get_last_down_time(MEMCACHED *server);
extern time_t server_get_last_connect_time(MEMCACHED *server);

extern int    server_count(void);

extern void   server_clear_prefixes(void);

extern double server_average_queue_length(void);

/* Declared here because MEMCACHED isn't defined in request.h */
extern void  request_set_server(REQUEST *req, MEMCACHED *server);

#endif /* __server_h__ */
