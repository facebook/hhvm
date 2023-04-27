/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace proxygen {

// enum class ConnectionCloseReason : unsigned int {
//  SHUTDOWN,               // shutdown...probably due to the short shutdown
//                          //  time we won't be able to see any of this
//  READ_EOF,               // received FIN from client so the connection is
//                          //  not reusable any more
//  GOAWAY,                 // session closed due to ingress goaway
//  SESSION_PARSE_ERROR,    // http/spdy parse error
//  REMOTE_ERROR,           // The various 5xx error
//  TRANSACTION_ABORT,      // transaction sendAbort()
//  TIMEOUT,                // read/write timeout excluding shutdown
//  IO_READ_ERROR,          // read error
//  IO_WRITE_ERROR,         // write error
//  REQ_NOTREUSABLE,        // client request is not reusable (http 1.0 or
//                          //  Connection: close)
//  ERR_RESP,               // various 4xx error
//  UNKNOWN,                // this probably indicate some bug that the close
//                          //  reason is not accounted for
//  kMAX_REASON
//};
// clang-format off
#define CONNECTION_CLOSE_REASON_GEN(x) \
  x(SHUTDOWN, "shutdown") \
  x(READ_EOF, "read_eof") \
  x(GOAWAY, "goaway") \
  x(SESSION_PARSE_ERROR, "session_parse_err") \
  x(REMOTE_ERROR, "remote_err") \
  x(TRANSACTION_ABORT, "transaction_abort") \
  x(TIMEOUT, "timeout") \
  x(IO_READ_ERROR, "io_read_err") \
  x(IO_WRITE_ERROR, "io_write_err") \
  x(REQ_NOTREUSABLE, "req_not_reusable") \
  x(ERR_RESP, "err_resp") \
  x(UNKNOWN, "unknown") \
  x(FLOW_CONTROL, "flow_control") \
  x(kMAX_REASON, "unset")
// clang-format on

#define CONNECTION_CLOSE_REASON_ENUM(e, r) e,
enum class ConnectionCloseReason {
  CONNECTION_CLOSE_REASON_GEN(CONNECTION_CLOSE_REASON_ENUM)
};
#undef CONNECTION_CLOSE_REASON_ENUM

extern const char* getConnectionCloseReasonStringByIndex(unsigned int i);
extern const char* getConnectionCloseReasonString(ConnectionCloseReason r);

/**
 * Protocol to which the HTTPTransaction was upgraded
 */
enum class UpgradeProtocol : int {
  // We only support changing to TCP after CONNECT requests
  TCP
};

} // namespace proxygen
