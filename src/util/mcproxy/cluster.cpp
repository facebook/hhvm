
#include "proxy.h"
#include "gfuncs.h"
#include "cluster.h"
#include "server.h"
#include "../network.h"

static int local_cluster = -1;
static int num_clusters = 0;

/*
 * Each cluster contains one or more networks, each of which is defined by a
 * network number and netmask. We use that to determine which cluster a
 * particular host is part of.
 */
typedef struct _cluster CLUSTER;
struct _cluster {
  int    id;
  in_addr_t  *networks;
  in_addr_t  *netmasks;
  int    num_networks;
  CLUSTER    *next;
};

static CLUSTER *clusters = NULL;

/*
 * Determines the ID of the local cluster.
 */
int
cluster_determine_id(void)
{
  struct in_addr addr;

  /* The user might have specified a cluster ID manually. */
  if (local_cluster > -1)
    return 0;

  if (determine_local_address(&addr)) {
    HPHP::Logger::Error("Can't determine local address");
    return -1;
  }

  local_cluster = cluster_find_id(&addr.s_addr);
  if (local_cluster < 0) {
    std::string local_addr = HPHP::Util::safe_inet_ntoa(addr);
    HPHP::Logger::Error("Local network address %s doesn't match any clusters",
                        local_addr.c_str());
    return -1;
  }

  return 0;
}

/*
 * Returns the local cluster ID.
 */
int cluster_id(void)
{
  return local_cluster;
}

/*
 * Explicitly sets the local cluster ID.
 */
void cluster_set_id(int id)
{
  local_cluster = id;
}

/*
 * Applies a netmask to a network address.
 */
static void
apply_netmask(const in_addr_t * const address,
              const in_addr_t * const mask,
              in_addr_t * const result)
{
  const unsigned char *address_c = (unsigned char *) address;
  const unsigned char *mask_c = (unsigned char *) mask;
  unsigned char *result_c = (unsigned char *) result;
  int i;

  for (i = 0; i < (int)sizeof(in_addr_t); i++)
    *result_c++ = *address_c++ & *mask_c++;
}

/*
 * Adds a new cluster to the cluster list.
 */
void
cluster_add(int id, in_addr_t *network, in_addr_t *netmask)
{
  CLUSTER  *cluster;
  int cluster_id;

  /* Make sure we don't have any duplicate entries */
  if ((cluster_id = cluster_find_id(network)) >= 0)
  {
    HPHP::Logger::Error("Duplicate network address in clusters %d and %d, "
      "ignoring in cluster %d", cluster_id, id, id);
    return;
  }

  for (cluster = clusters; cluster != NULL; cluster = cluster->next)
    if (cluster->id == id)
      break;
  if (cluster == NULL)
  {
    cluster = (CLUSTER*)gmalloc(sizeof(CLUSTER));
    cluster->id = id;
    cluster->networks = (in_addr_t*)gmalloc(sizeof(in_addr_t));
    cluster->netmasks = (in_addr_t*)gmalloc(sizeof(in_addr_t));
    cluster->num_networks = 0;

    cluster->next = clusters;
    clusters = cluster;

    num_clusters++;
  }
  else
  {
    cluster->networks = (in_addr_t*)grealloc(cluster->networks,
      (cluster->num_networks + 1) * sizeof(in_addr_t));
    cluster->netmasks = (in_addr_t*)grealloc(cluster->netmasks,
      (cluster->num_networks + 1) * sizeof(in_addr_t));
  }

  memcpy(cluster->netmasks + cluster->num_networks, netmask,
    sizeof(in_addr_t));
  apply_netmask(network, netmask,
    cluster->networks + cluster->num_networks);
  cluster->num_networks++;
}

/*
 * Finds a cluster that matches a network address.
 *
 * Returns: Cluster ID or -1 if not found
 */
int
cluster_find_id(in_addr_t *addr)
{
  CLUSTER  *cur;
  int i;

  for (cur = clusters; cur != NULL; cur = cur->next)
  {
    for (i = 0; i < cur->num_networks; i++)
    {
      in_addr_t masked_addr;

      apply_netmask(addr, &cur->netmasks[i], &masked_addr);
      if (! memcmp(&masked_addr, &cur->networks[i],
          sizeof(cur->networks[0])))
        return cur->id;
    }
  }

  return -1;
}

/*
 * Replicates a request across all the remote clusters, skipping any
 * servers that we have already sent the request to (including, possibly,
 * the server in the local cluster.) Optionally queue the request for
 * later delivery if it needs to be (semi-)reliable.
 */
void
cluster_replicate(REQUEST *req, int reliable)
{
  MEMCACHED **servers;
  int       nservers = 0;
  CLUSTER   *clust;
  char      *key = request_get_first_key(req);

  if (num_clusters == 0 || key == NULL)
    return;

  servers = (MEMCACHED **)gmalloc(sizeof(MEMCACHED *) * num_clusters);
  if (local_cluster != -1) {
    servers[0] = server_for_key(key, local_cluster);
    nservers ++;
  }

  for (clust = clusters; clust != NULL; clust = clust->next)
  {
    int i;
    MEMCACHED *server = server_for_key(key, clust->id);

    if (! server_is_real(server))
      continue;

    /* Have we already sent to this server? */
    for (i = 0; i < nservers; i++)
      if (server_is_same(server, servers[i]))
        break;
    if (i == nservers)
    {
      /* Nope. Send request and add this server to the list */
      server_do_request_async(server, req, reliable);

      if (nservers >= num_clusters)
      {
        HPHP::Logger::Error("Number of servers exceeds number of "
          "clusters -- bug!");
        break;
      }
      servers[nservers++] = server;
    }
  }

  free(servers);
}

/*
 * Clears the list of clusters.
 */
void
cluster_clear()
{
  CLUSTER *next;

  while (clusters != NULL)
  {
    next = clusters->next;
    gfree(clusters->networks);
    gfree(clusters->netmasks);
    gfree(clusters);
    clusters = next;
  }

  local_cluster = -1;
  num_clusters = 0;
}
