/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/server/takeover-agent.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/ext/ext_string.h"
#include "folly/String.h"
#include <afdt.h>

/*
TakeoverAgent provides the ability
to transfer the accept socket (the file descriptor used to accept
new connections) from an older instance of the server to a new one
that has just been brought up.

In takeover, the agent will attempt to
use libafdt to transfer a file descriptor from an existing process
(which should exist, since we couldn't bind to the socket).
The transfer is performed in a separate event loop that we wait on,
so it is effectively synchronous.  If we fail to get the socket,
we just return up to higher-level code (the HttpServer class will
then try to kill the existing server).  If we do get the socket,
we send another request to the existing server to make it shut down
its main port, then we set up our own file descriptor server so
we can give the socket to the next instance that starts.

It is a little bit of a hack to use libafdt to send the shutdown
request, but we need to synchronously shut down the admin server,
so we cannot use the admin server for it.
*/

// We use a very simple protocol for communicating over libafdt:
// One byte for the protocol version and a second code byte.
#define P_VERSION  "\x01"
#define C_FD_REQ   "\x02"
#define C_TERM_REQ "\x03"
#define C_FD_RESP  "\x04"
#define C_TERM_OK  "\x05"
#define C_TERM_BAD "\x06"
#define C_UNKNOWN  "\x07"

