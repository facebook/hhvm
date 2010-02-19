// libevent needs this for u_char
#define _BSD_SOURCE

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <event.h>

#include "config.h"
#include "afdt.h"
#include "internal.h"


// Older versions of libevent take their timeout as a non-const pointer.
// However, the contents are not modified, so it is safe to const_cast.
#if HAVE_LIBEVENT_145
# define EVENT_TIMEOUT_CAST
#else
# define EVENT_TIMEOUT_CAST (struct timeval *)
#endif


/**
 * Structure passed to all of our libevent callbacks.
 * It's a little overloaded because we use it for both clients and servers.
 */
struct ev_arg {
  struct event_base* eb;
  struct event ev;
  afdt_request_handler_t request_handler;
  afdt_response_handler_t response_handler;
  afdt_post_handler_t post_handler;
  afdt_error_handler_t error_handler;
  void* afdt_userdata;
};


void afdt_no_post(
    const uint8_t* request,
    uint32_t request_length,
    const uint8_t* response,
    uint32_t response_length,
    int sent_fd,
    void* userdata) {
  (void)request;
  (void)request_length;
  (void)response;
  (void)response_length;
  (void)sent_fd;
  (void)userdata;
}


// event_base_set + event_add
static int event_bsa(struct event_base* eb, struct event* ev, const struct timeval* timeout, enum afdt_operation* err_op) {
  int ret;
  errno = 0;

  ret = event_base_set(eb, ev);
  if (ret < 0) {
    *err_op = AFDT_EVENT_BASE_SET;
    return ret;
  }

  ret = event_add(ev, EVENT_TIMEOUT_CAST timeout);
  if (ret < 0) {
    *err_op = AFDT_EVENT_ADD;
    return ret;
  }

  return 0;
}


// libevent callback for a server when a client socket is readable.
// Receives the request, calls the request handler, sends the response,
// and calls the post handler.
static void server_handle_client_read(int clientfd, short event_type, void* arg) {
  assert(event_type == EV_READ);

  int ret;
  struct ev_arg* ev_userdata = arg;

  struct afdt_error_t err = AFDT_ERROR_T_INIT;
  err.phase = AFDT_HANDLE_REQUEST;

  uint8_t req_buf[AFDT_MSGLEN];
  uint8_t res_buf[AFDT_MSGLEN];

  uint32_t req_len = sizeof(req_buf);
  ret = afdt_recv_plain_msg(clientfd, req_buf, &req_len, &err);
  if (ret < 0) {
    ev_userdata->error_handler(&err, ev_userdata->afdt_userdata);
    goto drop_client;
  }

  uint32_t res_len = sizeof(res_buf);
  int fd_to_send = ev_userdata->request_handler(
      req_buf,
      req_len,
      res_buf,
      &res_len,
      ev_userdata->afdt_userdata
      );
  assert((size_t)res_len <= AFDT_MSGLEN);

  ret = afdt_send_fd_msg(clientfd, res_buf, res_len, fd_to_send, &err);
  if (ret < 0) {
    ev_userdata->error_handler(&err, ev_userdata->afdt_userdata);
  } else {
    ev_userdata->post_handler(
        req_buf,
        req_len,
        res_buf,
        res_len,
        fd_to_send,
        ev_userdata->afdt_userdata);
  }

drop_client:
  close(clientfd);
  free(arg);
}


// libevent callback for a server when the accept socket is readable.
// Accepts a client, allocates a new context for the connection,
// and sets up a callback to wait for a request.
static void server_handle_accept_read(int acceptfd, short event_type, void* arg) {
  assert(event_type == EV_READ);

  int ret;
  struct ev_arg* ev_userdata = arg;

  struct afdt_error_t err = AFDT_ERROR_T_INIT;
  err.phase = AFDT_ACCEPT_CLIENT;

  enum afdt_operation err_op = AFDT_NO_OPERATION;

  ret = accept(acceptfd, NULL, NULL);
  if (ret < 0) {
    err_op = AFDT_ACCEPT;
    goto accept_failed;
  }
  int clientfd = ret;

  struct ev_arg* new_userdata = malloc(sizeof(*ev_userdata));
  if (new_userdata == NULL) {
    err_op = AFDT_MALLOC;
    goto malloc_failed;
  }
  new_userdata->eb = ev_userdata->eb;
  new_userdata->request_handler = ev_userdata->request_handler;
  new_userdata->response_handler = NULL;
  new_userdata->post_handler = ev_userdata->post_handler;
  new_userdata->error_handler = ev_userdata->error_handler;
  new_userdata->afdt_userdata = ev_userdata->afdt_userdata;

  event_set(&new_userdata->ev, clientfd, EV_READ, server_handle_client_read, new_userdata);
  ret = event_bsa(new_userdata->eb, &new_userdata->ev, NULL, &err_op);
  if (ret < 0) {
    goto event_failed;
  }

  return;

  int errno_save;
event_failed:
  errno_save = errno;
  free(new_userdata);
  errno = errno_save;
malloc_failed:
  errno_save = errno;
  close(clientfd);
  errno = errno_save;
accept_failed:
  set_error(&err, err_op, "");
  ev_userdata->error_handler(&err, ev_userdata->afdt_userdata);
}


