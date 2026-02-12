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

#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

#include <folly/String.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/ucred.h> // @manual
#endif

namespace {

#ifndef _WIN32
uid_t errnoToUid(int no) {
  static_assert(
      sizeof(int) <= sizeof(uid_t), "We want to stash errno in a uid_t");
  if (no < 0) {
    // Make up an invalid errno value - negative shouldn't happen.
    no = std::numeric_limits<int>::max();
  }
  return static_cast<uid_t>(no);
}
#endif

} // namespace

namespace apache::thrift {

Cpp2ConnContext::PeerCred Cpp2ConnContext::PeerCred::queryFromSocket(
    folly::NetworkSocket socket) {
#if defined(SO_PEERCRED) // Linux
  struct ucred cred = {};
  socklen_t len = sizeof(cred);
  if (getsockopt(socket.toFd(), SOL_SOCKET, SO_PEERCRED, &cred, &len)) {
    return PeerCred{ErrorRetrieving, errnoToUid(errno), 0};
  } else {
    return PeerCred{cred.pid, cred.uid, cred.gid};
  }
#elif defined(LOCAL_PEERCRED) // macOS and FreeBSD
  struct xucred cred = {};
  socklen_t len;
  if (getsockopt(
          socket.toFd(),
          SOL_LOCAL,
          LOCAL_PEERCRED,
          &cred,
          &(len = sizeof(cred)))) {
    return PeerCred{ErrorRetrieving, errnoToUid(errno), 0};
  } else {
#ifdef __APPLE__
    pid_t epid = 0;
    if (getsockopt(
            socket.toFd(),
            SOL_LOCAL,
            LOCAL_PEEREPID,
            &epid,
            &(len = sizeof(epid)))) {
      return PeerCred{ErrorRetrieving, errnoToUid(errno), 0};
    } else {
      return PeerCred{epid, cred.cr_uid, cred.cr_gid};
    }
#else
    return PeerCred{cred.cr_pid, cred.cr_uid, cred.cr_gid};
#endif
  }
#else
  (void)socket;
  return PeerCred{UnsupportedPlatform};
#endif
}

folly::Optional<std::string> Cpp2ConnContext::PeerCred::getError() const {
  if (UnsupportedPlatform == pid_) {
    return folly::make_optional<std::string>("unsupported platform");
#ifndef _WIN32
  } else if (ErrorRetrieving == pid_) {
    return folly::to<std::string>("getsockopt failed: ", folly::errnoStr(uid_));
#endif
  } else {
    return folly::none;
  }
}

std::optional<std::string_view> ClientMetadataRef::getAgent() const {
  if (!md_.agent()) {
    return {};
  }
  return std::string_view{*md_.agent()};
}

std::optional<std::string_view> ClientMetadataRef::getHostname() const {
  if (!md_.hostname()) {
    return {};
  }
  return std::string_view{*md_.hostname()};
}

std::optional<std::string_view> ClientMetadataRef::getOtherMetadataField(
    std::string_view key) {
  if (const auto& otherMetadata = md_.otherMetadata()) {
    if (auto* value = folly::get_ptr(*otherMetadata, std::string{key})) {
      return std::string_view{*value};
    }
    return {};
  }
  return {};
}

const folly::F14NodeMap<std::string, std::string>&
ClientMetadataRef::getFields() const {
  static const folly::F14NodeMap<std::string, std::string> emptyFields;

  return static_cast<bool>(md_.otherMetadata()) ? *md_.otherMetadata()
                                                : emptyFields;
}

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    ConnectionInternalFieldsT, createPerConnectionInternalFields) {
  return ConnectionInternalFieldsT();
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    RequestInternalFieldsT, createPerRequestInternalFields) {
  return RequestInternalFieldsT();
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::vector<EkmInfo>, populateCachedEkms, const folly::AsyncTransport&) {
  return std::vector<EkmInfo>();
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::optional<SecurityPolicy>,
    getConnectionSecurityPolicy,
    const Cpp2ConnContext& /* connContext */) {
  return std::nullopt;
}

} // namespace detail

} // namespace apache::thrift
