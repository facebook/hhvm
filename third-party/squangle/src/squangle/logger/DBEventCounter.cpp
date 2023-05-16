/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/logger/DBEventCounter.h"

#include <folly/Conv.h>
#include <folly/String.h>
#include <glog/logging.h>

namespace facebook {
namespace db {

void ConnectionContextBase::collectNormalValues(
    const AddNormalValueFunction& add) const {
  add("is_ssl", folly::to<std::string>(isSslConnection));
  add("is_ssl_session_reused", folly::to<std::string>(sslSessionReused));
  if (!sslVersion.empty()) {
    add("ssl_version", sslVersion);
  }
  if (sslCertCn.hasValue()) {
    add("ssl_server_cert_cn", sslCertCn.value());
  }
  if (sslCertSan.hasValue() && !sslCertSan.value().empty()) {
    add("ssl_server_cert_san", folly::join(',', sslCertSan.value()));
  }
  if (sslCertIdentities.hasValue() && !sslCertIdentities.value().empty()) {
    add("ssl_server_cert_identities",
        folly::join(',', sslCertIdentities.value()));
  }
  if (!endpointVersion.empty()) {
    add("endpoint_version", endpointVersion);
  }
}

void ConnectionContextBase::collectIntValues(
    const AddIntValueFunction& add) const {
  add("ssl_server_cert_validated", isServerCertValidated ? 1 : 0);
  add("ssl_client_identity_cert", isIdentityClientCert ? 1 : 0);
  if (certCacheSize.has_value()) {
    add("ssl_cert_cache_size", certCacheSize.value());
  }
}

folly::Optional<std::string> ConnectionContextBase::getNormalValue(
    folly::StringPiece key) const {
  if (key == "is_ssl") {
    return folly::to<std::string>(isSslConnection);
  } else if (key == "is_ssl_session_reused") {
    return folly::to<std::string>(sslSessionReused);
  } else if (key == "ssl_version" && !sslVersion.empty()) {
    return sslVersion;
  } else if (key == "ssl_server_cert_cn") {
    return sslCertCn;
  } else if (key == "ssl_server_cert_san") {
    if (sslCertSan.hasValue()) {
      return folly::join(',', sslCertSan.value());
    } else {
      return folly::none;
    }
  } else if (key == "ssl_server_cert_identities") {
    if (sslCertIdentities.hasValue()) {
      return folly::join(',', sslCertIdentities.value());
    } else {
      return folly::none;
    }
  } else if (key == "endpoint_version" && !endpointVersion.empty()) {
    return endpointVersion;
  } else if (key == "ssl_client_identity_cert") {
    return folly::to<std::string>(isIdentityClientCert);
  } else if (key == "ssl_cert_cache_size" && certCacheSize.has_value()) {
    return folly::to<std::string>(certCacheSize.value());
  } else {
    return folly::none;
  }
}

ExponentialMovingAverage::ExponentialMovingAverage(double smoothingFactor)
    : smoothingFactor_(smoothingFactor) {}

void ExponentialMovingAverage::addSample(double sample) {
  if (hasRegisteredFirstSample_) {
    currentValue_ =
        smoothingFactor_ * sample + (1 - smoothingFactor_) * currentValue_;
  } else {
    currentValue_ = sample;
    hasRegisteredFirstSample_ = true;
  }
}

void SimpleDbCounter::printStats() {
  LOG(INFO) << "Client Stats\n"
            << "Opened Connections " << numOpenedConnections() << "\n"
            << "Closed Connections " << numClosedConnections() << "\n"
            << "Failed Queries " << numFailedQueries() << "\n"
            << "Succeeded Queries " << numSucceededQueries() << "\n"
            << "Reused SSL Sessions " << numReusedSSLSessions() << "\n";
}
} // namespace db
} // namespace facebook
