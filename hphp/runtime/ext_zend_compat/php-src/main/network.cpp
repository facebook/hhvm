/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Stig Venaas <venaas@uninett.no>                              |
   | Streams work by Wez Furlong <wez@thebrainroom.com>                   |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

/*#define DEBUG_MAIN_NETWORK 1*/

#include "php.h"

#include <stddef.h>

#include <sys/param.h>

#include <sys/types.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifndef _FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifndef HAVE_INET_ATON
int inet_aton(const char *, struct in_addr *);
#endif

#include "php_network.h"

#if defined(AF_UNIX)
#include <sys/un.h>
#endif

#include <ext/standard/file.h>

# define SOCK_ERR -1
# define SOCK_CONN_ERR -1
# define PHP_TIMEOUT_ERROR_VALUE    ETIMEDOUT

#if HAVE_GETADDRINFO
#ifdef HAVE_GAI_STRERROR
#  define PHP_GAI_STRERROR(x) (gai_strerror(x))
#else
#  define PHP_GAI_STRERROR(x) (php_gai_strerror(x))
/* {{{ php_gai_strerror
 */
static const char *php_gai_strerror(int code)
{
        static struct {
                int code;
                const char *msg;
        } values[] = {
#  ifdef EAI_ADDRFAMILY
                {EAI_ADDRFAMILY, "Address family for hostname not supported"},
#  endif
                {EAI_AGAIN, "Temporary failure in name resolution"},
                {EAI_BADFLAGS, "Bad value for ai_flags"},
                {EAI_FAIL, "Non-recoverable failure in name resolution"},
                {EAI_FAMILY, "ai_family not supported"},
                {EAI_MEMORY, "Memory allocation failure"},
#  ifdef EAI_NODATA
                {EAI_NODATA, "No address associated with hostname"},
#  endif
                {EAI_NONAME, "Name or service not known"},
                {EAI_SERVICE, "Servname not supported for ai_socktype"},
                {EAI_SOCKTYPE, "ai_socktype not supported"},
                {EAI_SYSTEM, "System error"},
                {0, NULL}
        };
        int i;

        for (i = 0; values[i].msg != NULL; i++) {
                if (values[i].code == code) {
                        return (char *)values[i].msg;
                }
        }

        return "Unknown error";
}
/* }}} */
#endif
#endif

/* {{{ php_network_freeaddresses
 */
PHPAPI void php_network_freeaddresses(struct sockaddr **sal)
{
  struct sockaddr **sap;

  if (sal == NULL)
    return;
  for (sap = sal; *sap != NULL; sap++)
    efree(*sap);
  efree(sal);
}
/* }}} */

/* {{{ php_network_getaddresses
 * Returns number of addresses, 0 for none/error
 */
