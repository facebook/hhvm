/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/samples/hq/FizzContext.h>

#include <folly/portability/GTest.h>

#include <algorithm>

using namespace quic::samples;

TEST(FizzContextTest, DefaultSharesIncludeMLKEM768WhenAvailable) {
  HQBaseParams params;
  auto ctx = createFizzClientContext(params, {"h3"}, false, "", "");
  const auto& shares = ctx->getDefaultShares();

#if FIZZ_HAVE_OQS && OQS_ENABLE_KEM_ml_kem_768
  EXPECT_NE(
      std::find(shares.begin(), shares.end(), fizz::NamedGroup::X25519MLKEM768),
      shares.end())
      << "X25519MLKEM768 should be in default shares when OQS is available";
#else
  EXPECT_EQ(
      std::find(shares.begin(), shares.end(), fizz::NamedGroup::X25519MLKEM768),
      shares.end())
      << "X25519MLKEM768 should not be in default shares without OQS";
#endif

  EXPECT_NE(std::find(shares.begin(), shares.end(), fizz::NamedGroup::x25519),
            shares.end())
      << "x25519 should always be in default shares";
}
