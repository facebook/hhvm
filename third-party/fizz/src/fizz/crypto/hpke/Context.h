/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/hpke/Hkdf.h>

#include <fizz/crypto/hpke/Types.h>

#include <fizz/protocol/Types.h>

namespace fizz {
namespace hpke {

class HpkeContext {
 public:
  enum class Role { Sender, Receiver };
  virtual ~HpkeContext() = default;
  virtual Status seal(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      const folly::IOBuf* aad,
      std::unique_ptr<folly::IOBuf> pt) = 0;
  virtual Status open(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      const folly::IOBuf* aad,
      std::unique_ptr<folly::IOBuf> ct) = 0;
  virtual Status exportSecret(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf> exporterContext,
      size_t desiredLength) const = 0;

  // Deprecated: Use the Status-returning overloads instead. Will be removed.
  virtual std::unique_ptr<folly::IOBuf> seal(
      const folly::IOBuf* aad,
      std::unique_ptr<folly::IOBuf> pt);
  virtual std::unique_ptr<folly::IOBuf> exportSecret(
      std::unique_ptr<folly::IOBuf> exporterContext,
      size_t desiredLength) const;
  // NOTE: This should only be used for testing.
  virtual std::unique_ptr<folly::IOBuf> getExporterSecret() = 0;
};

class HpkeContextImpl : public HpkeContext {
 public:
  HpkeContextImpl(
      std::unique_ptr<Aead> cipher,
      std::unique_ptr<folly::IOBuf> exporterSecret,
      std::unique_ptr<fizz::hpke::Hkdf> hkdf,
      HpkeSuiteId suiteId,
      uint64_t seqNum,
      Role role);
  HpkeContextImpl(
      std::unique_ptr<Aead> cipher,
      std::unique_ptr<folly::IOBuf> exporterSecret,
      std::unique_ptr<fizz::hpke::Hkdf> hkdf,
      HpkeSuiteId suiteId,
      Role role);
  Status seal(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      const folly::IOBuf* aad,
      std::unique_ptr<folly::IOBuf> pt) override;
  Status open(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      const folly::IOBuf* aad,
      std::unique_ptr<folly::IOBuf> ct) override;
  Status exportSecret(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf> exporterContext,
      size_t desiredLength) const override;
  std::unique_ptr<folly::IOBuf> getExporterSecret() override;

 private:
  Status incrementSeq(Error& err);
  uint64_t seqNum_{0};
  std::unique_ptr<Aead> cipher_;
  std::unique_ptr<folly::IOBuf> exporterSecret_;
  std::unique_ptr<fizz::hpke::Hkdf> hkdf_;
  HpkeSuiteId suiteId_;
  Role role_;
};

} // namespace hpke
} // namespace fizz
