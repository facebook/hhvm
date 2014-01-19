/*
   Demo server for libafdt.

   This test program is an http server based on evhttp.  It uses libafdt to
   transfer its listen/accept socket between instances of the server, allowing
   for zero-downtime upgrades.  Each instance of the server also listens on an
   "administrative port", allowing it to receive administrative commands.

   The server supports four URLs:
     /status:      Report server identity, defined on the command line.
     /shutdown:    Shut down the server after a short delay.
     /bind_prod:   Bind to the "production port", or request the production
                   listen/accept socket from an existing server if the bind
                   fails.
     /close_prod:  Close the production socket.
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <sys/queue.h>
#include <evhttp.h>
#include <afdt.h>


#define SHUTDOWN_DELAY_USEC ((1.0/16)*1000*1000)
#define REQUEST_CONTENT "abc123"
#define RESPONSE_CONTENT "987zyx"


// Context object use for most libevent and afdt callbacks.
struct transfer_context {
  // References to the top-level servers.
  struct event_base* const eb;
  struct evhttp* const eh;
  // /bind_prod request.  This doesn't allow more than one concurrent request.
  struct evhttp_request* request;
  // Port to use for serving "production" requests.
  const int prod_port;
  // Filename to use for AFDT socket.
  const char* const transfer_fname;
  // Timeout when waiting for AFDT response.
  const int transfer_timeout_usec;
  // Socket handle for dropping the "production" socket.
  struct evhttp_bound_socket* prod_handle;
  // File descriptor used to accept "production" requests.
  int prod_fd;
  // Handle used to shut down file descriptor server.
  void* server_handle;
  // Current role of the process with respect to AFDT.
  const char* role;
};


void send_const_reply(struct evhttp_request* request, int code, const char* content) {
  const char* reason = "";
  switch (code) {
    case 200: reason = "OK"; break;
    case 500: reason = "Internal Server Error"; break;
  }
  struct evbuffer* buf = evbuffer_new();
  assert(buf != NULL);
  int ret = evbuffer_add(buf, content, strlen(content));
  assert(ret == 0);
  evhttp_send_reply(request, code, reason, buf);
  evbuffer_free(buf);
}

void status_handler(struct evhttp_request* request, void* arg) {
  const char* status_message = (const char*)arg;
  send_const_reply(request, 200, status_message);
}

void shutdown_handler(struct evhttp_request* request, void* arg) {
  send_const_reply(request, 200, "shutting_down");

  struct event_base* eb = (struct event_base*)arg;
  struct timeval timeout = { .tv_sec = 0, .tv_usec = SHUTDOWN_DELAY_USEC };
  int ret = event_base_loopexit(eb, &timeout);
  assert(ret == 0);
}


void fd_transfer_error_hander(
    const struct afdt_error_t* err,
    void* userdata) {
  struct transfer_context* ctx = (struct transfer_context*)userdata;
  fprintf(stderr, "AFDT ERROR: role=%s phase=%s operation=%s message=\"%s\" errno=\"%s\"\n",
      ctx->role, afdt_phase_str(err->phase), afdt_operation_str(err->operation), err->message, strerror(errno));
  if (strcmp(ctx->role, "client") == 0) {
    send_const_reply(ctx->request, 500, "afdt_fail");
    ctx->request = NULL;
  }
}

int fd_transfer_request_handler(
    const uint8_t* request,
    uint32_t request_length,
    uint8_t* response,
    uint32_t* response_length,
    void* userdata) {
  int fd_to_transfer = -1;
  const char* resp;
  if (request_length != strlen(REQUEST_CONTENT) ||
      memcmp(request, REQUEST_CONTENT, request_length) != 0) {
    resp = "invalid request";
  } else {
    struct transfer_context* ctx = (struct transfer_context*)userdata;
    fd_to_transfer = ctx->prod_fd;
    resp = RESPONSE_CONTENT;
  }
  assert(strlen(resp) < (size_t)*response_length);
  memcpy(response, resp, strlen(resp));
  *response_length = strlen(resp);
  return fd_to_transfer;
}

void setup_prod_fd_server(struct transfer_context* ctx, const char* source) {
  int ret;
  if (ctx->server_handle != NULL) {
    ret = afdt_close_server(ctx->server_handle);
    assert(ret >= 0);
  }
  ret = unlink(ctx->transfer_fname);
  // TODO(dreiss): This is not a reasonable assert
  assert(ret >= 0 || errno == ENOENT);
  ctx->role = "server";
  ret = afdt_create_server(
      ctx->transfer_fname,
      ctx->eb,
      fd_transfer_request_handler,
      afdt_no_post,
      fd_transfer_error_hander,
      &ctx->server_handle,
      ctx);
  assert(ret >= 0);
  send_const_reply(ctx->request, 200, source);
  ctx->request = NULL;
}

void fd_transfer_response_handler(
    const uint8_t* response,
    uint32_t response_length,
    int received_fd,
    void* userdata) {
  struct transfer_context* ctx = (struct transfer_context*)userdata;
  if (response_length != strlen(RESPONSE_CONTENT) ||
      memcmp(response, RESPONSE_CONTENT, response_length) != 0) {
    fputs("Unexpected response:\n", stderr);
    fwrite(response, 1, response_length, stderr);
    fputc('\n', stderr);
    abort();
  }
  if (received_fd < 0) {
    fputs("Did not receive file descriptor\n", stderr);
    abort();
  }

  ctx->prod_fd = received_fd;
  ctx->prod_handle = evhttp_accept_socket_with_handle(ctx->eh, received_fd);
  assert(ctx->prod_handle != NULL);

  setup_prod_fd_server(ctx, "afdt");
}

void bind_prod_handler(struct evhttp_request* request, void* arg) {
  struct transfer_context* ctx = (struct transfer_context*)arg;
  int ret;

  if (ctx->prod_fd >= 0) {
    send_const_reply(request, 200, "already_open");
    return;
  }
  if (ctx->request != NULL) {
    send_const_reply(request, 500, "already_acquiring");
    return;
  }

  ctx->prod_handle = evhttp_bind_socket_with_handle(ctx->eh, NULL, ctx->prod_port);
  if (ctx->prod_handle != NULL) {
    ctx->prod_fd = evhttp_bound_socket_get_fd(ctx->prod_handle);
    ctx->request = request;
    setup_prod_fd_server(ctx, "bind");
  } else if (errno == EADDRINUSE) {
    ctx->request = request;
    ctx->role = "client";
    struct timeval timeout = {
      .tv_sec = 0, .tv_usec = ctx->transfer_timeout_usec };
    ret = afdt_create_client(
        ctx->transfer_fname,
        ctx->eb,
        (const uint8_t*)REQUEST_CONTENT,
        strlen(REQUEST_CONTENT),
        fd_transfer_response_handler,
        fd_transfer_error_hander,
        ctx->transfer_timeout_usec >= 0 ? &timeout : NULL,
        ctx);
    // Any errors are handled by the error callback.
    (void)ret;
  } else {
    send_const_reply(request, 500, "bind_fail");
  }
}

void close_prod_handler(struct evhttp_request* request, void* arg) {
  struct transfer_context* ctx = (struct transfer_context*)arg;
  int ret;

  if (ctx->prod_handle == NULL) {
    send_const_reply(request, 200, "no_prod");
    return;
  }

  evhttp_del_accept_socket(ctx->eh, ctx->prod_handle);
  ctx->prod_handle = NULL;

  ret = close(ctx->prod_fd);
  ctx->prod_fd = -1;
  if (ret < 0) {
    send_const_reply(request, 500, "close_fail");
    return;
  }

  send_const_reply(request, 200, "closed");
}


int main(int argc, char* argv[]) {
  int prod_port = 8080;
  int admin_port = 9090;
  const char* status_message = "UNDEFINED";
  const char* transfer_fname = "./fd_transfer_sock";
  int transfer_timeout_usec = -1;
  int opt;
  while ((opt = getopt(argc, argv, "p:a:s:f:t:")) != -1) {
    switch (opt) {
      case 'p':
        prod_port = atoi(optarg);
        break;
      case 'a':
        admin_port = atoi(optarg);
        break;
      case 's':
        status_message = optarg;
        break;
      case 'f':
        transfer_fname = optarg;
        break;
      case 't':
        transfer_timeout_usec = atoi(optarg);
        break;
      case '?':
        fprintf(stderr,
            "usage: %s [options]\n"
            "  options:\n"
            "    [-p prod_port]\n"
            "    [-a admin_port]\n"
            "    [-s status_message]\n"
            "    [-f transfer_fname]\n"
            "    [-t transfer_timeout_usec]\n"
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
  struct evhttp* eh = evhttp_new(eb);
  assert(eh != NULL);
  ret = evhttp_bind_socket(eh, NULL, admin_port);
  assert(ret >= 0);

  struct transfer_context transfer_ctx = {
    .eb = eb,
    .eh = eh,
    .request = NULL,
    .prod_port = prod_port,
    .transfer_fname = transfer_fname,
    .transfer_timeout_usec = transfer_timeout_usec,
    .prod_handle = NULL,
    .prod_fd = -1,
    .server_handle = NULL,
  };

  evhttp_set_cb(eh, "/status", status_handler, (void*)status_message);
  evhttp_set_cb(eh, "/shutdown", shutdown_handler, (void*)eb);
  evhttp_set_cb(eh, "/bind_prod", bind_prod_handler, (void*)&transfer_ctx);
  evhttp_set_cb(eh, "/close_prod", close_prod_handler, (void*)&transfer_ctx);

  ret = event_base_dispatch(eb);
  assert(ret >= 0);

  if (transfer_ctx.server_handle != NULL) {
    ret = afdt_close_server(transfer_ctx.server_handle);
    assert(ret >= 0);
  }
  evhttp_free(eh);
  event_base_free(eb);

  return 0;
}