PHPAPI int php_network_getaddresses(const char *host, int socktype, struct sockaddr ***sal, char **error_string TSRMLS_DC)
{
  struct sockaddr **sap;
  int n;
#if HAVE_GETADDRINFO
# if HAVE_IPV6
  static int ipv6_borked = -1; /* the way this is used *is* thread safe */
# endif
  struct addrinfo hints, *res, *sai;
#else
  struct hostent *host_info;
  struct in_addr in;
#endif

  if (host == NULL) {
    return 0;
  }
#if HAVE_GETADDRINFO
  memset(&hints, '\0', sizeof(hints));

  hints.ai_family = AF_INET; /* default to regular inet (see below) */
  hints.ai_socktype = socktype;

# if HAVE_IPV6
  /* probe for a working IPv6 stack; even if detected as having v6 at compile
   * time, at runtime some stacks are slow to resolve or have other issues
   * if they are not correctly configured.
   * static variable use is safe here since simple store or fetch operations
   * are atomic and because the actual probe process is not in danger of
   * collisions or race conditions. */
  if (ipv6_borked == -1) {
    int s;

    s = socket(PF_INET6, SOCK_DGRAM, 0);
    if (s == SOCK_ERR) {
      ipv6_borked = 1;
    } else {
      ipv6_borked = 0;
      closesocket(s);
    }
  }
  hints.ai_family = ipv6_borked ? AF_INET : AF_UNSPEC;
# endif

  if ((n = getaddrinfo(host, NULL, &hints, &res))) {
    if (error_string) {
      spprintf(error_string, 0, "php_network_getaddresses: getaddrinfo failed: %s", PHP_GAI_STRERROR(n));
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", *error_string);
    } else {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "php_network_getaddresses: getaddrinfo failed: %s", PHP_GAI_STRERROR(n));
    }
    return 0;
  } else if (res == NULL) {
    if (error_string) {
      spprintf(error_string, 0, "php_network_getaddresses: getaddrinfo failed (null result pointer) errno=%d", errno);
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", *error_string);
    } else {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "php_network_getaddresses: getaddrinfo failed (null result pointer)");
    }
    return 0;
  }

  sai = res;
  for (n = 1; (sai = sai->ai_next) != NULL; n++)
    ;

  *sal = (sockaddr**) safe_emalloc((n + 1), sizeof(*sal), 0);
  sai = res;
  sap = *sal;

  do {
    *sap = (sockaddr*) emalloc(sai->ai_addrlen);
    memcpy(*sap, sai->ai_addr, sai->ai_addrlen);
    sap++;
  } while ((sai = sai->ai_next) != NULL);

  freeaddrinfo(res);
#else
  if (!inet_aton(host, &in)) {
    /* XXX NOT THREAD SAFE (is safe under win32) */
    host_info = gethostbyname(host);
    if (host_info == NULL) {
      if (error_string) {
        spprintf(error_string, 0, "php_network_getaddresses: gethostbyname failed. errno=%d", errno);
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", *error_string);
      } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "php_network_getaddresses: gethostbyname failed");
      }
      return 0;
    }
    in = *((struct in_addr *) host_info->h_addr);
  }

  *sal = safe_emalloc(2, sizeof(*sal), 0);
  sap = *sal;
  *sap = emalloc(sizeof(struct sockaddr_in));
  (*sap)->sa_family = AF_INET;
  ((struct sockaddr_in *)*sap)->sin_addr = in;
  sap++;
  n = 1;
#endif

  *sap = NULL;
  return n;
}
/* }}} */

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

#if !defined(__BEOS__)
# define HAVE_NON_BLOCKING_CONNECT 1
# ifdef PHP_WIN32
typedef u_long php_non_blocking_flags_t;
#  define SET_SOCKET_BLOCKING_MODE(sock, save) \
     save = TRUE; ioctlsocket(sock, FIONBIO, &save)
#  define RESTORE_SOCKET_BLOCKING_MODE(sock, save) \
   ioctlsocket(sock, FIONBIO, &save)
# else
typedef int php_non_blocking_flags_t;
#  define SET_SOCKET_BLOCKING_MODE(sock, save) \
   save = fcntl(sock, F_GETFL, 0); \
   fcntl(sock, F_SETFL, save | O_NONBLOCK)
#  define RESTORE_SOCKET_BLOCKING_MODE(sock, save) \
   fcntl(sock, F_SETFL, save)
# endif
#endif

/* Connect to a socket using an interruptible connect with optional timeout.
 * Optionally, the connect can be made asynchronously, which will implicitly
 * enable non-blocking mode on the socket.
 * */
