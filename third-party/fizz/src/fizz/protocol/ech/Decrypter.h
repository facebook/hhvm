/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/ech/Encryption.h>
#include <fizz/protocol/ech/Types.h>

namespace fizz {
namespace ech {

struct DecrypterParams {
  ECHConfig echConfig;
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
  virtual folly::Optional<DecrypterResult> decryptClientHello(
      const ClientHello& chlo) = 0;
  /**
   * Similar to above, but handles the HRR case. Config ID is always required,
   * and stateful HRR will pass in the existing context while stateless HRR
   * will pass in the encapsulated key to recreate the context.
   */
  virtual ClientHello decryptClientHelloHRR(
      const ClientHello& chlo,
      std::unique_ptr<hpke::HpkeContext>& context) = 0;
  virtual ClientHello decryptClientHelloHRR(
      const ClientHello& chlo,
      const std::unique_ptr<folly::IOBuf>& encapsulatedKey) = 0;
  virtual std::vector<ech::ECHConfig> getRetryConfigs() const = 0;
};

class ECHConfigManager : public Decrypter {
 public:
  void addDecryptionConfig(DecrypterParams decrypterParams);
  folly::Optional<DecrypterResult> decryptClientHello(
      const ClientHello& chlo) override;
  ClientHello decryptClientHelloHRR(
      const ClientHello& chlo,
      std::unique_ptr<hpke::HpkeContext>& context) override;
  ClientHello decryptClientHelloHRR(
      const ClientHello& chlo,
      const std::unique_ptr<folly::IOBuf>& encapsulatedKey) override;
  std::vector<ech::ECHConfig> getRetryConfigs() const override;

 private:
  std::vector<DecrypterParams> configs_;
};

} // namespace ech
} // namespace fizz
