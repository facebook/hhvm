
#include "proxy.h"
#include "prefix.h"
#include "cluster.h"
#include "gfuncs.h"
#include "tinystdio.h"

/*
 * Configuration file parsing module.
 */
#define MAX_LINE_LENGTH    5000
#define SERVER_DELIMITERS  " \t,"
#define  PREFIX_DELIMITERS  " \t,"
#define DEFAULT_MEMCACHED_PORT  11211
#define CONFIG_DATA_KEY         "mcproxy.conf"


/*
 * Configuration context. When we read a "servers" line, all the servers are
 * added to this context, so that they can all be affected by subsequent
 * config lines.
 */
typedef struct {
  int    nservers;
  MEMCACHED  *servers[1];
} CONFIG_CONTEXT;

#define CONTEXT_SIZE(nservers) (sizeof(CONFIG_CONTEXT) + \
        (sizeof(MEMCACHED *) * ((nservers) - 1)))

static MEMCACHED *config_mcd = NULL;
static const char* config_key = NULL;

char* latest_config = NULL;
int latest_config_len = 0;
char* latest_config_file = NULL;
char* latest_config_server = NULL;

/*
 * Cleans up a configuration context.
 */
static void
context_cleanup(CONFIG_CONTEXT **context)
{
  if (context != NULL && *context != NULL)
  {
    gfree(*context);
    *context = NULL;
  }
}


/*
 * Adds a server to the configuration context so that subsequent config
 * directives will affect it.
 */
static void
context_add_server(CONFIG_CONTEXT **context, MEMCACHED *server)
{
  if (context == NULL || server == NULL)
  {
    HPHP::Logger::Error("Invalid parameter to context_add_server");
                throw HPHP::Exception("abort");
  }

  if (*context == NULL)
    *context = (CONFIG_CONTEXT *)gcalloc(CONTEXT_SIZE(1));
  else
    *context = (CONFIG_CONTEXT *)grealloc(*context,
        CONTEXT_SIZE((*context)->nservers + 1));

  (*context)->servers[(*context)->nservers++] = server;
}


/*
 * Returns a MEMCACHED* for a "host[:port]" string.
 */
static MEMCACHED *
host_port_to_server(char *hostport, int cluster_id)
{
  char *c = strchr(hostport, ':');
  int port = DEFAULT_MEMCACHED_PORT;

  if (c != NULL)
  {
    *c = '\0';
    port = atoi(c + 1);
  }

  return server_new(hostport, port, cluster_id);
}


/*
 * Adds a list of servers to the configuration.
 */
static int
config_server(char *servers, CONFIG_CONTEXT **context)
{
  char *server;
  int cluster_id;

  if (servers == NULL)
  {
    HPHP::Logger::Error("Found \"server\" config line with no servers");
    return -1;
  }

  context_cleanup(context);
  server = strtok(servers, SERVER_DELIMITERS);
  cluster_id = atoi(server);
  if (cluster_id == 0)
  {
    HPHP::Logger::Error("Invalid cluster ID '%s'", server);
    return -1;
  }

  server = strtok(NULL, SERVER_DELIMITERS);

  do {
    MEMCACHED *mcd = host_port_to_server(server, cluster_id);
    if (mcd == NULL)
      return -1;
    context_add_server(context, mcd);
  } while ((server = strtok(NULL, SERVER_DELIMITERS)) != NULL);

  return 0;
}

/*
 * Adds a list of prefixes to the current set of servers.
 */
static int
config_prefix(char *prefixes, CONFIG_CONTEXT **context)
{
  char *prefix;

  if (context == NULL)
  {
    HPHP::Logger::Error("Null context pointer");
                throw HPHP::Exception("abort");
  }

  if (prefixes == NULL)
  {
    HPHP::Logger::Error("Found \"prefix\" config line with no prefixes");
    return -1;
  }

  if (*context == NULL)
  {
    HPHP::Logger::Error("Found \"prefix\" config line with no server list");
    return -1;
  }

  prefix = strtok(prefixes, PREFIX_DELIMITERS);
  do {
    PREFIX *pfx = prefix_add(prefix);
    int i;

    for (i = 0; i < (*context)->nservers; i++)
      prefix_add_server(pfx, (*context)->servers[i]);
  } while ((prefix = strtok(NULL, PREFIX_DELIMITERS)) != NULL);

  return 0;
}

/*
 * Marks the current server set as wildcards, so they can serve requests for
 * keys that aren't prefixed by one of the explicit prefixes.
 */
static int
config_wildcard(char *value, CONFIG_CONTEXT **context)
{
  int i;

  if (context == NULL)
  {
    HPHP::Logger::Error("Null context pointer");
                throw HPHP::Exception("abort");
  }

  if (*context == NULL)
  {
    HPHP::Logger::Error("Found \"wildcard\" config line with no server list");
    return -1;
  }

  for (i = 0; i < (*context)->nservers; i++)
    prefix_add_server(prefix_find(NULL), (*context)->servers[i]);

  return 0;
}

/*
 * Defines a cluster address and netmask.
 */
