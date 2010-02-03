
#include "proxy.h"
#include "server.h"
#include "qio.h"
#include "gfuncs.h"

#define BIN_VERSION "1.0"

// global stat vals
int stats_cmdget_count = 0;
int stats_cmdset_count = 0;
int stats_cmddelete_count = 0;
int stats_get_hits = 0;

long stats_bytes_read = 0;
long stats_bytes_written = 0;
int stats_failed_client_connections = 0;
int stats_successful_client_connections = 0;
int stats_closed_client_connections = 0;
int stats_starttime = 0;

// setter properties
void stats_incr_cmdget_count() { stats_cmdget_count++; }
void stats_incr_cmdset_count() { ++stats_cmdset_count; }
void stats_incr_cmddelete_count() { ++stats_cmddelete_count; }
void stats_incr_get_hits() { ++stats_get_hits; }
void stats_add_bytes_read(int v) { stats_bytes_read += v; }
void stats_add_bytes_written(int v) { stats_bytes_written += v; }
void stats_incr_failed_client_connections() { ++stats_failed_client_connections; }
void stats_incr_closed_client_connections() { ++stats_closed_client_connections; }
void stats_incr_successful_client_connections() { ++stats_successful_client_connections; }
void stats_set_starttime(int v) { stats_starttime = v; }

// tracking of remote delete hit rate on a per-prefix basis
typedef struct {
  char  *prefix;
  int  deletes;
  int  hits;
} REMOTE_DELETE;

static REMOTE_DELETE *remote_deletes = NULL;
static int nremote_deletes = 0;
void stats_remote_delete(REQUEST *req) {
  int    i;
  REMOTE_DELETE  *rd = NULL;
  char    *key = (char*)request_get_command(req);
  int    is_hit = request_get_response(req)[0] == 'D';
  char    *c;

  if (strncmp(key, "delete ", 7))
    return;
  key += 7;

  c = strchr(key, ':');
  if (! c)
    c = key + 3;

  // Number of prefixes should stay low enough that linear-searching
  // the list won't be too big a performance hit.
  for (i = 0; i < nremote_deletes; i++) {
    if (! strncmp(remote_deletes[i].prefix, key, c - key)) {
      rd = &remote_deletes[i];
      break;
    }
  }

  if (! rd) {
    rd = (REMOTE_DELETE*)grealloc(remote_deletes,
                  sizeof(REMOTE_DELETE) * (nremote_deletes + 1));
    remote_deletes = rd;
    rd += nremote_deletes;
    nremote_deletes++;

    rd->prefix = (char*)gmalloc(c - key + 1);
    gstrncpy(rd->prefix, key, c - key + 1);
    rd->deletes = 0;
    rd->hits = 0;
  }

  rd->deletes++;
  if (is_hit)
    rd->hits++;
}


double getTimeFromTimeval(struct timeval mytimeval)
{
    return  ((double) mytimeval.tv_sec) + (((double) mytimeval.tv_usec) / 1000000);
}

void qio_printf(int fd, char *fmt, ...) {
  char buf[1000];
  int len;
  va_list ap;

  va_start(ap, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  qio_write(fd, buf, len);
}

void  stats_dump(int fd)
{
  int currentTime = (int) time(NULL);
  struct rusage myrusage;
  int i;

  getrusage(RUSAGE_SELF, &myrusage); // this is the child pid

  qio_printf(fd, "STAT parent_pid %d\n", (int) getppid());
  qio_printf(fd, "STAT child_pid %d\n", (int) getpid());
  qio_printf(fd, "STAT uptime %d\n", currentTime - stats_starttime);
  qio_printf(fd, "STAT time %d\n", (int) time(NULL));
  qio_printf(fd, "STAT version %s\n", BIN_VERSION);
  qio_printf(fd, "STAT rusage_user %f\n", getTimeFromTimeval(myrusage.ru_utime));
  qio_printf(fd, "STAT rusage_system %f\n", getTimeFromTimeval(myrusage.ru_stime));
  qio_printf(fd, "STAT total_connections %d\n",
      stats_successful_client_connections -
      stats_closed_client_connections);
  qio_printf(fd, "STAT successful_client_connections %d\n", stats_successful_client_connections);
  qio_printf(fd, "STAT failed_client_connections %d\n", stats_failed_client_connections);
  qio_printf(fd, "STAT average_client_queue_length %f\n",
      server_average_queue_length());
  qio_printf(fd, "STAT num_servers %d\n", server_count());
  qio_printf(fd, "STAT cmd_get %d\n", stats_cmdget_count);
  qio_printf(fd, "STAT cmd_set %d\n", stats_cmdset_count);
  qio_printf(fd, "STAT cmd_delete %d\n", stats_cmddelete_count);
  qio_printf(fd, "STAT get_hits %d\n", stats_get_hits);

  // note: this approximates misses, since we don't receive the number
  // of misses, but have to subtract hits from total get attempts
  qio_printf(fd, "STAT get_misses %d\n", stats_cmdget_count - stats_get_hits);
  qio_printf(fd, "STAT bytes_written %ld\n", stats_bytes_written);
  qio_printf(fd, "STAT bytes read %ld\n", stats_bytes_read);

  for (i = 0; i < nremote_deletes; i++) {
    REMOTE_DELETE *rd = &remote_deletes[i];
    qio_printf(fd, "STAT remote_delete %s %d %d\r\n",
      rd->prefix, rd->deletes, rd->hits);
  }

  qio_printf(fd, "END\r\n");
}