/* {{{ php_network_connect_socket */
PHPAPI int php_network_connect_socket(php_socket_t sockfd,
    const struct sockaddr *addr,
    socklen_t addrlen,
    int asynchronous,
    struct timeval *timeout,
    char **error_string,
    int *error_code)
{
#if HAVE_NON_BLOCKING_CONNECT
  php_non_blocking_flags_t orig_flags;
  int n;
  int error = 0;
  socklen_t len;
  int ret = 0;

  SET_SOCKET_BLOCKING_MODE(sockfd, orig_flags);

  if ((n = connect(sockfd, addr, addrlen)) != 0) {
    error = php_socket_errno();

    if (error_code) {
      *error_code = error;
    }

    if (error != EINPROGRESS) {
      if (error_string) {
        *error_string = php_socket_strerror(error, NULL, 0);
      }

      return -1;
    }
    if (asynchronous && error == EINPROGRESS) {
      /* this is fine by us */
      return 0;
    }
  }

  if (n == 0) {
    goto ok;
  }
# ifdef PHP_WIN32
  /* The documentation for connect() says in case of non-blocking connections
   * the select function reports success in the writefds set and failure in
   * the exceptfds set. Indeed, using PHP_POLLREADABLE results in select
   * failing only due to the timeout and not immediately as would be
   * expected when a connection is actively refused. This way,
   * php_pollfd_for will return a mask with POLLOUT if the connection
   * is successful and with POLLPRI otherwise. */
  if ((n = php_pollfd_for(sockfd, POLLOUT|POLLPRI, timeout)) == 0) {
#else
  if ((n = php_pollfd_for(sockfd, PHP_POLLREADABLE|POLLOUT, timeout)) == 0) {
#endif
    error = PHP_TIMEOUT_ERROR_VALUE;
  }

  if (n > 0) {
    len = sizeof(error);
    /*
       BSD-derived systems set errno correctly
       Solaris returns -1 from getsockopt in case of error
       */
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len) != 0) {
      ret = -1;
    }
  } else {
    /* whoops: sockfd has disappeared */
    ret = -1;
  }

ok:
  if (!asynchronous) {
    /* back to blocking mode */
    RESTORE_SOCKET_BLOCKING_MODE(sockfd, orig_flags);
  }

  if (error_code) {
    *error_code = error;
  }

  if (error) {
    ret = -1;
    if (error_string) {
      *error_string = php_socket_strerror(error, NULL, 0);
    }
  }
  return ret;
#else
  if (asynchronous) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Asynchronous connect() not supported on this platform");
  }
  return (connect(sockfd, addr, addrlen) == 0) ? 0 : -1;
#endif
}
/* }}} */

/* {{{ sub_times */
static inline void sub_times(struct timeval a, struct timeval b, struct timeval *result)
{
  result->tv_usec = a.tv_usec - b.tv_usec;
  if (result->tv_usec < 0L) {
    a.tv_sec--;
    result->tv_usec += 1000000L;
  }
  result->tv_sec = a.tv_sec - b.tv_sec;
  if (result->tv_sec < 0L) {
    result->tv_sec++;
    result->tv_usec -= 1000000L;
  }
}
/* }}} */

PHPAPI void php_network_populate_name_from_sockaddr(
    /* input address */
    struct sockaddr *sa, socklen_t sl,
    /* output readable address */
    char **textaddr, long *textaddrlen,
    /* output address */
    struct sockaddr **addr,
    socklen_t *addrlen
    TSRMLS_DC)
{
  if (addr) {
    *addr = (sockaddr*) emalloc(sl);
    memcpy(*addr, sa, sl);
    *addrlen = sl;
  }

  if (textaddr) {
#if HAVE_IPV6 && HAVE_INET_NTOP
    char abuf[256];
#endif
    char *buf = NULL;

    switch (sa->sa_family) {
      case AF_INET:
        /* generally not thread safe, but it *is* thread safe under win32 */
        buf = inet_ntoa(((struct sockaddr_in*)sa)->sin_addr);
        if (buf) {
          *textaddrlen = spprintf(textaddr, 0, "%s:%d",
            buf, ntohs(((struct sockaddr_in*)sa)->sin_port));
        }

        break;

#if HAVE_IPV6 && HAVE_INET_NTOP
      case AF_INET6:
        buf = (char*)inet_ntop(sa->sa_family, &((struct sockaddr_in6*)sa)->sin6_addr, (char *)&abuf, sizeof(abuf));
        if (buf) {
          *textaddrlen = spprintf(textaddr, 0, "%s:%d",
            buf, ntohs(((struct sockaddr_in6*)sa)->sin6_port));
        }

        break;
#endif
#ifdef AF_UNIX
      case AF_UNIX:
        {
          struct sockaddr_un *ua = (struct sockaddr_un*)sa;

          if (ua->sun_path[0] == '\0') {
            /* abstract name */
            int len = strlen(ua->sun_path + 1) + 1;
            *textaddrlen = len;
            *textaddr = (char*) emalloc(len + 1);
            memcpy(*textaddr, ua->sun_path, len);
            (*textaddr)[len] = '\0';
          } else {
            *textaddrlen = strlen(ua->sun_path);
            *textaddr = estrndup(ua->sun_path, *textaddrlen);
          }
        }
        break;
#endif

    }

  }
}

