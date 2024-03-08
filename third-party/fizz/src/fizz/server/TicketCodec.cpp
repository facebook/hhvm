/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/TicketCodec.h>
#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>
#include <folly/ssl/OpenSSLCertUtils.h>

namespace fizz {
std::string toString(fizz::server::CertificateStorage storage) {
  using fizz::server::CertificateStorage;
  switch (storage) {
    case CertificateStorage::None:
      return "None";
    case CertificateStorage::X509:
      return "X509";
    case CertificateStorage::IdentityOnly:
      return "IdentityOnly";
    default:
      return "Unknown storage";
  }
}
namespace server {
void appendClientCertificate(
    CertificateStorage storage,
    const std::shared_ptr<const Cert>& cert,
    folly::io::Appender& appender) {
  Buf clientCertBuf = folly::IOBuf::create(0);
  CertificateStorage selectedStorage;

  auto serializeIdentity = [&]() {
    selectedStorage = CertificateStorage::IdentityOnly;
    clientCertBuf = folly::IOBuf::copyBuffer(cert->getIdentity());
  };

  auto trySerializeX509 = [&]() {
    auto opensslCert =
        dynamic_cast<const folly::OpenSSLTransportCertificate*>(cert.get());
    if (opensslCert && opensslCert->getX509()) {
      selectedStorage = CertificateStorage::X509;
      clientCertBuf =
          folly::ssl::OpenSSLCertUtils::derEncode(*opensslCert->getX509());
    } else {
      serializeIdentity();
    }
  };

  if (!cert || storage == CertificateStorage::None) {
    selectedStorage = CertificateStorage::None;
  } else if (storage == CertificateStorage::X509) {
    trySerializeX509();
  } else {
    serializeIdentity();
  }
  fizz::detail::write(selectedStorage, appender);
  if (selectedStorage != CertificateStorage::None) {
    fizz::detail::writeBuf<uint16_t>(clientCertBuf, appender);
  }
}

std::shared_ptr<const Cert> readClientCertificate(
    folly::io::Cursor& cursor,
    const Factory& factory) {
  CertificateStorage storage;
  fizz::detail::read(storage, cursor);
  switch (storage) {
    case CertificateStorage::None:
      return nullptr;
    case CertificateStorage::X509: {
      CertificateEntry certEntry;
      fizz::detail::readBuf<uint16_t>(certEntry.cert_data, cursor);
      return factory.makePeerCert(std::move(certEntry), /* leaf */ true);
    }
    case CertificateStorage::IdentityOnly: {
      Buf ident;
      fizz::detail::readBuf<uint16_t>(ident, cursor);
      return factory.makeIdentityOnlyCert(
          ident->moveToFbString().toStdString());
    }
  }

  return nullptr;
}
} // namespace server
} // namespace fizz
