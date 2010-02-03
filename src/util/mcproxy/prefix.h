#ifndef __prefix_h__
#define __prefix_h__ 1

#include "include.h"
#include "prefix_int.h"

/*
 * Dummy prefix description for type safety.
 */
typedef struct {
  char  dummy1[8];
  char  dummy2[1];
  char  *dummy3;
} PREFIX;

extern PREFIX *prefix_add(const char *prefix);
extern PREFIX *prefix_find(const char *key);

extern const CLUSTER_SERVERS*
prefix_get_cluster_servers_for_key(const char *key, const int cluster);
extern void prefix_get_servers_for_cluster_servers
(const CLUSTER_SERVERS * cluster, MEMCACHED ***servers, uint32_t *nservers);

extern void prefix_add_server(PREFIX *prefix, MEMCACHED *server);
extern void prefix_server_up(PREFIX *prefix, MEMCACHED *server);
extern void prefix_server_down(PREFIX *prefix, MEMCACHED *server);
extern void prefix_clear(void);
extern void prefix_init();

/* declared here because server.h doesn't define PREFIX */
extern void   server_add_prefix(MEMCACHED *vmcd, PREFIX *prefix);

#endif /* __prefix_h__ */