PHPAPI int php_network_get_peer_name(php_socket_t sock,
    char **textaddr, long *textaddrlen,
    struct sockaddr **addr,
    socklen_t *addrlen
    TSRMLS_DC)
{
  php_sockaddr_storage sa;
  socklen_t sl = sizeof(sa);
  memset(&sa, 0, sizeof(sa));

  if (getpeername(sock, (struct sockaddr*)&sa, &sl) == 0) {
    php_network_populate_name_from_sockaddr((struct sockaddr*)&sa, sl,
        textaddr, textaddrlen,
        addr, addrlen
        TSRMLS_CC);
    return 0;
  }
  return -1;
}

PHPAPI int php_network_get_sock_name(php_socket_t sock,
    char **textaddr, long *textaddrlen,
    struct sockaddr **addr,
    socklen_t *addrlen
    TSRMLS_DC)
{
  php_sockaddr_storage sa;
  socklen_t sl = sizeof(sa);
  memset(&sa, 0, sizeof(sa));

  if (getsockname(sock, (struct sockaddr*)&sa, &sl) == 0) {
    php_network_populate_name_from_sockaddr((struct sockaddr*)&sa, sl,
        textaddr, textaddrlen,
        addr, addrlen
        TSRMLS_CC);
    return 0;
  }
  return -1;

}


/* Accept a client connection from a server socket,
 * using an optional timeout.
 * Returns the peer address in addr/addrlen (it will emalloc
 * these, so be sure to efree the result).
 * If you specify textaddr/textaddrlen, a text-printable
 * version of the address will be emalloc'd and returned.
 * */

/* {{{ php_network_accept_incoming */
PHPAPI php_socket_t php_network_accept_incoming(php_socket_t srvsock,
    char **textaddr, long *textaddrlen,
    struct sockaddr **addr,
    socklen_t *addrlen,
    struct timeval *timeout,
    char **error_string,
    int *error_code
    TSRMLS_DC)
{
  php_socket_t clisock = -1;
  int error = 0, n;
  php_sockaddr_storage sa;
  socklen_t sl;

  n = php_pollfd_for(srvsock, PHP_POLLREADABLE, timeout);

  if (n == 0) {
    error = PHP_TIMEOUT_ERROR_VALUE;
  } else if (n == -1) {
    error = php_socket_errno();
  } else {
    sl = sizeof(sa);

    clisock = accept(srvsock, (struct sockaddr*)&sa, &sl);

    if (clisock != SOCK_ERR) {
      php_network_populate_name_from_sockaddr((struct sockaddr*)&sa, sl,
          textaddr, textaddrlen,
          addr, addrlen
          TSRMLS_CC);
    } else {
      error = php_socket_errno();
    }
  }

  if (error_code) {
    *error_code = error;
  }
  if (error_string) {
    *error_string = php_socket_strerror(error, NULL, 0);
  }

  return clisock;
}
/* }}} */



/* Connect to a remote host using an interruptible connect with optional timeout.
 * Optionally, the connect can be made asynchronously, which will implicitly
 * enable non-blocking mode on the socket.
 * Returns the connected (or connecting) socket, or -1 on failure.
 * */

