#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "config.h"
#include "afdt.h"
#include "internal.h"


#if !HAVE_DECL_AF_LOCAL
# define AF_LOCAL AF_UNIX
#endif
#if !HAVE_DECL_PF_LOCAL
# define PF_LOCAL PF_UNIX
#endif


// listen queue length
#define BACKLOG 2


size_t afdt_strlcpy(char *dst, const char *src, size_t siz);


/**
 * Callback function to prepare a socket for some operation.
 *
 * @return 0 on success, <0 on failure.
 */
typedef int (*socket_prep_func_t)(int sockfd, struct sockaddr* addr, socklen_t addr_len, struct afdt_error_t* err);

/**
 * Create a Unix domain socket and prepare it to communicate,
 * either by binding it to an address and asking it to listen
 * or connecting it to an existing socket.
 *
 * @return 0 on success, <0 on failure.
 */
static int create_and_prep_socket(const char* fname, socket_prep_func_t prep, struct afdt_error_t* err) {
  int ret;
  struct sockaddr_un addr;
  int addr_len;

  ret = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (ret < 0) {
    set_error(err, AFDT_SOCKET, "");
    return ret;
  }
  int sockfd = ret;

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_LOCAL;
  size_t fname_len = afdt_strlcpy(addr.sun_path, fname, sizeof(addr.sun_path));
  addr_len = sizeof(sa_family_t) + fname_len + 1;
  if (fname_len >= sizeof(addr.sun_path)) {
    ret = -1;
    errno = ENAMETOOLONG;
    set_error(err, AFDT_PATHNAME, "");
    goto destruct;
  }

  ret = prep(sockfd, (struct sockaddr *)&addr, addr_len, err);
  if (ret < 0) {
    goto destruct;
  }

  return sockfd;

  int errno_save;
destruct:
  errno_save = errno;
  close(sockfd);
  errno = errno_save;
  return ret;
}


static int prep_accept_socket(int sockfd, struct sockaddr* addr, socklen_t addr_len, struct afdt_error_t* err) {
  int ret;

  ret = bind(sockfd, (struct sockaddr *)addr, addr_len);
  if (ret < 0) {
    set_error(err, AFDT_BIND, "");
    return ret;
  }

  ret = listen(sockfd, BACKLOG);
  if (ret < 0) {
    set_error(err, AFDT_LISTEN, "");
    return ret;
  }

  return 0;
}


static int prep_connect_socket(int sockfd, struct sockaddr* addr, socklen_t addr_len, struct afdt_error_t* err) {
  int ret;

  // blocking connect is probably okay for PF_LOCAL with a listen backlog
  ret = connect(sockfd, (struct sockaddr *)addr, addr_len);
  if (ret < 0) {
    set_error(err, AFDT_CONNECT, "");
    return ret;
  }

  return 0;
}


int afdt_listen(const char* fname, struct afdt_error_t* err) {
  return create_and_prep_socket(fname, prep_accept_socket, err);
}


int afdt_connect(const char* fname, struct afdt_error_t* err) {
  return create_and_prep_socket(fname, prep_connect_socket, err);
}


int afdt_send_fd_msg(
    int connfd, 
    const uint8_t* content,
    uint32_t content_len,
    int fd_to_send,
    struct afdt_error_t* err) {
  if (content_len > AFDT_MSGLEN) {
    errno = 0;
    set_error(err, AFDT_FORMAT, "message too long");
    return -1;
  }

  struct iovec iov[2];
  struct msghdr msg;
  char cmsg_buf[CMSG_LEN(sizeof(int))];
  struct cmsghdr* cmsg = (struct cmsghdr *)cmsg_buf;

  // Using host byte order is okay since this is always a local connection.
  iov[0].iov_base = &content_len;
  iov[0].iov_len = sizeof(content_len);
  iov[1].iov_base = (void*)content;
  iov[1].iov_len = content_len;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = iov;
  msg.msg_iovlen = 2;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  if (fd_to_send < 0) {
    msg.msg_control    = NULL;
    msg.msg_controllen = 0;
  } else {
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = sizeof(cmsg_buf);
    msg.msg_control = cmsg;
    msg.msg_controllen = cmsg->cmsg_len;
    memcpy(CMSG_DATA(cmsg), &fd_to_send, sizeof(int));
  }

  errno = 0;
  ssize_t nwritten = sendmsg(connfd, &msg, 0);
  if (nwritten < 0) {
    set_error(err, AFDT_SENDMSG, "");
    return -1;
  } else if ((size_t)nwritten != sizeof(content_len) + content_len) {
    set_error(err, AFDT_SENDMSG, "short write");
    return -1;
  }

  return 0;
}


int afdt_send_plain_msg(
    int connfd,
    const uint8_t* content,
    uint32_t content_len,
    struct afdt_error_t* err) {
  return afdt_send_fd_msg(connfd, content, content_len, -1, err);
}


int afdt_recv_fd_msg(
    int connfd,
    uint8_t* content,
    uint32_t* content_len,
    int* received_fd,
    struct afdt_error_t* err) {
  *received_fd = -1;

  struct msghdr msg;
  struct iovec iov[2];
  char cmsg_buf[CMSG_LEN(sizeof(int))];
  struct cmsghdr* cmsg = (struct cmsghdr *)cmsg_buf;

  // Using host byte order is okay since this is always a local connection.
  iov[0].iov_base = content_len;
  iov[0].iov_len = sizeof(*content_len);
  iov[1].iov_base = content;
  iov[1].iov_len = *content_len;
  msg.msg_iov = iov;
  msg.msg_iovlen = 2;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_control = cmsg;
  msg.msg_controllen = sizeof(cmsg_buf);

  errno = 0;
  ssize_t nread = recvmsg(connfd, &msg, 0);
  if (nread < 0) {
    set_error(err, AFDT_RECVMSG, "");
    return -1;
  }

  // recvmsg was successful.  Check for a received file descriptor early
  // so that we don't leak it if we got a garbled message.
  if (msg.msg_controllen != 0) {
    // TODO(dreiss): This first comparison should be "!=",
    // but on 64-bit Linux it is coming back as 24,
    // while sizeof(cmsg_buf) is 20.  Investigate further.
    if (msg.msg_controllen < sizeof(cmsg_buf) ||
        cmsg->cmsg_len != sizeof(cmsg_buf) ||
        cmsg->cmsg_level != SOL_SOCKET ||
        cmsg->cmsg_type != SCM_RIGHTS) {
      // We got an unexpected control message back.
      // This is weird, but we don't report an error since
      // we might still have gotten a reasonable response.
      } else {
        memcpy(received_fd, CMSG_DATA(cmsg), sizeof(int));
    }
  }

  if (nread == 0) {
    set_error(err, AFDT_RECVMSG, "got empty message or EOF");
    return -1;
  } else if (nread < (ssize_t)sizeof(*content_len)) {
    set_error(err, AFDT_FORMAT, "message too short for header");
    return -1;
  }

  if ((size_t)nread != sizeof(*content_len) + *content_len) {
    set_error(err, AFDT_FORMAT, "message header gives wrong length");
    return -1;
  }

  return 0;
}


int afdt_recv_plain_msg(
    int connfd,
    uint8_t* content,
    uint32_t* content_len,
    struct afdt_error_t* err) {
  int got_fd;
  int ret = afdt_recv_fd_msg(connfd, content, content_len, &got_fd, err);
  if (got_fd >= 0) {
    int errno_save = errno;
    close(ret);
    errno = errno_save;
  }
  return ret;
}
