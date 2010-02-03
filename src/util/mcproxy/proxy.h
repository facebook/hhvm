#ifndef __proxy_h__
#define __proxy_h__ 1

#include "include.h"

/* Flag on a set/add/replace to indicate the request should be replicated */
#define FLAG_REPLICATE 0x400
/*
 * Flag on a set that permits it to return before actually executing
 * the store. This semantics of this flag are:
 *
 *  - The request will be executed in a timely fashion. Specifically,
 *  it would be executed with the same speed as if the flag is not
 *  set.
 *
 * - The request will be executed in order with all other requests to
 *   the same key, even synchronous ones. For example, if you do an
 *   async set and then a sync delete, then by the time the delete call
 *   returns, the key will be deleted and there will be no risk of the
 *   async set taking affect.
 */
#define FLAG_ASYNC_SET 0x1000

extern int    serversock(int port);
extern void    host_to_addr(const char *hostname, char *addrbuf);
extern int    set_nodelay(int fd, int want_nodelay);
extern int    determine_local_address(struct in_addr *addrbuf);
extern int    clientsock_nb(char *host, int port, int *fd);
extern unsigned int  hash_crc32(const char *key, const unsigned int len);

extern void    client_add_handler(int fd);

#endif /* __proxy_h__ */
