/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>

#include <proxygen/lib/http/codec/compress/HPACKEncoder.h>
#include <proxygen/lib/http/codec/compress/HPACKHeader.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionSimulator.h>

DEFINE_string(input, "", "File containing requests");
DEFINE_string(scheme, "qpack", "Scheme: <qpack|qmin|hpack>");

DEFINE_int32(rtt, 100, "Simulated RTT");
DEFINE_double(lossp, 0.0, "Loss Probability");
DEFINE_double(delayp, 0.05, "Delay Probability");
DEFINE_int32(delay, 100, "Max extra delay");
DEFINE_int32(ooo_thresh, 0, "First seqn to allow ooo");
DEFINE_int32(table_size, 4096, "HPACK dynamic table size");
DEFINE_int64(seed, 0, "RNG seed");
DEFINE_bool(blend, true, "Blend all facebook.com and fbcdn.net domains");
DEFINE_int32(max_blocking,
             100,
             "Maximum number of vulnerable/blocking header blocks");
DEFINE_bool(same_packet_compression,
            true,
            "Allow QPACK to compress across "
            "headers the same packet");

using namespace proxygen::compress;

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv, true);
  if (FLAGS_same_packet_compression) {
    LOG(WARNING) << "Same packet compression no longer supported";
  }

  if (FLAGS_input.empty()) {
    LOG(ERROR) << "Must supply a filename";
    return 1;
  }

  SchemeType t = SchemeType::QPACK;
  if (FLAGS_scheme == "qpack") {
    LOG(INFO) << "Using QPACK";
    t = SchemeType::QPACK;
  } else if (FLAGS_scheme == "qmin") {
    LOG(INFO) << "Using QMIN";
    t = SchemeType::QMIN;
  } else if (FLAGS_scheme == "hpack") {
    LOG(INFO) << "Using HPACK with table size=" << FLAGS_table_size;
    t = SchemeType::HPACK;
  } else {
    LOG(ERROR) << "Unsupported scheme";
    return 1;
  }

  if (FLAGS_seed == 0) {
    FLAGS_seed = folly::Random::rand64();
    std::cout << "Seed: " << FLAGS_seed << std::endl;
  }
  SimParams p{t,
              FLAGS_seed,
              std::chrono::milliseconds(FLAGS_rtt),
              FLAGS_lossp,
              FLAGS_delayp,
              std::chrono::milliseconds(FLAGS_delay),
              uint16_t(FLAGS_ooo_thresh),
              FLAGS_blend,
              FLAGS_same_packet_compression,
              uint32_t(FLAGS_table_size),
              uint32_t(FLAGS_max_blocking)};
  CompressionSimulator sim(p);
  if (sim.readInputFromFileAndSchedule(FLAGS_input)) {
    sim.run();
  }

  return 0;
}
