/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/RandomGenerator.h>
#include <fizz/util/KeyLogTypes.h>
#include <fizz/util/KeyLogWriter.h>
#include <folly/portability/GTest.h>

namespace fizz {
namespace test {

TEST(KeyLogWriterTest, WriteLog) {
  auto random = RandomGenerator<32>().generateRandom();
  auto secret = RandomGenerator<32>().generateRandom();
  std::string logLine = KeyLogWriter::generateLogLine(
      random, NSSLabel::CLIENT_HANDSHAKE_TRAFFIC_SECRET, secret);

  std::vector<std::string> fields;
  folly::split(" ", logLine, fields);
  EXPECT_EQ(fields.size(), 3);
  EXPECT_EQ(fields[0], "CLIENT_HANDSHAKE_TRAFFIC_SECRET");
  EXPECT_EQ(fields[1], folly::hexlify(random));
  EXPECT_EQ(fields[2], folly::hexlify(secret) + "\n");
}

} // namespace test
} // namespace fizz
