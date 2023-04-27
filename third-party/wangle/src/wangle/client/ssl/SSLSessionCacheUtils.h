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

#pragma once

#include <folly/FBString.h>
#include <folly/Optional.h>
#include <wangle/client/ssl/SSLSessionCacheData.h>

#include <openssl/ssl.h>

namespace wangle {

// Service identity access on the session.
folly::Optional<std::string> getSessionServiceIdentity(SSL_SESSION* sess);
bool setSessionServiceIdentity(SSL_SESSION* sess, const std::string& str);

// Helpers to convert SSLSessionCacheData to/from SSL_SESSION
folly::Optional<SSLSessionCacheData> getCacheDataForSession(SSL_SESSION* sess);
SSL_SESSION* getSessionFromCacheData(const SSLSessionCacheData& data);

// Does a clone of just the session data and service identity
// Internal links to SSL structs are not kept
SSL_SESSION* cloneSSLSession(SSL_SESSION* toClone);

folly::Optional<std::string> getSessionPeerIdentities(SSL_SESSION* sess);
bool setSessionPeerIdentities(SSL_SESSION* sess, const std::string& str);

} // namespace wangle