int afdt_create_server(
    const char* fname,
    struct event_base* eb,
    afdt_request_handler_t request_handler,
    afdt_post_handler_t post_handler,
    afdt_error_handler_t error_handler,
    void** out_close_handle,
    void* userdata
    ) {
  int ret;

  struct afdt_error_t err = AFDT_ERROR_T_INIT;
  err.phase = AFDT_CREATE_SERVER;


  struct ev_arg* ev_userdata = malloc(sizeof(*ev_userdata));
  if (ev_userdata == NULL) {
    set_error(&err, AFDT_MALLOC, "");
    error_handler(&err, userdata);
    return -1;
  }
  ev_userdata->eb = eb;
  ev_userdata->request_handler = request_handler;
  ev_userdata->response_handler = NULL;
  ev_userdata->post_handler = post_handler;
  ev_userdata->error_handler = error_handler;
  ev_userdata->afdt_userdata = userdata;


  ret = afdt_listen(fname, &err);
  if (ret < 0) {
    goto destruct;
  }
  int acceptfd = ret;

  enum afdt_operation err_op = AFDT_NO_OPERATION;
  event_set(&ev_userdata->ev, acceptfd, EV_READ | EV_PERSIST, server_handle_accept_read, ev_userdata);
  ret = event_bsa(eb, &ev_userdata->ev, NULL, &err_op);
  if (ret < 0) {
    set_error(&err, err_op, "");
    goto destruct;
  }

  if (out_close_handle != NULL) {
    *out_close_handle = ev_userdata;
  }
  return 0;

destruct:
  error_handler(&err, userdata);
  free(ev_userdata);
  return ret;
}


int afdt_close_server(void* close_handle) {
  int ret;
  struct ev_arg* ev_userdata = close_handle;
  errno = 0;
  ret = event_del(&ev_userdata->ev);
  if (ret < 0) {
    return ret;
  }
  ret = close(EVENT_FD(&ev_userdata->ev));
  if (ret < 0) {
    return ret;
  }
  free(close_handle);
  return 0;
}


// libevent callback for a client when the server socket is readable.
// Receives the response and calls the response handler.
static void client_handle_read(int connfd, short event_type, void* arg) {
  int ret;
  struct ev_arg* ev_userdata = arg;

  struct afdt_error_t err = AFDT_ERROR_T_INIT;
  err.phase = AFDT_HANDLE_RESPONSE;

  if (event_type == EV_TIMEOUT) {
    errno = 0;
    set_error(&err, AFDT_TIMEOUT, "");
    ev_userdata->error_handler(&err, ev_userdata->afdt_userdata);
    goto drop_conn;
  }

  assert(event_type == EV_READ);

  uint8_t res_buf[AFDT_MSGLEN];
  uint32_t res_len = sizeof(res_buf);

  int got_fd;
  ret = afdt_recv_fd_msg(connfd, res_buf, &res_len, &got_fd, &err);

  if (ret < 0) {
    ev_userdata->error_handler(&err, ev_userdata->afdt_userdata);
  } else {
    ev_userdata->response_handler(
        res_buf,
        res_len,
        got_fd,
        ev_userdata->afdt_userdata);
  }

drop_conn:
  close(connfd);
  free(arg);
}


int afdt_create_client(
    const char* fname,
    struct event_base* eb,
    const uint8_t* request,
    uint32_t request_length,
    afdt_response_handler_t response_handler,
    afdt_error_handler_t error_handler,
    const struct timeval* timeout,
    void* userdata) {
  int ret;

  struct afdt_error_t err = AFDT_ERROR_T_INIT;
  err.phase = AFDT_CREATE_CLIENT;

  struct ev_arg* ev_userdata = malloc(sizeof(*ev_userdata));
  if (ev_userdata == NULL) {
    set_error(&err, AFDT_MALLOC, "");
    error_handler(&err, userdata);
    return -1;
  }
  ev_userdata->eb = eb;
  ev_userdata->request_handler = NULL;
  ev_userdata->response_handler = response_handler;
  ev_userdata->post_handler = NULL;
  ev_userdata->error_handler = error_handler;
  ev_userdata->afdt_userdata = userdata;

  ret = afdt_connect(fname, &err);
  if (ret < 0) {
    goto socket_failed;
  }
  int connfd = ret;

  ret = afdt_send_plain_msg(connfd, request, request_length, &err);

  if (ret < 0) {
    goto write_failed;
  }

  enum afdt_operation err_op = AFDT_NO_OPERATION;
  event_set(&ev_userdata->ev, connfd, EV_READ, client_handle_read, ev_userdata);
  ret = event_bsa(eb, &ev_userdata->ev, timeout, &err_op);
  if (ret < 0) {
    set_error(&err, err_op, "");
    goto event_failed;
  }

  return 0;

  int errno_save;
write_failed:
event_failed:
  errno_save = errno;
  close(connfd);
  errno = errno_save;
socket_failed:
  error_handler(&err, userdata);
  free(ev_userdata);
  return ret;
}
