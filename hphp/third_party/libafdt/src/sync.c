#include <assert.h>
#include <poll.h>
#include <sys/time.h>

#include "config.h"
#include "afdt.h"
#include "internal.h"

int afdt_sync_client(
    const char* fname,
    const uint8_t* request,
    uint32_t request_length,
    uint8_t* response,
    uint32_t* response_length,
    int* received_fd,
    const struct timeval* timeout,
    struct afdt_error_t* err) {
  int ret;

  ret = afdt_connect(fname, err);
  if (ret < 0) {
    return ret;
  }
  int connfd = ret;

  ret = afdt_send_plain_msg(connfd, request, request_length, err);
  if (ret < 0) {
    return ret;
  }

  if (timeout != NULL) {
    struct pollfd pfd = { .fd = connfd, .events = POLLIN };
    int timeout_ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
    ret = poll(&pfd, 1, timeout_ms);
    if (ret < 0) {
      set_error(err, AFDT_POLL, "");
      return ret;
    } else if (ret == 0) {
      assert(!(pfd.revents & POLLIN));
      set_error(err, AFDT_TIMEOUT, "");
      return -1;
    }
    assert(ret == 1);
    assert(pfd.revents & POLLIN);
  }

  ret = afdt_recv_fd_msg(connfd, response, response_length, received_fd, err);
  if (ret < 0) {
    return ret;
  }

  return 0;
}
