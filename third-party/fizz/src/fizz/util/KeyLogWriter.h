/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/Types.h>
#include <fmt/format.h>
#include <folly/Range.h>
#include <fstream>

namespace fizz {

/**
 * Dump NSS Key Log File.
 * More details can be found here:
 * https://datatracker.ietf.org/doc/draft-ietf-tls-keylogfile/
 * The syntax: <Label> <space> <ClientRandom> <space> <Secret>.
 *
 * The values here map with values in a tracepoint script (fizztrace.py).
 * They have no correlation with the NSS standard.
 */
class KeyLogWriter {
 public:
  enum class Label : uint64_t {
    RSA = 0, // 48 bytes for the premaster secret, encoded as 96 hexadecimal
             // characters
    CLIENT_RANDOM =
        1, // 48 bytes for the master secret, encoded as 96 hexadecimal
           // characters (for SSL 3.0, TLS 1.0, 1.1 and 1.2)
    CLIENT_EARLY_TRAFFIC_SECRET = 2, // the hex-encoded early traffic secret for
                                     // the client side (for TLS 1.3)
    CLIENT_HANDSHAKE_TRAFFIC_SECRET =
        3, // the hex-encoded handshake traffic secret
           // for the client side (for TLS 1.3)
    SERVER_HANDSHAKE_TRAFFIC_SECRET =
        4, // the hex-encoded handshake traffic secret
           // for the server side (for TLS 1.3)
    CLIENT_TRAFFIC_SECRET_0 = 5, // the first hex-encoded application traffic
                                 // secret for the client side (for TLS 1.3)
    SERVER_TRAFFIC_SECRET_0 = 6, // the first hex-encoded application traffic
                                 // secret for the server side (for TLS 1.3)
    EARLY_EXPORTER_SECRET = 7, // the hex-encoded early exporter secret (for
                               // TLS 1.3, used for 0-RTT keys in older QUIC
                               // drafts).
    EXPORTER_SECRET = 8 // the hex-encoded exporter secret (for TLS 1.3, used
                        // for 1-RTT keys in older QUIC drafts)
  };

  /**
   * Instantiate a KeyLogWriter.
   * @param fileName, input, name of the file to keep key logs.
   */
  KeyLogWriter(const std::string& fileName) {
    outputFile_.open(fileName.c_str(), std::ios_base::app);
    if (outputFile_.fail()) {
      throw std::runtime_error("Error opening NSS key log output file");
    }
  }

  /**
   * Convert SecretType to NSS Keylog label equivalent.
   * @param secretType The secretType to convert to keylog label.
   * @return the keylog label for secretType
   */
  static folly::Optional<Label> secretToNSSLabel(SecretType secretType) {
    switch (secretType.type()) {
      case SecretType::Type::EarlySecrets_E:
        switch (secretType.tryAsEarlySecrets()) {
          case EarlySecrets::ExternalPskBinder:
            return folly::none;
          case EarlySecrets::ResumptionPskBinder:
            return folly::none;
          case EarlySecrets::ClientEarlyTraffic:
            return Label::CLIENT_EARLY_TRAFFIC_SECRET;
          case EarlySecrets::EarlyExporter:
            return Label::EARLY_EXPORTER_SECRET;
          case EarlySecrets::ECHAcceptConfirmation:
            return folly::none;
          case EarlySecrets::HRRECHAcceptConfirmation:
            return folly::none;
        }
      case SecretType::Type::HandshakeSecrets_E:
        switch (secretType.tryAsHandshakeSecrets()) {
          case HandshakeSecrets::ClientHandshakeTraffic:
            return Label::CLIENT_HANDSHAKE_TRAFFIC_SECRET;
          case HandshakeSecrets::ServerHandshakeTraffic:
            return Label::SERVER_HANDSHAKE_TRAFFIC_SECRET;
          case HandshakeSecrets::ECHAcceptConfirmation:
            return folly::none;
        }
      case SecretType::Type::MasterSecrets_E:
        switch (secretType.tryAsMasterSecrets()) {
          case MasterSecrets::ExporterMaster:
            return Label::EXPORTER_SECRET;
          case MasterSecrets::ResumptionMaster:
            return folly::none;
        }
      case SecretType::Type::AppTrafficSecrets_E:
        switch (secretType.tryAsAppTrafficSecrets()) {
          case AppTrafficSecrets::ClientAppTraffic:
            return Label::CLIENT_TRAFFIC_SECRET_0;
          case AppTrafficSecrets::ServerAppTraffic:
            return Label::SERVER_TRAFFIC_SECRET_0;
        }
    }
    return folly::none;
  }

  /**
   * Append a new log line to the key log file.
   * @param clientRandom, input, 32 bytes random value from the Client Hello
   * message; identifier of the each NSS key log line.
   * @param label, input, type of the log defined in
   * https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format.
   * @param secret, input, the secret corresponding to the secret type.
   */
  void write(
      const fizz::Random& clientRandom,
      KeyLogWriter::Label label,
      const folly::ByteRange& secret) {
    outputFile_ << generateLogLine(clientRandom, label, secret);
  }

  /**
   * Generate a NSS key log line.
   * @param clientRandom, input, 32 bytes random value from the Client Hello
   * message; identifier of the each NSS key log line.
   * @param label, input, type of the log defined in
   * https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format.
   * @param secret, input, the secret corresponding to the secret type.
   * @return the NSS key log line ended with a new line character.
   */
  static std::string generateLogLine(
      const fizz::Random& clientRandom,
      KeyLogWriter::Label label,
      const folly::ByteRange& secret) {
    return fmt::format(
        "{0} {1} {2}\n",
        labelToString(label),
        folly::hexlify(clientRandom),
        folly::hexlify(secret));
  }

 private:
  /**
   * Convert the Label enumerate to string.
   */
  static std::string labelToString(KeyLogWriter::Label label) {
    switch (label) {
      case Label::RSA:
        return "RSA";
      case Label::CLIENT_RANDOM:
        return "CLIENT_RANDOM";
      case Label::CLIENT_EARLY_TRAFFIC_SECRET:
        return "CLIENT_EARLY_TRAFFIC_SECRET";
      case Label::CLIENT_HANDSHAKE_TRAFFIC_SECRET:
        return "CLIENT_HANDSHAKE_TRAFFIC_SECRET";
      case Label::SERVER_HANDSHAKE_TRAFFIC_SECRET:
        return "SERVER_HANDSHAKE_TRAFFIC_SECRET";
      case Label::CLIENT_TRAFFIC_SECRET_0:
        return "CLIENT_TRAFFIC_SECRET_0";
      case Label::SERVER_TRAFFIC_SECRET_0:
        return "SERVER_TRAFFIC_SECRET_0";
      case Label::EARLY_EXPORTER_SECRET:
        return "EARLY_EXPORTER_SECRET";
      case Label::EXPORTER_SECRET:
        return "EXPORTER_SECRET";
      default:
        break;
    }

    return "";
  }

 private:
  std::ofstream outputFile_;
};

} // namespace fizz
