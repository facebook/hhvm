/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/crypto/Hasher.h>
#include <fizz/crypto/Hmac.h>
#include <fizz/crypto/Utils.h>
#include <fizz/extensions/exportedauth/ExportedAuthenticator.h>
#include <fizz/extensions/exportedauth/Util.h>
#include <fizz/protocol/DefaultFactory.h>

namespace fizz {

Status ExportedAuthenticator::getAuthenticatorRequest(
    Buf& ret,
    Error& err,
    Buf certificateRequestContext,
    std::vector<fizz::Extension> extensions) {
  if (!certificateRequestContext || certificateRequestContext->empty()) {
    return err.error(
        "certificate request context must not be empty",
        AlertDescription::illegal_parameter);
  }

  CertificateRequest cr;
  cr.certificate_request_context = std::move(certificateRequestContext);
  cr.extensions = std::move(extensions);
  FIZZ_RETURN_ON_ERROR(encode<CertificateRequest>(ret, err, std::move(cr)));
  return Status::Success;
}

Status ExportedAuthenticator::getAuthenticator(
    Buf& ret,
    Error& err,
    const fizz::AsyncFizzBase& transport,
    Direction dir,
    const SelfCert& cert,
    Buf authenticatorRequest) {
  auto cipher = transport.getCipher();
  HashFunction hashFunction;
  size_t hashLength;
  const HasherFactoryWithMetadata* makeHasher = nullptr;
  FIZZ_RETURN_ON_ERROR(getHashFunction(hashFunction, err, *cipher));
  FIZZ_RETURN_ON_ERROR(getHashSize(hashLength, err, hashFunction));
  FIZZ_RETURN_ON_ERROR(
      ::fizz::DefaultFactory().makeHasherFactory(
          makeHasher, err, hashFunction));
  FIZZ_DCHECK_NE(makeHasher, nullptr);

  auto supportedSchemes = transport.getSupportedSigSchemes();
  Buf handshakeContext;
  Buf finishedMacKey;
  if (dir == Direction::UPSTREAM) {
    handshakeContext = transport.getExportedKeyingMaterial(
        "EXPORTER-client authenticator handshake context", nullptr, hashLength);
    finishedMacKey = transport.getExportedKeyingMaterial(
        "EXPORTER-client authenticator finished key", nullptr, hashLength);
  } else {
    handshakeContext = transport.getExportedKeyingMaterial(
        "EXPORTER-server authenticator handshake context", nullptr, hashLength);
    finishedMacKey = transport.getExportedKeyingMaterial(
        "EXPORTER-server authenticator finished key", nullptr, hashLength);
  }
  return makeAuthenticator(
      ret,
      err,
      makeHasher,
      supportedSchemes,
      cert,
      std::move(authenticatorRequest),
      std::move(handshakeContext),
      std::move(finishedMacKey),
      CertificateVerifyContext::Authenticator);
}

Status ExportedAuthenticator::getAuthenticatorContext(
    Buf& ret,
    Error& err,
    Buf authenticator) {
  folly::IOBufQueue authQueue{folly::IOBufQueue::cacheChainLength()};
  authQueue.append(std::move(authenticator));
  folly::Optional<Param> param;
  FIZZ_RETURN_ON_ERROR(
      fizz::ReadRecordLayer::decodeHandshakeMessage(param, err, authQueue));
  auto certMsgPtr = param->asCertificateMsg();
  if (!certMsgPtr) {
    return err.error("Param isn't cert msg");
  }
  ret = std::move(certMsgPtr->certificate_request_context);
  return Status::Success;
}

Status ExportedAuthenticator::validateAuthenticator(
    folly::Optional<std::vector<CertificateEntry>>& ret,
    Error& err,
    const fizz::AsyncFizzBase& transport,
    Direction dir,
    Buf authenticatorRequest,
    Buf authenticator) {
  auto cipher = transport.getCipher();
  HashFunction hashFunction;
  size_t hashLength;
  const HasherFactoryWithMetadata* makeHasher = nullptr;
  FIZZ_RETURN_ON_ERROR(getHashFunction(hashFunction, err, *cipher));
  FIZZ_RETURN_ON_ERROR(getHashSize(hashLength, err, hashFunction));
  FIZZ_RETURN_ON_ERROR(
      ::fizz::DefaultFactory().makeHasherFactory(
          makeHasher, err, hashFunction));

  Buf handshakeContext;
  Buf finishedMacKey;
  if (dir == Direction::UPSTREAM) {
    handshakeContext = transport.getExportedKeyingMaterial(
        "EXPORTER-server authenticator handshake context", nullptr, hashLength);
    finishedMacKey = transport.getExportedKeyingMaterial(
        "EXPORTER-server authenticator finished key", nullptr, hashLength);
  } else {
    handshakeContext = transport.getExportedKeyingMaterial(
        "EXPORTER-client authenticator handshake context", nullptr, hashLength);
    finishedMacKey = transport.getExportedKeyingMaterial(
        "EXPORTER-client authenticator finished key", nullptr, hashLength);
  }
  FIZZ_RETURN_ON_ERROR(validate(
      ret,
      err,
      makeHasher,
      std::move(authenticatorRequest),
      std::move(authenticator),
      std::move(handshakeContext),
      std::move(finishedMacKey),
      CertificateVerifyContext::Authenticator));
  return Status::Success;
}

Status ExportedAuthenticator::makeAuthenticator(
    Buf& ret,
    Error& err,
    const HasherFactoryWithMetadata* makeHasher,
    std::vector<SignatureScheme> supportedSchemes,
    const SelfCert& cert,
    Buf authenticatorRequest,
    Buf handshakeContext,
    Buf finishedMacKey,
    CertificateVerifyContext context) {
  Buf certificateRequestContext;
  std::vector<fizz::Extension> extensions;
  FIZZ_RETURN_ON_ERROR(
      detail::decodeAuthRequest(
          certificateRequestContext, extensions, err, authenticatorRequest));
  folly::Optional<SignatureScheme> scheme;
  FIZZ_RETURN_ON_ERROR(
      detail::getSignatureScheme(
          scheme, err, supportedSchemes, cert, extensions));
  // No proper signature scheme could be selected, return an empty
  // authenticator.
  if (!scheme) {
    FIZZ_RETURN_ON_ERROR(
        detail::getEmptyAuthenticator(
            ret,
            err,
            makeHasher,
            std::move(authenticatorRequest),
            std::move(handshakeContext),
            std::move(finishedMacKey)));
    return Status::Success;
  }
  // Compute CertificateMsg.
  CertificateMsg certificate;
  FIZZ_RETURN_ON_ERROR(cert.getCertMessage(
      certificate, err, std::move(certificateRequestContext)));
  Buf encodedCertMsg;
  FIZZ_RETURN_ON_ERROR(
      encodeHandshake(encodedCertMsg, err, std::move(certificate)));
  // Compute CertificateVerify.
  auto transcript = detail::computeTranscript(
      handshakeContext, authenticatorRequest, encodedCertMsg);
  auto transcriptHash = detail::computeTranscriptHash(makeHasher, transcript);
  auto sig = cert.sign(*scheme, context, transcriptHash->coalesce());
  CertificateVerify verify;
  verify.algorithm = *scheme;
  verify.signature = std::move(sig);
  Buf encodedCertificateVerify;
  FIZZ_RETURN_ON_ERROR(
      encodeHandshake(encodedCertificateVerify, err, std::move(verify)));
  // Compute Finished.
  auto finishedTranscript =
      detail::computeFinishedTranscript(transcript, encodedCertificateVerify);
  auto finishedTranscriptHash =
      detail::computeTranscriptHash(makeHasher, finishedTranscript);
  auto verifyData = detail::getFinishedData(
      makeHasher, finishedMacKey, finishedTranscriptHash);
  Finished finished;
  finished.verify_data = std::move(verifyData);
  Buf encodedFinished;
  FIZZ_RETURN_ON_ERROR(
      encodeHandshake(encodedFinished, err, std::move(finished)));

  ret = detail::computeTranscript(
      encodedCertMsg, encodedCertificateVerify, encodedFinished);
  return Status::Success;
}

Status ExportedAuthenticator::validate(
    folly::Optional<std::vector<CertificateEntry>>& ret,
    Error& err,
    const HasherFactoryWithMetadata* makeHasher,
    Buf authenticatorRequest,
    Buf authenticator,
    Buf handshakeContext,
    Buf finishedMacKey,
    CertificateVerifyContext context) {
  folly::IOBufQueue authQueue{folly::IOBufQueue::cacheChainLength()};
  constexpr uint16_t capacity = 256;
  // Clone the authenticator which is later compared to the re-calculated empty
  // authenticator.
  auto authClone = authenticator->clone();
  authQueue.append(std::move(authenticator));
  folly::Optional<Param> param;
  FIZZ_RETURN_ON_ERROR(
      fizz::ReadRecordLayer::decodeHandshakeMessage(param, err, authQueue));
  if (!param) {
    ret = folly::none;
    return Status::Success;
  }
  // First check if the authenticator is empty.
  auto finished = param->asFinished();
  if (finished) {
    Buf emptyAuth;
    FIZZ_RETURN_ON_ERROR(
        detail::getEmptyAuthenticator(
            emptyAuth,
            err,
            makeHasher,
            std::move(authenticatorRequest),
            std::move(handshakeContext),
            std::move(finishedMacKey)));
    if (CryptoUtils::equal(emptyAuth->coalesce(), authClone->coalesce())) {
      ret = std::vector<CertificateEntry>();
    } else {
      ret = folly::none;
    }
    return Status::Success;
  }
  folly::Optional<Param> param2;
  FIZZ_RETURN_ON_ERROR(
      fizz::ReadRecordLayer::decodeHandshakeMessage(param2, err, authQueue));
  if (!param2) {
    ret = folly::none;
    return Status::Success;
  }
  folly::Optional<Param> param3;
  FIZZ_RETURN_ON_ERROR(
      fizz::ReadRecordLayer::decodeHandshakeMessage(param3, err, authQueue));
  if (!param3) {
    ret = folly::none;
    return Status::Success;
  }
  auto certMsg = param->asCertificateMsg();
  auto certVerify = param2->asCertificateVerify();
  finished = param3->asFinished();
  if (!certMsg || !certVerify || !finished ||
      certMsg->certificate_list.empty()) {
    ret = folly::none;
    return Status::Success;
  }

  auto leafCert = folly::IOBuf::create(capacity);
  folly::io::Appender appender(leafCert.get(), capacity);
  detail::writeBuf(certMsg->certificate_list.front().cert_data, appender);
  auto peerCert = openssl::CertUtils::makePeerCert(std::move(leafCert));
  Buf encodedCertMsg;
  FIZZ_RETURN_ON_ERROR(
      encodeHandshake(encodedCertMsg, err, std::move(*certMsg)));
  auto transcript = detail::computeTranscript(
      handshakeContext, authenticatorRequest, encodedCertMsg);
  auto transcriptHash = detail::computeTranscriptHash(makeHasher, transcript);
  try {
    if (peerCert->verify(
            err,
            certVerify->algorithm,
            context,
            transcriptHash->coalesce(),
            certVerify->signature->coalesce()) == Status::Fail) {
      ret = folly::none;
      return Status::Success;
    }
  } catch (const std::runtime_error&) {
    ret = folly::none;
    return Status::Success;
  }
  // Verify if Finished message matches.
  Buf encodedCertVerify;
  FIZZ_RETURN_ON_ERROR(
      encodeHandshake(encodedCertVerify, err, std::move(*certVerify)));
  auto finishedTranscript =
      detail::computeFinishedTranscript(transcript, encodedCertVerify);
  auto finishedTranscriptHash =
      detail::computeTranscriptHash(makeHasher, finishedTranscript);
  auto verifyData = detail::getFinishedData(
      makeHasher, finishedMacKey, finishedTranscriptHash);

  if (CryptoUtils::equal(
          finished->verify_data->coalesce(), verifyData->coalesce())) {
    ret = std::move(certMsg->certificate_list);
  } else {
    ret = folly::none;
  }
  return Status::Success;
}

namespace detail {

Status decodeAuthRequest(
    Buf& certRequestContext,
    std::vector<fizz::Extension>& extensions,
    Error& err,
    const Buf& authRequest) {
  if (authRequest && !(authRequest->empty())) {
    folly::io::Cursor cursor(authRequest.get());
    CertificateRequest decodedCertRequest;
    FIZZ_RETURN_ON_ERROR(
        decode<CertificateRequest>(decodedCertRequest, err, cursor));
    certRequestContext =
        std::move(decodedCertRequest.certificate_request_context);
    extensions = std::move(decodedCertRequest.extensions);
  } else {
    certRequestContext = folly::IOBuf::copyBuffer("");
    extensions.clear();
  }
  return Status::Success;
}

Buf computeTranscriptHash(
    const HasherFactoryWithMetadata* makeHasher,
    const Buf& toBeHashed) {
  auto hasher = makeHasher->make();

  auto hashLength = hasher->getHashLen();
  auto data = folly::IOBuf::create(hashLength);
  data->append(hashLength);
  auto transcriptHash =
      folly::MutableByteRange(data->writableData(), data->length());
  hasher->hash_update(*toBeHashed);
  hasher->hash_final(transcriptHash);
  return data;
}

void writeBuf(const Buf& buf, folly::io::Appender& out) {
  if (buf && !(buf->empty())) {
    auto current = buf.get();
    size_t chainElements = buf->countChainElements();
    for (size_t i = 0; i < chainElements; ++i) {
      out.push(current->data(), current->length());
      current = current->next();
    }
  }
}

Buf computeTranscript(
    const Buf& handshakeContext,
    const Buf& authenticatorRequest,
    const Buf& certificate) {
  constexpr uint16_t capacity = 256;
  auto out = folly::IOBuf::create(capacity);
  folly::io::Appender appender(out.get(), capacity);
  detail::writeBuf(handshakeContext, appender);
  detail::writeBuf(authenticatorRequest, appender);
  detail::writeBuf(certificate, appender);
  return out;
}

Buf computeFinishedTranscript(const Buf& crTranscript, const Buf& certVerify) {
  constexpr uint16_t capacity = 256;
  auto out = folly::IOBuf::create(capacity);
  folly::io::Appender appender(out.get(), capacity);
  detail::writeBuf(crTranscript, appender);
  detail::writeBuf(certVerify, appender);
  return out;
}

Buf getFinishedData(
    const HasherFactoryWithMetadata* makeHasher,
    Buf& finishedMacKey,
    const Buf& finishedTranscript) {
  auto hashLength = makeHasher->hashLength();
  auto data = folly::IOBuf::create(hashLength);
  data->append(hashLength);
  auto outRange = folly::MutableByteRange(data->writableData(), data->length());
  fizz::hmac(
      makeHasher, finishedMacKey->coalesce(), *finishedTranscript, outRange);
  return data;
}

Status getRequestedSchemes(
    folly::Optional<std::vector<SignatureScheme>>& ret,
    Error& err,
    const std::vector<fizz::Extension>& authRequestExtensions) {
  if (!(authRequestExtensions.empty())) {
    folly::Optional<SignatureAlgorithms> sigAlgsExtension;
    FIZZ_RETURN_ON_ERROR(
        getExtension<SignatureAlgorithms>(
            sigAlgsExtension, err, authRequestExtensions));
    if (sigAlgsExtension) {
      ret = sigAlgsExtension->supported_signature_algorithms;
    } else {
      ret = folly::none;
    }
  } else {
    ret = folly::none;
  }
  return Status::Success;
}

Status getSignatureScheme(
    folly::Optional<SignatureScheme>& ret,
    Error& err,
    const std::vector<SignatureScheme>& supportedSchemes,
    const SelfCert& cert,
    const std::vector<fizz::Extension>& authRequestExtensions) {
  folly::Optional<SignatureScheme> selectedScheme;
  const auto certSchemes = cert.getSigSchemes();
  folly::Optional<std::vector<SignatureScheme>> requestedSchemes;
  FIZZ_RETURN_ON_ERROR(
      getRequestedSchemes(requestedSchemes, err, authRequestExtensions));
  if (requestedSchemes) {
    for (const auto& scheme : supportedSchemes) {
      if (std::find(certSchemes.begin(), certSchemes.end(), scheme) !=
              certSchemes.end() &&
          std::find(
              (*requestedSchemes).begin(), (*requestedSchemes).end(), scheme) !=
              (*requestedSchemes).end()) {
        selectedScheme = scheme;
        break;
      }
    }
  }

  if (!selectedScheme) {
    FIZZ_VLOG(1) << "authenticator request without proper signature algorithms";
    for (const auto& scheme : supportedSchemes) {
      if (std::find(certSchemes.begin(), certSchemes.end(), scheme) !=
          certSchemes.end()) {
        selectedScheme = scheme;
        break;
      }
    }
  }
  ret = selectedScheme;
  return Status::Success;
}

Status getEmptyAuthenticator(
    Buf& ret,
    Error& err,
    const HasherFactoryWithMetadata* makeHasher,
    Buf authRequest,
    Buf handshakeContext,
    Buf finishedMacKey) {
  CertificateMsg emptyCertMsg;
  std::vector<fizz::Extension> unusedExtensions;
  FIZZ_RETURN_ON_ERROR(
      detail::decodeAuthRequest(
          emptyCertMsg.certificate_request_context,
          unusedExtensions,
          err,
          authRequest));
  Buf encodedEmptyCertMsg;
  FIZZ_RETURN_ON_ERROR(
      encodeHandshake(encodedEmptyCertMsg, err, std::move(emptyCertMsg)));
  auto emptyAuthTranscript = detail::computeTranscript(
      handshakeContext, authRequest, encodedEmptyCertMsg);
  auto emptyAuthTranscriptHash =
      detail::computeTranscriptHash(makeHasher, emptyAuthTranscript);
  auto finVerify = detail::getFinishedData(
      makeHasher, finishedMacKey, emptyAuthTranscriptHash);
  Finished emptyAuth;
  emptyAuth.verify_data = std::move(finVerify);
  FIZZ_RETURN_ON_ERROR(encodeHandshake(ret, err, std::move(emptyAuth)));
  return Status::Success;
}

} // namespace detail
} // namespace fizz
