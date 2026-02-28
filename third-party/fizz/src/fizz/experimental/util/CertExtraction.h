/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/server/AsyncFizzServer.h>

namespace fizz {
namespace detail {
template <class SM>
std::pair<std::shared_ptr<const Cert>, std::shared_ptr<const Cert>>
getSelfPeerCertificateShared(const fizz::client::AsyncFizzClientT<SM>& client) {
  return std::make_pair(
      client.getState().clientCert(), client.getState().serverCert());
}
template <class SM>
std::pair<std::shared_ptr<const Cert>, std::shared_ptr<const Cert>>
getSelfPeerCertificateShared(const fizz::server::AsyncFizzServerT<SM>& server) {
  return std::make_pair(
      server.getState().serverCert(), server.getState().clientCert());
}
} // namespace detail
} // namespace fizz
