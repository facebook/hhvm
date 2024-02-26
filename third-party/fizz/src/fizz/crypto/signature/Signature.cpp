/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/openssl/OpenSSLKeyUtils.h>
#include <fizz/crypto/signature/Signature.h>
#include <openssl/crypto.h>

#include <folly/Conv.h>
#include <folly/ScopeGuard.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

using namespace folly;

namespace fizz {
namespace detail {

static const EVP_MD* getHash(int hashNid) {
  const auto hash = EVP_get_digestbynid(hashNid);
  if (!hash) {
    throw std::runtime_error("Invalid hash. Have you initialized openssl?");
  }
  return hash;
}

std::unique_ptr<folly::IOBuf> ecSign(
    folly::ByteRange data,
    const folly::ssl::EvpPkeyUniquePtr& pkey,
    int hashNid) {
  folly::ssl::EvpMdCtxUniquePtr mdCtx(EVP_MD_CTX_new());
  if (!mdCtx) {
    throw std::runtime_error(
        to<std::string>("Could not allocate EVP_MD_CTX", getOpenSSLError()));
  }

  auto hash = getHash(hashNid);

  if (EVP_SignInit(mdCtx.get(), hash) != 1) {
    throw std::runtime_error("Could not initialize signature");
  }
  if (EVP_SignUpdate(mdCtx.get(), data.data(), data.size()) != 1) {
    throw std::runtime_error(
        to<std::string>("Could not sign data ", getOpenSSLError()));
  }
  auto out = folly::IOBuf::create(EVP_PKEY_size(pkey.get()));
  unsigned int bytesWritten = 0;
  if (EVP_SignFinal(
          mdCtx.get(), out->writableData(), &bytesWritten, pkey.get()) != 1) {
    throw std::runtime_error("Failed to sign");
  }
  out->append(bytesWritten);
  return out;
}

void ecVerify(
    folly::ByteRange data,
    folly::ByteRange signature,
    const folly::ssl::EvpPkeyUniquePtr& pkey,
    int hashNid) {
  auto hash = getHash(hashNid);
  folly::ssl::EvpMdCtxUniquePtr mdCtx(EVP_MD_CTX_new());
  if (!mdCtx) {
    throw std::runtime_error(
        to<std::string>("Could not allocate EVP_MD_CTX", getOpenSSLError()));
  }

  if (EVP_VerifyInit(mdCtx.get(), hash) != 1) {
    throw std::runtime_error("Could not initialize verification");
  }

  if (EVP_VerifyUpdate(mdCtx.get(), data.data(), data.size()) != 1) {
    throw std::runtime_error("Could not update verification");
  }

  if (EVP_VerifyFinal(
          mdCtx.get(), signature.data(), signature.size(), pkey.get()) != 1) {
    throw std::runtime_error("Signature verification failed");
  }
}

#if FIZZ_OPENSSL_HAS_ED25519
std::unique_ptr<folly::IOBuf> edSign(
    folly::ByteRange data,
    const folly::ssl::EvpPkeyUniquePtr& pkey) {
  folly::ssl::EvpMdCtxUniquePtr mdCtx(EVP_MD_CTX_new());
  if (!mdCtx) {
    throw std::runtime_error(
        to<std::string>("Could not allocate EVP_MD_CTX", getOpenSSLError()));
  }
  if (EVP_DigestSignInit(mdCtx.get(), nullptr, nullptr, nullptr, pkey.get()) !=
      1) {
    throw std::runtime_error("Could not initialize digest signature");
  }
  auto out = folly::IOBuf::create(EVP_PKEY_size(pkey.get()));
  size_t bytesWritten = out->capacity();

  // Sign & verify APIs for EdDSA exist in OpenSSL only as one-shot digest APIs
  // because they are implemented using PureEdDSA, which only provides one-shot
  // digest APIs. See https://www.openssl.org/docs/manmaster/man7/Ed25519.html
  // for more details on this constraint.
  if (EVP_DigestSign(
          mdCtx.get(),
          out->writableData(),
          &bytesWritten,
          data.data(),
          data.size()) != 1) {
    throw std::runtime_error("Failed to sign");
  }
  out->append(bytesWritten);
  return out;
}

void edVerify(
    folly::ByteRange data,
    folly::ByteRange signature,
    const folly::ssl::EvpPkeyUniquePtr& pkey) {
  folly::ssl::EvpMdCtxUniquePtr mdCtx(EVP_MD_CTX_new());
  if (!mdCtx) {
    throw std::runtime_error(
        to<std::string>("Could not allocate EVP_MD_CTX", getOpenSSLError()));
  }
  if (EVP_DigestVerifyInit(
          mdCtx.get(), nullptr, nullptr, nullptr, pkey.get()) != 1) {
    throw std::runtime_error("Could not initialize digest signature");
  }

  // Sign & verify APIs for EdDSA exist in OpenSSL only as one-shot digest APIs
  // because they are implemented using PureEdDSA, which only provides one-shot
  // digest APIs. See https://www.openssl.org/docs/manmaster/man7/Ed25519.html
  // for more details on this constraint.
  if (EVP_DigestVerify(
          mdCtx.get(),
          signature.data(),
          signature.size(),
          data.data(),
          data.size()) != 1) {
    throw std::runtime_error("Signature verification failed");
  }
}
#endif

std::unique_ptr<folly::IOBuf> rsaPssSign(
    folly::ByteRange data,
    const folly::ssl::EvpPkeyUniquePtr& pkey,
    int hashNid) {
  auto hash = getHash(hashNid);
  folly::ssl::EvpMdCtxUniquePtr mdCtx(EVP_MD_CTX_new());
  if (!mdCtx) {
    throw std::runtime_error(
        to<std::string>("Could not allocate EVP_MD_CTX", getOpenSSLError()));
  }

  EVP_PKEY_CTX* ctx;
  if (EVP_DigestSignInit(mdCtx.get(), &ctx, hash, nullptr, pkey.get()) != 1) {
    throw std::runtime_error("Could not initialize signature");
  }

  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PSS_PADDING) <= 0) {
    throw std::runtime_error("Could not set pss padding");
  }

