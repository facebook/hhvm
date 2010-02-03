
#ifndef __MCPROXY_INCLUDE__
#define __MCPROXY_INCLUDE__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <util/config.h>
#include <util/base.h>
#include <util/logger.h>
#include <util/exception.h>

extern "C" {
#include <ch/continuum.h>
}

/*
 * Dummy memcached server description for type safety.
 */
typedef struct {
  int  dummy1;
  char dummy2[47];
  int  dummy3;
} MEMCACHED;

/**
 * Returns port number mcproxy listens on.
 */
int start_mcproxy(int argc, char **argv);
void run_mcproxy();

#endif // __MCPROXY_INCLUDE__
