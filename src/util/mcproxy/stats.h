#ifndef __stats_h__
#define __stats_h__ 1

#include "include.h"
#include "request.h"

// setter properties
extern void  stats_incr_cmdget_count();
extern void  stats_incr_cmdset_count();
extern void  stats_incr_cmddelete_count();
extern void  stats_incr_get_hits();
extern void  stats_add_bytes_read(int v);
extern void  stats_add_bytes_written(int v);
extern void  stats_incr_failed_client_connections();
extern void  stats_incr_closed_client_connections();
extern void  stats_incr_successful_client_connections();
extern void  stats_set_starttime(int v);

extern void  stats_remote_delete(REQUEST *req);
extern double  getTimeFromTimeval(struct timeval mytimeval);
extern void  stats_dump(int fd);

#endif /* __stats_h__ */
