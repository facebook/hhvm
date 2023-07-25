/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>
#include <proxygen/httpclient/samples/httperf2/HTTPerf2.h>

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage(std::string("\n\nusage: httperf2 (see flags)\n"));
  auto _ = folly::Init(&argc, &argv, true);

  return proxygen::httperf2();
}
