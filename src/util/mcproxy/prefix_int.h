#ifndef __prefix_int_h__
#define __prefix_int_h__ 1

#include "include.h"
#include "server.h"

/*
 * Cluster server list. This is just a cluster number and a list of servers.
 * Each prefix has a server list for each cluster.
 */
typedef struct _cluster_servers CLUSTER_SERVERS;
struct _cluster_servers {
  int    cluster;    /* cluster ID */
  MEMCACHED  **servers;
  uint32_t  nservers;    /* Size of servers list */
  uint32_t  nservers_up;    /* How many servers are up? */
  int             use_consistent_hashing;
  CLUSTER_SERVERS  *next;
};

/*
 * Key prefix description. Each known prefix is associated with a list
 * of servers.  If a key with a known prefix is looked up or stored, only
 * servers that serve that prefix are contacted (unless all of them are
 * down, in which case the request is hashed over the complete list of
 * servers.)
 */
typedef struct _prefix PREFIX_IMPL;
struct _prefix {
  char    *prefix;
  int    prefix_len;    /* strlen(prefix) */
  CLUSTER_SERVERS  *clusters;
};

#endif /* __prefix_int_h__ */
