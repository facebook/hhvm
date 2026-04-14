/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Factory.h>
#include <fizz/protocol/ech/Encryption.h>
#include <fizz/protocol/ech/Types.h>

namespace fizz {
namespace ech {

struct DecrypterParams {
  ParsedECHConfig echConfig;
  std::unique_ptr<KeyExchange> kex;
};

struct DecrypterResult {
  ClientHello chlo;
  uint8_t configId;
  std::unique_ptr<hpke::HpkeContext> context;
};

class Decrypter {
 public:
  virtual ~Decrypter() = default;
  virtual Status decryptClientHello(
      folly::Optional<DecrypterResult>& ret,
      Error& err,
      const ClientHello& chlo) = 0;
  /**
   * Similar to above, but handles the HRR case. Config ID is always required,
   * and stateful HRR will pass in the existing context while stateless HRR
   * will pass in the encapsulated key to recreate the context.
   */
  virtual Status decryptClientHelloHRR(
      ClientHello& ret,
      Error& err,
      const ClientHello& chlo,
      std::unique_ptr<hpke::HpkeContext>& context) = 0;
  virtual Status decryptClientHelloHRR(
      ClientHello& ret,
      Error& err,
      const ClientHello& chlo,
      const std::unique_ptr<folly::IOBuf>& encapsulatedKey) = 0;
  virtual Status getRetryConfigs(
      std::vector<ech::ECHConfig>& ret,
      Error& err,
      const folly::Optional<std::string>& maybeSni) const = 0;
};

class ECHConfigManager : public Decrypter {
 public:
  explicit ECHConfigManager(std::shared_ptr<Factory> factory)
      : factory_(std::move(factory)) {}
  void addDecryptionConfig(DecrypterParams decrypterParams);
  Status decryptClientHello(
      folly::Optional<DecrypterResult>& ret,
      Error& err,
      const ClientHello& chlo) override;
  Status decryptClientHelloHRR(
      ClientHello& ret,
      Error& err,
      const ClientHello& chlo,
      std::unique_ptr<hpke::HpkeContext>& context) override;
  Status decryptClientHelloHRR(
      ClientHello& ret,
      Error& err,
      const ClientHello& chlo,
      const std::unique_ptr<folly::IOBuf>& encapsulatedKey) override;
  Status getRetryConfigs(
      std::vector<ech::ECHConfig>& ret,
      Error& err,
      const folly::Optional<std::string>& maybeSni) const override;

 protected:
  std::shared_ptr<Factory> factory_;
  std::vector<DecrypterParams> configs_;
};

} // namespace ech
} // namespace fizz
