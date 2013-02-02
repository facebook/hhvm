/*
   Demo program for libafdt.

   This test program is a simple "remote cat" implementation.  It has two
   modes: server and client.  Server mode just starts up a libafdt server
   that makes its stdout file descriptor available, then terminates after
   a specified interval.  Client mode uses libafdt to get the server's
   stdout descriptor, then copies data from its own stdin to the remote
   stdout using libevent's buffered I/O.  It terminates when it receives
   an EOF on stdin and finishes writing everything to the remote stdout.
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <event.h>
#include <afdt.h>


// Context object use for most libevent and afdt callbacks.
struct transfer_context {
  // References to the top-level servers.
  struct event_base* const eb;
  // bufferevents used by the client.
  struct bufferevent* wbufev;
  struct bufferevent* rbufev;
  // Flag indicating a transfer problem (for the client)
  int got_error;
  // Current role of the process with respect to AFDT.
  const char* role;
  // The state of the clients I/O buffers.
  int got_eof;
  int flushed_out;
};

void maybe_break(struct transfer_context* ctx) {
  if (ctx->got_eof && ctx->flushed_out) {
    event_base_loopbreak(ctx->eb);
  }
}

void bufferevent_error(struct bufferevent* bufev, short what, void* arg) {
  (void)bufev;
  struct transfer_context* ctx = (struct transfer_context*)arg;
  if (what == (EVBUFFER_READ | EVBUFFER_EOF)) {
    ctx->got_eof = 1;
    maybe_break(ctx);
  } else {
    fprintf(stderr, "BUFFEREVENT ERROR: what=%hd\n", what);
    abort();
  }
}

void bufferevent_rcb(struct bufferevent* bufev, void* arg) {
  struct transfer_context* ctx = (struct transfer_context*)arg;
  int ret = bufferevent_write_buffer(ctx->wbufev, bufev->input);
  assert(ret >= 0);
  ctx->flushed_out = 0;
}

void bufferevent_wcb(struct bufferevent* bufev, void* arg) {
  (void)bufev;
  struct transfer_context* ctx = (struct transfer_context*)arg;
  ctx->flushed_out = 1;
  maybe_break(ctx);
}

void fd_transfer_error_hander(
    const struct afdt_error_t* err,
    void* userdata) {
  struct transfer_context* ctx = (struct transfer_context*)userdata;
  fprintf(stderr, "AFDT ERROR: role=%s phase=%s operation=%s message=\"%s\" errno=\"%s\"\n",
      ctx->role, afdt_phase_str(err->phase), afdt_operation_str(err->operation), err->message, strerror(errno));
  if (strcmp(ctx->role, "client") == 0) {
    ctx->got_error = 1;
    event_base_loopbreak(ctx->eb);
  }
}

int fd_transfer_request_handler(
    const uint8_t* request,
    uint32_t request_length,
    uint8_t* response,
    uint32_t* response_length,
    void* userdata) {
  (void)request;
  (void)request_length;
  (void)response;
  (void)userdata;
  *response_length = 0;
  return STDOUT_FILENO;
}

void fd_transfer_response_handler(
    const uint8_t* response,
    uint32_t response_length,
    int received_fd,
    void* userdata) {
  (void)response;
  (void)response_length;
  struct transfer_context* ctx = (struct transfer_context*)userdata;

  if (received_fd < 0) {
    fputs("Did not receive file descriptor\n", stderr);
    abort();
  }

  ctx->wbufev = bufferevent_new(received_fd, NULL, bufferevent_wcb, bufferevent_error, userdata);
  assert(ctx->wbufev != NULL);
  bufferevent_base_set(ctx->eb, ctx->wbufev);
  bufferevent_enable(ctx->wbufev, EV_WRITE);

  ctx->rbufev = bufferevent_new(STDIN_FILENO, bufferevent_rcb, NULL, bufferevent_error, userdata);
  assert(ctx->rbufev != NULL);
  bufferevent_base_set(ctx->eb, ctx->rbufev);
  bufferevent_enable(ctx->rbufev, EV_READ);
}


int main(int argc, char* argv[]) {
  const char* transfer_fname = "./fd_transfer_sock";
  int transfer_timeout_usec = -1;
  int lifetime_sec = 1;
  int lifetime_usec = 1;
  bool is_server = false;
  int opt;
  while ((opt = getopt(argc, argv, "csf:t:L:l:")) != -1) {
    switch (opt) {
      case 'c':
        is_server = 0;
        break;
      case 's':
        is_server = true;
        break;
      case 'f':
        transfer_fname = optarg;
        break;
      case 't':
        transfer_timeout_usec = atoi(optarg);
        break;
      case 'L':
        lifetime_sec = atoi(optarg);
        break;
      case 'l':
        lifetime_usec = atoi(optarg);
        break;
      case '?':
        fprintf(stderr,
            "usage: %s [options]\n"
            "  options:\n"
            "    [-c] # client mode\n"
            "    [-s] # server mode\n"
            "    [-f transfer_fname]\n"
            "    [-t transfer_timeout_usec]\n"
            "    [-L lifetime_sec]\n"
            "    [-l lifetime_usec]\n"
            ,
            argv[0]);
        exit(EXIT_FAILURE);
        break;
      default:
        fprintf(stderr, "unexpected return from getopt: %d\n", opt);
        exit(EXIT_FAILURE);
    }
  }

  int ret;
  struct event_base* eb = event_base_new();
  assert(eb != NULL);

  struct transfer_context transfer_ctx = {
    .eb = eb,
    .got_error = 0,
    .wbufev = NULL,
    .rbufev = NULL,
    .got_eof = 0,
    .flushed_out = 0,
  };

  void* server_handle = NULL;

  if (is_server) {
    transfer_ctx.role = "server";
    int ret;
    ret = unlink(transfer_fname);
    // TODO(dreiss): This is not a reasonable assert
    assert(ret >= 0 || errno == ENOENT);
    ret = afdt_create_server(
        transfer_fname,
        transfer_ctx.eb,
        fd_transfer_request_handler,
        afdt_no_post,
        fd_transfer_error_hander,
        &server_handle,
        &transfer_ctx);
    assert(ret >= 0);
    struct timeval timeout = { .tv_sec = lifetime_sec, .tv_usec = lifetime_usec };
    ret = event_base_loopexit(eb, &timeout);
    assert(ret >= 0);
  } else {
    transfer_ctx.role = "client";
    struct timeval timeout = { .tv_sec = 0, .tv_usec = transfer_timeout_usec };
    ret = afdt_create_client(
        transfer_fname,
        transfer_ctx.eb,
        (const uint8_t*)"",
        0,
        fd_transfer_response_handler,
        fd_transfer_error_hander,
        transfer_timeout_usec >= 0 ? &timeout : NULL,
        &transfer_ctx);
    if (ret < 0) {
      exit(EXIT_FAILURE);
    }
  }

  ret = event_base_dispatch(eb);
  assert(ret >= 0);
  if (server_handle != NULL) {
    ret = afdt_close_server(server_handle);
    assert(ret >= 0);
  }
  if (transfer_ctx.wbufev) bufferevent_free(transfer_ctx.wbufev);
  if (transfer_ctx.rbufev) bufferevent_free(transfer_ctx.rbufev);
  event_base_free(eb);

  if (transfer_ctx.got_error) {
    exit(EXIT_FAILURE);
  }

  return 0;
}
