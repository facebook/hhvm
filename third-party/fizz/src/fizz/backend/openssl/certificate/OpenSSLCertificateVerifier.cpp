/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/certificate/OpenSSLCertificateVerifier.h>
#include <folly/FileUtil.h>
#include <folly/ssl/OpenSSLCertUtils.h>
#include <folly/synchronization/CallOnce.h>

namespace fizz {
namespace openssl {

struct STACK_OF_X509_deleter {
  void operator()(STACK_OF(X509) * sk) {
    sk_X509_free(sk);
  }
};

static AlertDescription toTLSAlert(int opensslVerifyErr) {
  switch (opensslVerifyErr) {
    case X509_V_ERR_CERT_REVOKED:
      return AlertDescription::certificate_revoked;
    case X509_V_ERR_CERT_NOT_YET_VALID:
    case X509_V_ERR_CERT_HAS_EXPIRED:
      /**
       * "A certificate has expired or is not currently valid."
       */
      return AlertDescription::certificate_expired;
    case X509_V_ERR_CERT_CHAIN_TOO_LONG:
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
    case X509_V_ERR_PATH_LENGTH_EXCEEDED:
    case X509_V_ERR_INVALID_CA:
    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
      /**
       * All of these conditions indicate that building a path to a trusted
       * anchor failed.
       */
      return AlertDescription::unknown_ca;
    default:
      return AlertDescription::bad_certificate;
  }
}

/* static */ Status OpenSSLCertificateVerifier::create(
    std::unique_ptr<OpenSSLCertificateVerifier>& ret,
    Error& err,
    VerificationContext context,
    folly::ssl::X509StoreUniquePtr&& store) {
  X509_STORE* storePtr = nullptr;
  if (store) {
    storePtr = store.get();
  } else {
    FIZZ_RETURN_ON_ERROR(getDefaultX509Store(storePtr, err));
  }
  CertificateAuthorities authorities;
  FIZZ_RETURN_ON_ERROR(createAuthorities(authorities, err, storePtr));
  ret = std::unique_ptr<OpenSSLCertificateVerifier>(
      new OpenSSLCertificateVerifier(
          context, std::move(store), std::move(authorities)));
  return Status::Success;
}

/* static */ Status OpenSSLCertificateVerifier::createFromCAFile(
    std::unique_ptr<OpenSSLCertificateVerifier>& ret,
    Error& err,
    VerificationContext context,
    const std::string& caFile) {
  auto store = folly::ssl::OpenSSLCertUtils::readStoreFromFile(caFile);
  return create(ret, err, context, std::move(store));
}

/* static */ Status OpenSSLCertificateVerifier::createFromCAFiles(
    std::unique_ptr<OpenSSLCertificateVerifier>& ret,
    Error& err,
    VerificationContext context,
    const std::vector<std::string>& caFiles) {
  std::string certBuffer;
  for (const auto& caFile : caFiles) {
    std::string readBuffer;
    if (!folly::readFile(caFile.c_str(), readBuffer)) {
      return err.error(
          folly::to<std::string>("Could not read store file: ", caFile));
    }
    folly::toAppend(readBuffer, &certBuffer);
  }
  return create(
      ret,
      err,
      context,
      folly::ssl::OpenSSLCertUtils::readStoreFromBuffer(
          folly::StringPiece(certBuffer)));
}

Status OpenSSLCertificateVerifier::verify(
    std::shared_ptr<const Cert>& ret,
    Error& err,
    const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs) const {
  folly::ssl::X509StoreCtxUniquePtr ctx;
  FIZZ_RETURN_ON_ERROR(verifyWithX509StoreCtx(ctx, err, certs));
  // Just return the original cert in the default case
  ret = certs.front();
  return Status::Success;
}

Status OpenSSLCertificateVerifier::verifyWithX509StoreCtx(
    folly::ssl::X509StoreCtxUniquePtr& ret,
    Error& err,
    const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs) const {
  if (certs.empty()) {
    return err.error("no certificates to verify");
  }

  auto leafCert = certs.front()->getX509();

  auto certChainStack = std::unique_ptr<STACK_OF(X509), STACK_OF_X509_deleter>(
      sk_X509_new_null());
  if (!certChainStack) {
    return err.error("", folly::none, Error::Category::StdBadAlloc);
  }

  for (size_t i = 1; i < certs.size(); i++) {
    sk_X509_push(certChainStack.get(), certs[i]->getX509().get());
  }

  auto ctx = folly::ssl::X509StoreCtxUniquePtr(X509_STORE_CTX_new());
  if (!ctx) {
    return err.error("", folly::none, Error::Category::StdBadAlloc);
  }

  X509_STORE* storePtr = nullptr;
  if (x509Store_) {
    storePtr = x509Store_.get();
  } else {
    FIZZ_RETURN_ON_ERROR(getDefaultX509Store(storePtr, err));
  }
  if (X509_STORE_CTX_init(
          ctx.get(), storePtr, leafCert.get(), certChainStack.get()) != 1) {
    return err.error("failed to initialize store context");
  }

  if (X509_STORE_CTX_set_default(
          ctx.get(),
          context_ == VerificationContext::Server ? "ssl_client"
                                                  : "ssl_server") != 1) {
    return err.error("failed to set default verification method");
  }

  if (customVerifyCallback_) {
    X509_STORE_CTX_set_verify_cb(ctx.get(), customVerifyCallback_);
  }

  folly::ssl::X509VerifyParam param(X509_VERIFY_PARAM_new());
  if (!param) {
    return err.error("", folly::none, Error::Category::StdBadAlloc);
  }

  if (X509_VERIFY_PARAM_set_flags(param.get(), X509_V_FLAG_X509_STRICT) != 1) {
    return err.error("failed to set strict certificate checking");
  }

  if (X509_VERIFY_PARAM_set1(
          X509_STORE_CTX_get0_param(ctx.get()), param.get()) != 1) {
    return err.error("failed to apply verification parameters");
  }

  int result = 0;
  // if openssl is not built with TSAN then we can get a TSAN false positive
  // when calling X509_verify_cert from multiple threads
  {
    folly::annotate_ignore_thread_sanitizer_guard g(__FILE__, __LINE__);
    result = X509_verify_cert(ctx.get());
  }

  if (result != 1) {
    const auto errorInt = X509_STORE_CTX_get_error(ctx.get());
    return err.error(
        fmt::format(
            "certificate verification failed: {}",
            X509_verify_cert_error_string(errorInt)),
        toTLSAlert(errorInt),
        Error::Category::Verifier);
  }

  ret = std::move(ctx);
  return Status::Success;
}

/* static */ Status OpenSSLCertificateVerifier::createAuthorities(
    CertificateAuthorities& ret,
    Error& err,
    X509_STORE* store) {
  // X509_STORE stores CA certs as objects in this stack.
  STACK_OF(X509_OBJECT)* entries = X509_STORE_get0_objects(store);
  CertificateAuthorities auth;
  for (int i = 0; i < sk_X509_OBJECT_num(entries); i++) {
    X509_OBJECT* obj = sk_X509_OBJECT_value(entries, i);
    if (X509_OBJECT_get_type(obj) == X509_LU_X509) {
      auto certIssuer = X509_get_subject_name(X509_OBJECT_get0_X509(obj));
      int dnLength = i2d_X509_NAME(certIssuer, nullptr);
      if (dnLength < 0) {
        return err.error("Error computing DN length");
      }
      DistinguishedName dn;
      dn.encoded_name = folly::IOBuf::create(dnLength);
      auto dnData = dn.encoded_name->writableData();
      dnLength = i2d_X509_NAME(certIssuer, &dnData);
      if (dnLength < 0) {
        return err.error("Error encoding DN in DER format");
      }
      dn.encoded_name->append(dnLength);
      auth.authorities.push_back(std::move(dn));
    }
  }
  ret = std::move(auth);
  return Status::Success;
}

Status OpenSSLCertificateVerifier::getDefaultX509Store(
    X509_STORE*& ret,
    Error& err) {
  static folly::once_flag flag;
  static folly::ssl::X509StoreUniquePtr defaultStore;

  Status status = Status::Success;
  bool initialized = folly::try_call_once(flag, [&]() noexcept {
    folly::ssl::X509StoreUniquePtr store(X509_STORE_new());
    if (!store) {
      status = err.error("", folly::none, Error::Category::StdBadAlloc);
      return false;
    }
    if (X509_STORE_set_default_paths(store.get()) != 1) {
      status = err.error("failed to set default paths");
      return false;
    }
    defaultStore = std::move(store);
    return true;
  });
  if (initialized) {
    ret = defaultStore.get();
  }
  return status;
}

Status OpenSSLCertificateVerifier::getCertificateRequestExtensions(
    std::vector<Extension>& ret,
    Error& err) const {
  std::vector<Extension> exts;
  Extension ext;
  FIZZ_RETURN_ON_ERROR(encodeExtension(ext, err, authorities_));
  exts.push_back(std::move(ext));
  ret = std::move(exts);
  return Status::Success;
}

} // namespace openssl
} // namespace fizz