  if (EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx, -1) <= 0) {
    throw std::runtime_error("Could not set pss salt length");
  }

  if (EVP_DigestSignUpdate(mdCtx.get(), data.data(), data.size()) != 1) {
    throw std::runtime_error("Could not update signature");
  }

  size_t bytesWritten = EVP_PKEY_size(pkey.get());
  auto out = folly::IOBuf::create(bytesWritten);
  if (EVP_DigestSignFinal(mdCtx.get(), out->writableData(), &bytesWritten) !=
      1) {
    throw std::runtime_error("Failed to sign");
  }
  out->append(bytesWritten);
  return out;
}

void rsaPssVerify(
    folly::ByteRange data,
    folly::ByteRange signature,
    const folly::ssl::EvpPkeyUniquePtr& pkey,
    int hashNid) {
  auto hash = getHash(hashNid);
  folly::ssl::EvpMdCtxUniquePtr mdCtx(EVP_MD_CTX_new());
  if (!mdCtx) {
    throw std::runtime_error(
        to<std::string>("Could not allocate EVP_MD_CTX", getOpenSSLError()));
  }

  EVP_PKEY_CTX* ctx;
  if (EVP_DigestVerifyInit(mdCtx.get(), &ctx, hash, nullptr, pkey.get()) != 1) {
    throw std::runtime_error("Could not initialize verification");
  }

  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PSS_PADDING) <= 0) {
    throw std::runtime_error("Could not set pss padding");
  }

  if (EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx, -1) <= 0) {
    throw std::runtime_error("Could not set pss salt length");
  }

  if (EVP_DigestVerifyUpdate(mdCtx.get(), data.data(), data.size()) != 1) {
    throw std::runtime_error("Could not update verification");
  }

  if (EVP_DigestVerifyFinal(
          mdCtx.get(),
          // const_cast<unsigned char*> is needed for OpenSSL 1.0.1 on Debian 8,
          // which HHVM currently expects to support until 2020/6/30
          const_cast<unsigned char*>(signature.data()),
          signature.size()) != 1) {
    throw std::runtime_error("Signature verification failed");
  }
}
} // namespace detail
} // namespace fizz