/* {{{ php_network_connect_socket_to_host */
php_socket_t php_network_connect_socket_to_host(const char *host, unsigned short port,
    int socktype, int asynchronous, struct timeval *timeout, char **error_string,
    int *error_code, char *bindto, unsigned short bindport
    TSRMLS_DC)
{
  int num_addrs, n, fatal = 0;
  php_socket_t sock;
  struct sockaddr **sal, **psal, *sa;
  struct timeval working_timeout;
  socklen_t socklen;
#if HAVE_GETTIMEOFDAY
  struct timeval limit_time, time_now;
#endif

  num_addrs = php_network_getaddresses(host, socktype, &psal, error_string TSRMLS_CC);

  if (num_addrs == 0) {
    /* could not resolve address(es) */
    return -1;
  }

  if (timeout) {
    memcpy(&working_timeout, timeout, sizeof(working_timeout));
#if HAVE_GETTIMEOFDAY
    gettimeofday(&limit_time, NULL);
    limit_time.tv_sec += working_timeout.tv_sec;
    limit_time.tv_usec += working_timeout.tv_usec;
    if (limit_time.tv_usec >= 1000000) {
      limit_time.tv_usec -= 1000000;
      limit_time.tv_sec++;
    }
#endif
  }

  for (sal = psal; !fatal && *sal != NULL; sal++) {
    sa = *sal;

    /* create a socket for this address */
    sock = socket(sa->sa_family, socktype, 0);

    if (sock == SOCK_ERR) {
      continue;
    }

    switch (sa->sa_family) {
#if HAVE_GETADDRINFO && HAVE_IPV6
      case AF_INET6:
        if (!bindto || strchr(bindto, ':')) {
          ((struct sockaddr_in6 *)sa)->sin6_family = sa->sa_family;
          ((struct sockaddr_in6 *)sa)->sin6_port = htons(port);
          socklen = sizeof(struct sockaddr_in6);
        } else {
          socklen = 0;
          sa = NULL;
        }
        break;
#endif
      case AF_INET:
        ((struct sockaddr_in *)sa)->sin_family = sa->sa_family;
        ((struct sockaddr_in *)sa)->sin_port = htons(port);
        socklen = sizeof(struct sockaddr_in);
        break;
      default:
        /* Unknown family */
        socklen = 0;
        sa = NULL;
    }

    if (sa) {
      /* make a connection attempt */

      if (bindto) {
        struct sockaddr *local_address = NULL;
        int local_address_len = 0;

        if (sa->sa_family == AF_INET) {
          struct sockaddr_in *in4 = (sockaddr_in*) emalloc(sizeof(struct sockaddr_in));

          local_address = (struct sockaddr*)in4;
          local_address_len = sizeof(struct sockaddr_in);

          in4->sin_family = sa->sa_family;
          in4->sin_port = htons(bindport);
          if (!inet_aton(bindto, &in4->sin_addr)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid IP Address: %s", bindto);
            goto skip_bind;
          }
          memset(&(in4->sin_zero), 0, sizeof(in4->sin_zero));
        }
#if HAVE_IPV6 && HAVE_INET_PTON
         else { /* IPV6 */
          struct sockaddr_in6 *in6 = (sockaddr_in6*) emalloc(sizeof(struct sockaddr_in6));

          local_address = (struct sockaddr*)in6;
          local_address_len = sizeof(struct sockaddr_in6);

          in6->sin6_family = sa->sa_family;
          in6->sin6_port = htons(bindport);
          if (inet_pton(AF_INET6, bindto, &in6->sin6_addr) < 1) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid IP Address: %s", bindto);
            goto skip_bind;
          }
        }
#endif
        if (!local_address || bind(sock, local_address, local_address_len)) {
          php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to bind to '%s:%d', system said: %s", bindto, bindport, strerror(errno));
        }
skip_bind:
        if (local_address) {
          efree(local_address);
        }
      }
      /* free error string received during previous iteration (if any) */
      if (error_string && *error_string) {
        efree(*error_string);
        *error_string = NULL;
      }

      n = php_network_connect_socket(sock, sa, socklen, asynchronous,
          timeout ? &working_timeout : NULL,
          error_string, error_code);

      if (n != -1) {
        goto connected;
      }

      /* adjust timeout for next attempt */
#if HAVE_GETTIMEOFDAY
      if (timeout) {
        gettimeofday(&time_now, NULL);

        if (timercmp(&time_now, &limit_time, >=)) {
          /* time limit expired; don't attempt any further connections */
          fatal = 1;
        } else {
          /* work out remaining time */
          sub_times(limit_time, time_now, &working_timeout);
        }
      }
#else
      if (error_code && *error_code == PHP_TIMEOUT_ERROR_VALUE) {
        /* Don't even bother trying to connect to the next alternative;
         * we have no way to determine how long we have already taken
         * and it is quite likely that the next attempt will fail too. */
        fatal = 1;
      } else {
        /* re-use the same initial timeout.
         * Not the best thing, but in practice it should be good-enough */
        if (timeout) {
          memcpy(&working_timeout, timeout, sizeof(working_timeout));
        }
      }
#endif
    }

    closesocket(sock);
  }
  sock = -1;

