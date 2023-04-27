/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GFlags.h>
#include <glog/logging.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>

using namespace proxygen;
using namespace std;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
  FakeHTTPCodecCallback callbacks;
  HTTP1xCodec codec(TransportDirection::DOWNSTREAM);
  codec.setCallback(&callbacks);
  parse(&codec, Data, Size, Size);
  return 0;
}
