/*
 * Prefix management module.  Each known prefix is associated with a list
 * of servers.  If a key with a known prefix is looked up or stored, only
 * servers that serve that prefix are contacted (unless all of them are
 * down, in which case the request is hashed over the complete list of
 * servers.)
 */
#include "proxy.h"
#include "server_int.h"
#include "gfuncs.h"

/* List of prefix descriptors, alphabetically sorted.  */
static PREFIX_IMPL **prefixes = NULL;
static int32_t nprefixes = 0;

/*
 * This special prefix is used as a catch-all for servers that can handle
 * requests of any type.
 */
static PREFIX_IMPL *fallback_prefix = NULL;

/*
 * Adds a new prefix to the list.
 */
PREFIX *prefix_add(const char *prefix)
{
  PREFIX_IMPL **newlist;
  PREFIX_IMPL *newprefix;
  PREFIX      *existing;
  int32_t     i;

  if (prefix == NULL || strlen(prefix) == 0)
    return (PREFIX *) fallback_prefix;

  existing = prefix_find(prefix);
  if (existing != (PREFIX *) fallback_prefix)
    return existing;

  newlist = (PREFIX_IMPL **)gmalloc(sizeof(PREFIX_IMPL *) * (nprefixes+1));

  for (i = 0; i < nprefixes; i++)
  {
    if (strcmp(prefixes[i]->prefix, prefix) < 0)
      newlist[i] = prefixes[i];
    else
      break;
  }

  newprefix = (PREFIX_IMPL *)gmalloc(sizeof(PREFIX_IMPL));
  newprefix->prefix = gstrdup(prefix);
  newprefix->prefix_len = strlen(prefix);
  newprefix->clusters = NULL;
  newlist[i] = newprefix;

  for (; i < nprefixes; i++)
    newlist[i + 1] = prefixes[i];

  if (prefixes != NULL)
    gfree(prefixes);

  prefixes = newlist;
  nprefixes++;

  return (PREFIX *) newprefix;
}


/*
 * Finds the prefix entry that matches a particular key. If there is no
 * matching prefix, returns the fallback prefix.
 */
PREFIX *prefix_find(const char *key)
{
  int32_t min, max, mid;

  if (key == NULL || key[0] == '\0' || nprefixes == 0)
    return (PREFIX *) fallback_prefix;

  max = nprefixes - 1;
  min = 0;
  while (min <= max)
  {
    mid = (min + max) / 2;
    if (strcmp(prefixes[mid]->prefix, key) > 0)
      max = mid - 1;
    else if (! strncmp(prefixes[mid]->prefix, key,
          prefixes[mid]->prefix_len))
      return (PREFIX *) prefixes[mid];
    else
      min = mid + 1;
  }

  return (PREFIX *) fallback_prefix;
}


/*
 * Finds the cluster with a given ID.
 */
static CLUSTER_SERVERS *prefix_find_cluster(PREFIX_IMPL *prefix,
          const int cluster)
{
  CLUSTER_SERVERS *clust;

  if (prefix == NULL)
    return NULL;
  for (clust = prefix->clusters; clust != NULL; clust = clust->next)
    if (clust->cluster == cluster)
      return clust;
  return NULL;
}


const CLUSTER_SERVERS *
prefix_get_cluster_servers_for_key(const char *key,
                                   const int cluster)
{
  PREFIX_IMPL *prefix = (PREFIX_IMPL *) prefix_find(key);
  CLUSTER_SERVERS *clust = prefix_find_cluster(prefix, cluster);

  /*
   * If this cluster doesn't have a prefix for the key (but some
   * other cluster does), fall back to the wildcard prefix.
   */
  if (clust == NULL)
  {
    prefix = (PREFIX_IMPL *) prefix_find("*");
    clust = prefix_find_cluster(prefix, cluster);
  }

  return clust;
}


/*
 * Finds the list of servers that can serve a key.
 */
void
prefix_get_servers_for_cluster_servers(const CLUSTER_SERVERS * cluster,
                                       MEMCACHED ***servers,
                                       uint32_t *nservers)
{
  if (cluster != NULL)
  {
    *nservers = cluster->nservers;
    *servers = cluster->servers;
  }
  else
  {
    *nservers = 0;
    *servers = NULL;
  }
}


/*
 * Adds a server to a prefix's server list.
 */
void
prefix_add_server(PREFIX *vprefix, MEMCACHED *server)
{
  PREFIX_IMPL  *prefix = (PREFIX_IMPL *) vprefix;
  int    cluster = server_get_cluster(server);
  CLUSTER_SERVERS  *clust = prefix_find_cluster(prefix, cluster);

  if (clust == NULL)
  {
    clust = (CLUSTER_SERVERS*)gcalloc(sizeof(CLUSTER_SERVERS));
    clust->cluster = cluster;
    if (is_consistent_server(server))
    {
      clust->use_consistent_hashing = 1;
    }
    else
    {
      clust->use_consistent_hashing = 0;
    }
    clust->next = prefix->clusters;
    prefix->clusters = clust;
  }

  clust->servers = (MEMCACHED**)grealloc(clust->servers,
    sizeof(MEMCACHED *) * (clust->nservers + 1));
  clust->servers[clust->nservers++] = server;

  server_add_prefix(server, vprefix);

  HPHP::Logger::Verbose("Added server for prefix %s", prefix->prefix);

  if (server_is_up(server) && server_is_real(server))
    prefix_server_up(vprefix, server);
}


/*
 * Records that one of a prefix's servers has come up.
 */
void
prefix_server_up(PREFIX *vprefix, MEMCACHED *server)
{
  PREFIX_IMPL  *prefix = (PREFIX_IMPL *) vprefix;
  int    cluster = server_get_cluster(server);
  CLUSTER_SERVERS  *clust = prefix_find_cluster(prefix, cluster);
  clust->nservers_up++;
  HPHP::Logger::Verbose("server up for prefix %s", prefix->prefix);
}


/*
 * Records that one of a prefix's servers has gone down.
 */
void
prefix_server_down(PREFIX *vprefix, MEMCACHED *server)
{
  PREFIX_IMPL  *prefix = (PREFIX_IMPL *) vprefix;
  int    cluster = server_get_cluster(server);
  CLUSTER_SERVERS  *clust = prefix_find_cluster(prefix, cluster);
  clust->nservers_up--;
  HPHP::Logger::Verbose("server down for prefix %s", prefix->prefix);
}

/*
 * Frees a prefix.
 */
static void
prefix_free(PREFIX_IMPL *prefix)
{
  CLUSTER_SERVERS *clust, *next;

  if (NULL == prefix)
    return;
  gfree(prefix->prefix);

  for (clust = prefix->clusters; clust != NULL; clust = next)
  {
    next = clust->next;
    gfree(clust->servers);
    gfree(clust);
  }

  gfree(prefix);
}

/*
 * Clears the current list of prefixes, e.g. because of a configuration
 * reload. The memcached server structures aren't touched.
 */
void
prefix_clear()
{
  server_clear_prefixes();

  while (nprefixes > 0)
  {
    prefix_free(prefixes[--nprefixes]);
  }

  prefix_free(fallback_prefix);
  fallback_prefix = (PREFIX_IMPL*)gmalloc(sizeof(PREFIX_IMPL));
  fallback_prefix->prefix = gstrdup("");
  fallback_prefix->prefix_len = 0;
  fallback_prefix->clusters = NULL;
}


/*
 * Initializes the prefix module.
 */
void
prefix_init()
{
  prefix_clear();
}