static int
config_cluster(char *value, CONFIG_CONTEXT **context)
{
  char  *raw_id, *raw_addr, *raw_mask;
  int  id;
  in_addr_t addr, mask;

  if (context == NULL)
  {
    HPHP::Logger::Error("Null context pointer");
                throw HPHP::Exception("abort");
  }

  raw_id = strtok(value, " \t");
  if (raw_id == NULL)
  {
    HPHP::Logger::Error("No cluster ID specified");
    return -1;
  }
  if ((id = atoi(raw_id)) <= 0)
  {
    HPHP::Logger::Error("Cluster ID must be a positive integer");
    return -1;
  }

  raw_addr = strtok(NULL, " \t");
  if (raw_addr == NULL)
  {
    HPHP::Logger::Error("No cluster address specified");
    return -1;
  }
  if ((addr = inet_addr(raw_addr)) == INADDR_NONE)
  {
    HPHP::Logger::Error("Invalid cluster address specified");
    return -1;
  }

  raw_mask = strtok(NULL, " \t");
  if (raw_mask == NULL)
  {
    HPHP::Logger::Error("No cluster netmask specified");
    return -1;
  }
  if ((mask = inet_addr(raw_mask)) == INADDR_NONE)
  {
    HPHP::Logger::Error("Invalid cluster netmask specified");
    return -1;
  }

  cluster_add(id, &addr, &mask);
  return 0;
}

/*
 * Interprets a configuration item.
 */
static int
use_config_entry(const char *keyword, char *value, CONFIG_CONTEXT **context)
{
  if (! strcmp(keyword, "server"))
    return config_server(value, context);
  else if (! strcmp(keyword, "prefix"))
    return config_prefix(value, context);
  else if (! strcmp(keyword, "wildcard"))
    return config_wildcard(value, context);
  else if (! strcmp(keyword, "cluster"))
    return config_cluster(value, context);

  return 0;
}


/*
 * Reads configuration from a TINYFILE stream.
 */
static int
read_config_stream(TINYFILE *fp)
{
  char    buf[MAX_LINE_LENGTH];
  CONFIG_CONTEXT  *context = NULL;
  int    ret = 0;

  prefix_clear();
  cluster_clear();

  while (ret == 0 && tiny_gets(fp, buf, MAX_LINE_LENGTH) != NULL)
  {
    char *keyword, *value;

    keyword = strtok(buf, " \r\n\t");
    if (keyword == NULL || keyword[0] == '\0' || keyword[0] == '#')
      continue;

    value = strtok(NULL, "\r\n");
    if (use_config_entry(keyword, value, &context))
      ret = -1;
  }

  context_cleanup(&context);
  return ret;
}


/*
 * Reads a config file of a given name.
 */
int
read_config_file(char *filename, REQUEST_CBF cbf)
{
  TINYFILE *fp;
  int status;

  latest_config_file = (char*)grealloc(latest_config_file,
                                       strlen(filename) + 1);
  strcpy(latest_config_file, filename);

  int fd = open(filename, O_RDONLY);
  if (fd < 0)
  {
    HPHP::Logger::Error("%s", filename);
    return -1;
  }

  fp = tiny_fdopen(fd);
  if (fp == NULL)
  {
    HPHP::Logger::Error("tiny_fdopen");
    close(fd);
    return -1;
  }

  status = read_config_stream(fp);

  tiny_free(fp);
  close(fd);

  if (0 == status && NULL != cbf)
    cbf(NULL, NULL);

  return status;
}


/*
 * Reads configuration from a buffer.
 */
int
read_config_buffer(char *buf, int len)
{
  TINYFILE *fp = tiny_bufopen(buf, len);
  int status;

  if (NULL == fp)
  {
    HPHP::Logger::Error("tiny_bufopen");
    return -1;
  }

  status = read_config_stream(fp);

  tiny_free(fp);

  return status;
}

/*
 * Configuration has been read from a server; parse it.
 */
static void
do_read_config_server(REQUEST *req, void *arg)
{
  REQUEST_CBF cbf = (REQUEST_CBF) arg;

  if (request_get_status(req) == Error)
  {
    HPHP::Logger::Info("Couldn't read configuration from server. Reconnecting.");
    server_disconnect(config_mcd);
  }
  else if (request_get_total(req) <= 0)
  {
    HPHP::Logger::Error("Server doesn't have any configuration data!");
  }
  else
  {
    latest_config_len = request_get_total(req);
    latest_config = (char*)grealloc(latest_config, latest_config_len+1);
    memcpy(latest_config, request_get_data(req), latest_config_len);
    latest_config[latest_config_len] = '\0';

    if (read_config_buffer(latest_config, latest_config_len))
    {
      HPHP::Logger::Info("Couldn't parse config data from server");
    }
    else
    {
      (cbf)(NULL, NULL);
    }
  }

  request_free(req);
}

/*
 * Reads configuration from a memcached server.
 */
void
read_config_server(REQUEST_CBF cbf)
{
  if (server_is_up(config_mcd))
  {
    server_do_request(config_mcd,
        request_new_get(&config_key, 1,
            do_read_config_server,
            (void *) cbf));
  }
  else {
    HPHP::Logger::Info("Skipping config from down config server.");
  }
}

/*
 * Sets the memcached server we will read our configuration from.
 */
int
config_set_server(char *hostport, const char *key)
{
  latest_config_server = (char*)grealloc(latest_config_server, strlen(hostport) + 1);
  strcpy(latest_config_server, hostport);

  config_mcd = host_port_to_server(gstrdup(hostport), 0);
  config_key = key ? key : CONFIG_DATA_KEY;
  if (NULL == config_mcd)
  {
    HPHP::Logger::Error("Can't attempt to open connection to %s",
                                    hostport);
    return -1;
  }

  return 0;
}
