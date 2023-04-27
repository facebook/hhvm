/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GFlags.h>

#include <folly/init/Init.h>
#include <folly/ssl/Init.h>

#include <proxygen/httpserver/samples/hq/ConnIdLogger.h>
#include <proxygen/httpserver/samples/hq/HQClient.h>
#include <proxygen/httpserver/samples/hq/HQCommandLine.h>
#include <proxygen/httpserver/samples/hq/HQParams.h>
#include <proxygen/httpserver/samples/hq/HQServerModule.h>
#include <proxygen/lib/transport/PersistentQuicPskCache.h>

using namespace quic::samples;

int main(int argc, char* argv[]) {
  auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock().now().time_since_epoch())
                       .count();
#if FOLLY_HAVE_LIBGFLAGS
  // Enable glog logging to stderr by default.
  gflags::SetCommandLineOptionWithMode(
      "logtostderr", "1", gflags::SET_FLAGS_DEFAULT);
#endif
  folly::init(&argc, &argv, false);
  folly::ssl::init();
  int err = 0;

  auto expectedParams = initializeParamsFromCmdline();
  if (expectedParams) {
    auto& params = expectedParams.value();
    // TODO: move sink to params
    proxygen::ConnIdLogSink sink(params.logdir, params.logprefix);
    if (sink.isValid()) {
      AddLogSink(&sink);
    } else if (!params.logdir.empty()) {
      LOG(ERROR) << "Cannot open " << params.logdir;
    }

    switch (params.mode) {
      case HQMode::SERVER:
        startServer(boost::get<HQToolServerParams>(params.params));
        break;
      case HQMode::CLIENT:
        err = startClient(boost::get<HQToolClientParams>(params.params));
        break;
      default:
        LOG(ERROR) << "Unknown mode specified: ";
        return -1;
    }
    if (params.logRuntime) {
      LOG(INFO) << "Run time: "
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock().now().time_since_epoch())
                           .count() -
                       startTime
                << "ms";
    }
    return err;
  } else {
    for (auto& param : expectedParams.error()) {
      LOG(ERROR) << "Invalid param: " << param.name << " " << param.value << " "
                 << param.errorMsg;
    }
    return -1;
  }
}