connected:

  php_network_freeaddresses(psal);

  return sock;
}
/* }}} */

/* {{{ php_any_addr
 * Fills the any (wildcard) address into php_sockaddr_storage
 */
PHPAPI void php_any_addr(int family, php_sockaddr_storage *addr, unsigned short port)
{
  memset(addr, 0, sizeof(php_sockaddr_storage));
  switch (family) {
#if HAVE_IPV6
  case AF_INET6: {
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) addr;
    sin6->sin6_family = AF_INET6;
    sin6->sin6_port = htons(port);
    sin6->sin6_addr = in6addr_any;
    break;
  }
#endif
  case AF_INET: {
    struct sockaddr_in *sin = (struct sockaddr_in *) addr;
    sin->sin_family = AF_INET;
    sin->sin_port = htons(port);
    sin->sin_addr.s_addr = htonl(INADDR_ANY);
    break;
  }
  }
}
/* }}} */

/* {{{ php_sockaddr_size
 * Returns the size of struct sockaddr_xx for the family
 */
PHPAPI int php_sockaddr_size(php_sockaddr_storage *addr)
{
  switch (((struct sockaddr *)addr)->sa_family) {
  case AF_INET:
    return sizeof(struct sockaddr_in);
#if HAVE_IPV6
  case AF_INET6:
    return sizeof(struct sockaddr_in6);
#endif
#ifdef AF_UNIX
  case AF_UNIX:
    return sizeof(struct sockaddr_un);
#endif
  default:
    return 0;
  }
}
/* }}} */

/* Given a socket error code, if buf == NULL:
 *   emallocs storage for the error message and returns
 * else
 *   sprintf message into provided buffer and returns buf
 */
/* {{{ php_socket_strerror */
PHPAPI char *php_socket_strerror(long err, char *buf, size_t bufsize)
{
#ifndef PHP_WIN32
  char *errstr;

  errstr = strerror(err);
  if (buf == NULL) {
    buf = estrdup(errstr);
  } else {
    strncpy(buf, errstr, bufsize);
  }
  return buf;
#else
  char *sysbuf;
  int free_it = 1;

  if (!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&sysbuf,
        0,
        NULL)) {
    free_it = 0;
    sysbuf = "Unknown Error";
  }

  if (buf == NULL) {
    buf = estrdup(sysbuf);
  } else {
    strncpy(buf, sysbuf, bufsize);
  }

  if (free_it) {
    LocalFree(sysbuf);
  }

  return buf;
#endif
}
/* }}} */

PHPAPI int php_set_sock_blocking(int socketd, int block TSRMLS_DC)
{
  int ret = SUCCESS;
  int flags;
  int myflag = 0;

#ifdef PHP_WIN32
  /* with ioctlsocket, a non-zero sets nonblocking, a zero sets blocking */
  flags = !block;
  if (ioctlsocket(socketd, FIONBIO, &flags) == SOCKET_ERROR) {
    ret = FAILURE;
  }
#else
  flags = fcntl(socketd, F_GETFL);
#ifdef O_NONBLOCK
  myflag = O_NONBLOCK; /* POSIX version */
#elif defined(O_NDELAY)
  myflag = O_NDELAY;   /* old non-POSIX version */
#endif
  if (!block) {
    flags |= myflag;
  } else {
    flags &= ~myflag;
  }
  if (fcntl(socketd, F_SETFL, flags) == -1) {
    ret = FAILURE;
  }
#endif
  return ret;
}

