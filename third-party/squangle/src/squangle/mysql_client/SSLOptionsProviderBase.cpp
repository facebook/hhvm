/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/SSLOptionsProviderBase.h"
#include <folly/io/async/SSLContext.h>

namespace facebook::common::mysql_client {

bool SSLOptionsProviderBase::setMysqlSSLOptions(MYSQL* mysql) {
  auto sslContext = getSSLContext();
  if (!sslContext) {
    return false;
  }
  // We need to set ssl_mode because we set it to disabled after we call
  // mysql_init.
  enum mysql_ssl_mode ssl_mode = SSL_MODE_PREFERRED;
  mysql_options(mysql, MYSQL_OPT_SSL_MODE, &ssl_mode);
  mysql_options(mysql, MYSQL_OPT_SSL_CONTEXT, sslContext->getSSLCtx());
  auto sslSession = getRawSSLSession();
  if (sslSession) {
    mysql_options4(
        mysql, MYSQL_OPT_SSL_SESSION, sslSession.release(), (void*)1);
  }
  return true;
}

bool SSLOptionsProviderBase::storeMysqlSSLSession(MYSQL* mysql) {
  auto reused = mysql_get_ssl_session_reused(mysql);
  if (!reused) {
    folly::ssl::SSLSessionUniquePtr session(
        (SSL_SESSION*)mysql_get_ssl_session(mysql));
    if (session) {
      storeRawSSLSession(std::move(session));
    }
  }
  return reused;
}

} // namespace facebook::common::mysql_client
