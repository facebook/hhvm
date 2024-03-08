/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/ClientExtensions.h>
#include <fizz/client/PskCache.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/protocol/Events.h>
#include <fizz/protocol/ech/Types.h>
#include <fizz/record/Types.h>
#include <fizz/util/Variant.h>
#include <folly/Executor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/WriteFlags.h>

namespace fizz {

class CertificateVerifier;
class ServerExtensions;

namespace server {
class FizzServerContext;
}

namespace client {
class FizzClientContext;
}

struct Accept : EventType<Event::Accept> {
  folly::Executor* executor;
  std::shared_ptr<const server::FizzServerContext> context;
  std::shared_ptr<ServerExtensions> extensions;
};

struct Connect : EventType<Event::Connect> {
  std::shared_ptr<const client::FizzClientContext> context;
  std::shared_ptr<const CertificateVerifier> verifier;
  folly::Optional<std::string> sni;
  folly::Optional<client::CachedPsk> cachedPsk;
  std::shared_ptr<ClientExtensions> extensions;
  folly::Optional<std::vector<ech::ECHConfig>> echConfigs;
};

struct EarlyAppWrite : EventType<Event::EarlyAppWrite> {
  void* token{nullptr};
  std::unique_ptr<folly::IOBuf> data;
  folly::WriteFlags flags;
  Aead::AeadOptions aeadOptions;
};

struct AppWrite : EventType<Event::AppWrite> {
  void* token{nullptr};
  std::unique_ptr<folly::IOBuf> data;
  folly::WriteFlags flags;
  Aead::AeadOptions aeadOptions;
};

struct AppData : EventType<Event::AppData> {
  std::unique_ptr<folly::IOBuf> data;

  explicit AppData(std::unique_ptr<folly::IOBuf> buf) : data(std::move(buf)) {}
};

struct KeyUpdateInitiation : EventType<Event::KeyUpdateInitiation> {
  KeyUpdateRequest request_update;
};

struct WriteNewSessionTicket : EventType<Event::WriteNewSessionTicket> {
  Buf appToken;
};

/**
 * Parameters for each event that will be processed by the state machine.
 */
#define FIZZ_PARAM(F, ...)              \
  F(ClientHello, __VA_ARGS__)           \
  F(ServerHello, __VA_ARGS__)           \
  F(EndOfEarlyData, __VA_ARGS__)        \
  F(HelloRetryRequest, __VA_ARGS__)     \
  F(EncryptedExtensions, __VA_ARGS__)   \
  F(CertificateRequest, __VA_ARGS__)    \
  F(CompressedCertificate, __VA_ARGS__) \
  F(CertificateMsg, __VA_ARGS__)        \
  F(CertificateVerify, __VA_ARGS__)     \
  F(Finished, __VA_ARGS__)              \
  F(NewSessionTicket, __VA_ARGS__)      \
  F(KeyUpdateInitiation, __VA_ARGS__)   \
  F(KeyUpdate, __VA_ARGS__)             \
  F(Alert, __VA_ARGS__)                 \
  F(CloseNotify, __VA_ARGS__)           \
  F(Accept, __VA_ARGS__)                \
  F(Connect, __VA_ARGS__)               \
  F(AppData, __VA_ARGS__)               \
  F(AppWrite, __VA_ARGS__)              \
  F(EarlyAppWrite, __VA_ARGS__)         \
  F(WriteNewSessionTicket, __VA_ARGS__)

FIZZ_DECLARE_VARIANT_TYPE(Param, FIZZ_PARAM)

// Given a Param variant, return the corresponding Event
class EventVisitor {
 public:
  Event operator()(const Param& param) const;
};

// App closes bypass the state machine so aren't in the Param variant.
struct AppClose {
  enum ClosePolicy { IMMEDIATE, WAIT };

  /*implicit */ constexpr AppClose(ClosePolicy pol) : policy(pol) {}

  ClosePolicy policy;
};

} // namespace fizz