namespace HPHP {

static void fd_transfer_error_hander(
    const struct afdt_error_t* err,
    void* userdata) {
  (void)userdata;
  Logger::Error(
      "AFDT ERROR: phase=%s operation=%s "
      "message=\"%s\" errno=\"%s\"",
      afdt_phase_str(err->phase),
      afdt_operation_str(err->operation),
      err->message,
      folly::errnoStr(errno).c_str());
}

static int fd_transfer_request_handler(
    const uint8_t* request,
    uint32_t request_length,
    uint8_t* response,
    uint32_t* response_length,
    void* userdata) {
  TakeoverAgent* agent = (TakeoverAgent*)userdata;
  String req((const char*)request, request_length, CopyString);
  String resp;
  int fd = agent->afdtRequest(req, &resp);
  assert(resp.size() <= (int)*response_length);
  memcpy(response, resp.data(), resp.size());
  *response_length = resp.size();
  return fd;
}

TakeoverAgent::TakeoverAgent(const std::string &fname)
  : m_delete_handle(nullptr),
    m_transfer_fname(fname),
    m_took_over(false),
    m_takeover_state(TakeoverState::NotStarted)
{
}

const StaticString
  s_ver_C_FD_REQ(P_VERSION C_FD_REQ),
  s_ver_C_TERM_REQ(P_VERSION C_TERM_REQ),
  s_ver_C_TERM_OK(P_VERSION C_TERM_OK);

int TakeoverAgent::afdtRequest(String request, String* response) {
  Logger::Info("takeover: received request");
  if (request == s_ver_C_FD_REQ) {
    Logger::Info("takeover: request is a listen socket request");
    *response = P_VERSION C_FD_RESP;
    m_takeover_state = TakeoverState::Started;
    (void)m_callback->onTakeoverRequest(RequestType::LISTEN_SOCKET);
    return m_sock;
  } else if (request == s_ver_C_TERM_REQ) {
    Logger::Info("takeover: request is a terminate request");
    // It is a little bit of a hack to use an AFDT request/response
    // to shut down our accept socket, but it has to be done from
    // within the main libevent thread.
    int ret;
    *response = P_VERSION C_TERM_BAD;
    if (m_callback->onTakeoverRequest(RequestType::TERMINATE) != 0) {
      return -1;
    }
    ret = afdt_close_server(m_delete_handle);
    if (ret < 0) {
      Logger::Error("Unable to close afdt server");
      return -1;
    }
    m_delete_handle = nullptr;
    m_takeover_state = TakeoverState::Complete;

    *response = P_VERSION C_TERM_OK;
    Logger::Info("takeover: notifying all listeners");
    for (std::set<TakeoverListener*>::iterator it =
           m_takeover_listeners.begin();
         it != m_takeover_listeners.end(); ++it) {
      (*it)->takeoverShutdown();
    }
    Logger::Info("takeover: notification complete");
    return -1;
  } else {
    Logger::Info("takeover: request is unrecognize");
    *response = P_VERSION C_UNKNOWN;
    return -1;
  }
}

int TakeoverAgent::setupFdServer(event_base *eventBase, int sock,
                                 Callback *callback) {
  int ret;
  m_sock = sock;
  m_callback = callback;
  ret = unlink(m_transfer_fname.c_str());
  if (ret < 0 && errno != ENOENT) {
    Logger::Error("Unalbe to unlink '%s': %s",
                  m_transfer_fname.c_str(), folly::errnoStr(errno).c_str());
    return -1;
  }

  ret = afdt_create_server(
      m_transfer_fname.c_str(),
      eventBase,
      fd_transfer_request_handler,
      afdt_no_post,
      fd_transfer_error_hander,
      &m_delete_handle,
      this);
  // We don't really care if this worked or not.
  // If it didn't, the next invocation of the server
  // will just have to kill us.
  if (ret >= 0) {
    Logger::Info("takeover: fd server set up successfully");
  }
  return ret;
}

int TakeoverAgent::takeover(std::chrono::seconds timeoutSec) {
  int ret;

  Logger::Info("takeover: beginning listen socket acquisition");
  uint8_t fd_request[3] = P_VERSION C_FD_REQ;
  uint8_t fd_response[3] = {0,0,0};
  uint32_t response_len = sizeof(fd_response);
  afdt_error_t err = AFDT_ERROR_T_INIT;
  // TODO(dreiss): Make this timeout configurable.
  struct timeval timeout = { timeoutSec.count() , 0 };
  ret = afdt_sync_client(
      m_transfer_fname.c_str(),
      fd_request,
      sizeof(fd_request) - 1,
      fd_response,
      &response_len,
      &m_sock,
      &timeout,
      &err);
  if (ret < 0) {
    fd_transfer_error_hander(&err, nullptr);
    errno = EADDRINUSE;
    return -1;
  } else if (m_sock < 0) {
    String resp((const char*)fd_response, response_len, CopyString);
    Logger::Error(
        "AFDT did not receive a file descriptor: "
        "response = '%s'",
        f_addcslashes(resp, null_string).data());
    errno = EADDRINUSE;
    return -1;
  }

  Logger::Info("takeover: acquired listen socket");
  m_took_over = true;

  return m_sock;
}

void TakeoverAgent::requestShutdown() {
  if (m_took_over) {
    Logger::Info("takeover: requesting shutdown of satellites");
    // Use AFDT to synchronously shut down the old server's satellites
    // so we can take their ports using accept.  The main server will be
    // stopped asynchronously.
    uint8_t shutdown_request[3] = P_VERSION C_TERM_REQ;
    uint8_t shutdown_response[3] = {0,0,0};
    uint32_t response_len = sizeof(shutdown_response);
    int should_not_receive_fd;
    afdt_error_t err = AFDT_ERROR_T_INIT;
    // TODO(dreiss): Make this timeout configurable.
    // We can aford to wait a long time here, since we've already started
    // the dispatcher for this server.  We want to give the old server
    // plenty of time to shut down all of its satellite servers.
    struct timeval timeout = { 10 , 0 };
    int ret = afdt_sync_client(
        m_transfer_fname.c_str(),
        shutdown_request,
        sizeof(shutdown_request) - 1,
        shutdown_response,
        &response_len,
        &should_not_receive_fd,
        &timeout,
        &err);
    if (ret < 0) {
      fd_transfer_error_hander(&err, nullptr);
      Logger::Warning("Failed to shut-down old server with AFDT.");
      // The higher-level start logic will try *very* hard to recover from this.
    }
    String resp((const char*)shutdown_response, response_len, CopyString);
    if (resp != s_ver_C_TERM_OK) {
      Logger::Error(
          "Old server could not shut down: "
          "response = '%s'",
          f_addcslashes(resp, null_string).data());
    } else {
      Logger::Info("takeover: old satellites have shut down");
    }
  }
}

void TakeoverAgent::stop() {
  if (m_delete_handle != nullptr) {
    afdt_close_server(m_delete_handle);
  }

  // If we're doing takeover, we don't want to gracefully close the
  // socket. If the takeover was fully completed the socket should
  // already be closed. On the other hand if the takeover was started,
  // we don't want to call shutdown because the new server might be
  // listening on that socket and we would cause it to never work. To
  // be safe we close the socket so that if nobody else is listening
  // the OS starts rejecting requests but if somebody is listening we
  // let them receive the requests.
  if (m_takeover_state != TakeoverState::NotStarted) {
    m_callback->takeoverAborted();
  }
}

TakeoverListener::~TakeoverListener() {
}

}
