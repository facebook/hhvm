/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
#include <wangle/client/ssl/SSLSessionCallbacks.h>

namespace wangle {
// static
void SSLSessionCallbacks::attachCallbacksToContext(
    folly::SSLContext* context,
    SSLSessionCallbacks* callbacks) {
  auto ctx = context->getSSLCtx();
  SSL_CTX_set_session_cache_mode(
      ctx,
      SSL_SESS_CACHE_NO_INTERNAL | SSL_SESS_CACHE_CLIENT |
          SSL_SESS_CACHE_NO_AUTO_CLEAR);
  // Only initializes the cache index the first time.
  SSLUtil::getSSLCtxExIndex(&getCacheIndex());
  SSL_CTX_set_ex_data(ctx, getCacheIndex(), callbacks);
  context->setSessionLifecycleCallbacks(
      std::make_unique<ContextSessionCallbacks>());
}

// static
void SSLSessionCallbacks::detachCallbacksFromContext(
    folly::SSLContext* context,
    SSLSessionCallbacks* callbacks) {
  auto ctx = context->getSSLCtx();
  auto sslSessionCache = getCacheFromContext(ctx);
  if (sslSessionCache != callbacks) {
    return;
  }
  // We don't unset flags here because we cannot assume that we are the only
  // code that sets the cache flags.
  SSL_CTX_set_ex_data(ctx, getCacheIndex(), nullptr);
  SSL_CTX_sess_set_remove_cb(ctx, nullptr);
  context->setSessionLifecycleCallbacks(nullptr);
}

// static
SSLSessionCallbacks* SSLSessionCallbacks::getCacheFromContext(SSL_CTX* ctx) {
  return static_cast<SSLSessionCallbacks*>(
      SSL_CTX_get_ex_data(ctx, getCacheIndex()));
}

// static
std::string SSLSessionCallbacks::getSessionKeyFromSSL(SSL* ssl) {
  auto sock = folly::AsyncSSLSocket::getFromSSL(ssl);
  return sock ? sock->getSessionKey() : "";
}

void SSLSessionCallbacks::ContextSessionCallbacks::onNewSession(
    SSL* ssl,
    folly::ssl::SSLSessionUniquePtr session) {
  SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);
  auto sslSessionCache = SSLSessionCallbacks::getCacheFromContext(ctx);

  // To guarantee that sessionKey that we use as the key in our cache matches
  // the session key stored in SSL_SESSION, we explicitly invoke any user logic
  // first, ensuring that we always have control over these fields.
  sslSessionCache->onNewSession(ssl, session.get());

  std::string sessionKey = SSLSessionCallbacks::getSessionKeyFromSSL(ssl);
  if (sessionKey.empty()) {
    const char* name = folly::AsyncSSLSocket::getSSLServerNameFromSSL(ssl);
    sessionKey = name ? name : "";
  }
  if (!sessionKey.empty()) {
    setSessionServiceIdentity(session.get(), sessionKey);
    sslSessionCache->setSSLSession(sessionKey, std::move(session));
  }
}
} // namespace wangle
