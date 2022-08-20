/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/ssl/OpenSSLPtrTypes.h>
#include <folly/ssl/SSLSession.h>
#include <mysql.h>

namespace folly {
class SSLContext;
}

namespace facebook {
namespace common {
namespace mysql_client {

/* Interface for an SSL connection options Provider */
class SSLOptionsProviderBase {
 public:
  virtual ~SSLOptionsProviderBase() {}

  // The SSL Context and Session options to be set for the connection
  virtual std::shared_ptr<folly::SSLContext> getSSLContext() = 0;

  // These sessions are raw OpenSSL sessions, currently used for resumption
  // in MySQL client
  virtual folly::ssl::SSLSessionUniquePtr getRawSSLSession() = 0;
  virtual void storeRawSSLSession(
      folly::ssl::SSLSessionUniquePtr ssl_session) = 0;

  // These sessions are abstracted ssl sessions, currently used for
  // resumption with folly::AsyncSSLSocket
  virtual std::shared_ptr<folly::ssl::SSLSession> getSSLSession() = 0;
  virtual void storeSSLSession(
      std::shared_ptr<folly::ssl::SSLSession> ssl_session) = 0;
  virtual void allowSessionResumption(bool allow) = 0;

  // Set the SSL Options on the MYSQL object.
  // Returns true if set was successful.
  bool setMysqlSSLOptions(MYSQL* mysql);

  // Fetches the SSL Session from the MYSQL object and stores it.
  // Returns if the SSL Session was reused for this connection.
  bool storeMysqlSSLSession(MYSQL* mysql);
};
} // namespace mysql_client
} // namespace common
} // namespace facebook
