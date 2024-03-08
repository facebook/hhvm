/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <folly/FileUtil.h>
#include <folly/ssl/OpenSSLCertUtils.h>

namespace fizz {

struct STACK_OF_X509_deleter {
  void operator()(STACK_OF(X509) * sk) {
    sk_X509_free(sk);
  }
};

/* static */ std::unique_ptr<DefaultCertificateVerifier>
DefaultCertificateVerifier::createFromCAFile(
    VerificationContext context,
    const std::string& caFile) {
  auto store = folly::ssl::OpenSSLCertUtils::readStoreFromFile(caFile);
  return std::make_unique<DefaultCertificateVerifier>(
      context, std::move(store));
}

/* static */ std::unique_ptr<DefaultCertificateVerifier>
DefaultCertificateVerifier::createFromCAFiles(
    VerificationContext context,
    const std::vector<std::string>& caFiles) {
  std::string certBuffer;
  for (const auto& caFile : caFiles) {
    std::string readBuffer;
    if (!folly::readFile(caFile.c_str(), readBuffer)) {
      throw std::runtime_error(
          folly::to<std::string>("Could not read store file: ", caFile));
    }
    folly::toAppend(readBuffer, &certBuffer);
  }
  return std::make_unique<DefaultCertificateVerifier>(
      context,
      folly::ssl::OpenSSLCertUtils::readStoreFromBuffer(
          folly::StringPiece(certBuffer)));
}

std::shared_ptr<const Cert> DefaultCertificateVerifier::verify(
    const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs) const {
  std::ignore = verifyWithX509StoreCtx(certs);
  // Just return the original cert in the default case
  return certs.front();
}

folly::ssl::X509StoreCtxUniquePtr
DefaultCertificateVerifier::verifyWithX509StoreCtx(
    const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs) const {
  if (certs.empty()) {
    throw std::runtime_error("no certificates to verify");
  }

  auto leafCert = certs.front()->getX509();

  auto certChainStack = std::unique_ptr<STACK_OF(X509), STACK_OF_X509_deleter>(
      sk_X509_new_null());
  if (!certChainStack) {
    throw std::bad_alloc();
  }

  for (size_t i = 1; i < certs.size(); i++) {
    sk_X509_push(certChainStack.get(), certs[i]->getX509().get());
  }

  auto ctx = folly::ssl::X509StoreCtxUniquePtr(X509_STORE_CTX_new());
  if (!ctx) {
    throw std::bad_alloc();
  }

  if (X509_STORE_CTX_init(
          ctx.get(),
          x509Store_ ? x509Store_.get() : getDefaultX509Store(),
          leafCert.get(),
          certChainStack.get()) != 1) {
    throw std::runtime_error("failed to initialize store context");
  }

  if (X509_STORE_CTX_set_default(
          ctx.get(),
          context_ == VerificationContext::Server ? "ssl_client"
                                                  : "ssl_server") != 1) {
    throw std::runtime_error("failed to set default verification method");
  }

  if (customVerifyCallback_) {
    X509_STORE_CTX_set_verify_cb(ctx.get(), customVerifyCallback_);
  }

  folly::ssl::X509VerifyParam param(X509_VERIFY_PARAM_new());
  if (!param) {
    throw std::bad_alloc();
  }

  if (X509_VERIFY_PARAM_set_flags(param.get(), X509_V_FLAG_X509_STRICT) != 1) {
    throw std::runtime_error("failed to set strict certificate checking");
  }

  if (X509_VERIFY_PARAM_set1(
          X509_STORE_CTX_get0_param(ctx.get()), param.get()) != 1) {
    throw std::runtime_error("failed to apply verification parameters");
  }

  int ret = 0;
  // if openssl is not built with TSAN then we can get a TSAN false positive
  // when calling X509_verify_cert from multiple threads
  {
    folly::annotate_ignore_thread_sanitizer_guard g(__FILE__, __LINE__);
    ret = X509_verify_cert(ctx.get());
  }

  if (ret != 1) {
    const auto errorInt = X509_STORE_CTX_get_error(ctx.get());
    std::string errorText =
        std::string(X509_verify_cert_error_string(errorInt));
    throw std::runtime_error("certificate verification failed: " + errorText);
  }

  return ctx;
}

void DefaultCertificateVerifier::createAuthorities() {
  CertificateAuthorities auth;
  X509_STORE* store = x509Store_ ? x509Store_.get() : getDefaultX509Store();
  // X509_STORE stores CA certs as objects in this stack.
  STACK_OF(X509_OBJECT)* entries = X509_STORE_get0_objects(store);

  for (int i = 0; i < sk_X509_OBJECT_num(entries); i++) {
    X509_OBJECT* obj = sk_X509_OBJECT_value(entries, i);
    if (X509_OBJECT_get_type(obj) == X509_LU_X509) {
      auto certIssuer = X509_get_subject_name(X509_OBJECT_get0_X509(obj));
      int dnLength = i2d_X509_NAME(certIssuer, nullptr);
      if (dnLength < 0) {
        throw std::runtime_error("Error computing DN length");
      }
      DistinguishedName dn;
      dn.encoded_name = folly::IOBuf::create(dnLength);
      auto dnData = dn.encoded_name->writableData();
      dnLength = i2d_X509_NAME(certIssuer, &dnData);
      if (dnLength < 0) {
        throw std::runtime_error("Error encoding DN in DER format");
      }
      dn.encoded_name->append(dnLength);
      auth.authorities.push_back(std::move(dn));
    }
  }
  authorities_ = std::move(auth);
}

X509_STORE* DefaultCertificateVerifier::getDefaultX509Store() {
  static folly::ssl::X509StoreUniquePtr defaultStore([]() {
    X509_STORE* store = X509_STORE_new();

    if (!store) {
      throw std::bad_alloc();
    }

    if (X509_STORE_set_default_paths(store) != 1) {
      throw std::runtime_error("failed to set default paths");
    }

    return store;
  }());

  return defaultStore.get();
}

std::vector<Extension>
DefaultCertificateVerifier::getCertificateRequestExtensions() const {
  std::vector<Extension> exts;
  exts.push_back(encodeExtension(authorities_));
  return exts;
}

} // namespace fizz
