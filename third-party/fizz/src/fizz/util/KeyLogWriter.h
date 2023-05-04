/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/protocol/Types.h>
#include <fizz/util/KeyLogTypes.h>
#include <fmt/format.h>
#include <folly/Range.h>
#include <fstream>

namespace fizz {
/**
 * Dump NSS Key Log File.
 * More details can be found here:
 * https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format.
 * The syntax: <Label> <space> <ClientRandom> <space> <Secret>.
 */
class KeyLogWriter {
 public:
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
   * Append a new log line to the key log file.
   * @param clientRandom, input, 32 bytes random value from the Client Hello
   * message; identifier of the each NSS key log line.
   * @param label, input, type of the log defined in
   * https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format.
   * @param secret, input, the secret corresponding to the secret type.
   */
  void write(
      const fizz::Random& clientRandom,
      NSSLabel label,
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
      NSSLabel label,
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
  static std::string labelToString(NSSLabel label) {
    switch (label) {
      case NSSLabel::RSA:
        return "RSA";
      case NSSLabel::CLIENT_RANDOM:
        return "CLIENT_RANDOM";
      case NSSLabel::CLIENT_EARLY_TRAFFIC_SECRET:
        return "CLIENT_EARLY_TRAFFIC_SECRET";
      case NSSLabel::CLIENT_HANDSHAKE_TRAFFIC_SECRET:
        return "CLIENT_HANDSHAKE_TRAFFIC_SECRET";
      case NSSLabel::SERVER_HANDSHAKE_TRAFFIC_SECRET:
        return "SERVER_HANDSHAKE_TRAFFIC_SECRET";
      case NSSLabel::CLIENT_TRAFFIC_SECRET_0:
        return "CLIENT_TRAFFIC_SECRET_0";
      case NSSLabel::SERVER_TRAFFIC_SECRET_0:
        return "SERVER_TRAFFIC_SECRET_0";
      case NSSLabel::EARLY_EXPORTER_SECRET:
        return "EARLY_EXPORTER_SECRET";
      case NSSLabel::EXPORTER_SECRET:
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
