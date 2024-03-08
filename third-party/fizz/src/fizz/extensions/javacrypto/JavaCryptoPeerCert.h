/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/signature/Signature.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/record/Types.h>
#include <jni.h>

namespace fizz {

class JavaCryptoPeerCert : public PeerCert {
 public:
  static void onLoad(JNIEnv* env);

  explicit JavaCryptoPeerCert(Buf certData);

  ~JavaCryptoPeerCert() override = default;

  // Returns the full Distinguished Name of the certificate
  std::string getIdentity() const override;

  void verify(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned,
      folly::ByteRange signature) const override;

  folly::ssl::X509UniquePtr getX509() const override;

 private:
  jobject jobject_;
};

} // namespace fizz