PHPAPI void _php_emit_fd_setsize_warning(int max_fd)
{
  TSRMLS_FETCH();

#ifdef PHP_WIN32
  php_error_docref(NULL TSRMLS_CC, E_WARNING,
    "PHP needs to be recompiled with a larger value of FD_SETSIZE.\n"
    "If this binary is from an official www.php.net package, file a bug report\n"
    "at http://bugs.php.net, including the following information:\n"
    "FD_SETSIZE=%d, but you are using %d.\n"
    " --enable-fd-setsize=%d is recommended, but you may want to set it\n"
    "to match to maximum number of sockets each script will work with at\n"
    "one time, in order to avoid seeing this error again at a later date.",
    FD_SETSIZE, max_fd, (max_fd + 128) & ~127);
#else
  php_error_docref(NULL TSRMLS_CC, E_WARNING,
    "You MUST recompile PHP with a larger value of FD_SETSIZE.\n"
    "It is set to %d, but you have descriptors numbered at least as high as %d.\n"
    " --enable-fd-setsize=%d is recommended, but you may want to set it\n"
    "to equal the maximum number of open files supported by your system,\n"
    "in order to avoid seeing this error again at a later date.",
    FD_SETSIZE, max_fd, (max_fd + 1024) & ~1023);
#endif
}

#if defined(PHP_USE_POLL_2_EMULATION)

/* emulate poll(2) using select(2), safely. */

PHPAPI int php_poll2(php_pollfd *ufds, unsigned int nfds, int timeout)
{
  fd_set rset, wset, eset;
  php_socket_t max_fd = SOCK_ERR;
  unsigned int i;
  int n;
  struct timeval tv;

  /* check the highest numbered descriptor */
  for (i = 0; i < nfds; i++) {
    if (ufds[i].fd > max_fd)
      max_fd = ufds[i].fd;
  }

  PHP_SAFE_MAX_FD(max_fd, nfds + 1);

  FD_ZERO(&rset);
  FD_ZERO(&wset);
  FD_ZERO(&eset);

  for (i = 0; i < nfds; i++) {
    if (ufds[i].events & PHP_POLLREADABLE) {
      PHP_SAFE_FD_SET(ufds[i].fd, &rset);
    }
    if (ufds[i].events & POLLOUT) {
      PHP_SAFE_FD_SET(ufds[i].fd, &wset);
    }
    if (ufds[i].events & POLLPRI) {
      PHP_SAFE_FD_SET(ufds[i].fd, &eset);
    }
  }

  if (timeout >= 0) {
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout - (tv.tv_sec * 1000)) * 1000;
  }
/* Reseting/initializing */
#ifdef PHP_WIN32
  WSASetLastError(0);
#else
  errno = 0;
#endif
  n = select(max_fd + 1, &rset, &wset, &eset, timeout >= 0 ? &tv : NULL);

  if (n >= 0) {
    for (i = 0; i < nfds; i++) {
      ufds[i].revents = 0;

      if (PHP_SAFE_FD_ISSET(ufds[i].fd, &rset)) {
        /* could be POLLERR or POLLHUP but can't tell without probing */
        ufds[i].revents |= POLLIN;
      }
      if (PHP_SAFE_FD_ISSET(ufds[i].fd, &wset)) {
        ufds[i].revents |= POLLOUT;
      }
      if (PHP_SAFE_FD_ISSET(ufds[i].fd, &eset)) {
        ufds[i].revents |= POLLPRI;
      }
    }
  }
  return n;
}

#endif


/*
 * Local variables:
 * tab-width: 8
 * c-basic-offset: 8
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
