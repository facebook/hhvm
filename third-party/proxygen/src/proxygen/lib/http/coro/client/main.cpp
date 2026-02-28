/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/HTTPClient.h"
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>

#include <iostream>

namespace {
folly::coro::Task<void> fetchUrl(folly::EventBase* evb, std::string url) {
  auto res = co_await co_awaitTry(proxygen::coro::HTTPClient::get(
      evb, std::move(url), std::chrono::seconds(5)));
  if (res.hasException()) {
    std::cerr << res.exception().what() << std::endl;
  } else {
    std::cout << res->body.move()->moveToFbString() << std::endl;
  }
}
} // namespace

int main(int argc, char* argv[]) {
  const folly::Init init(&argc, &argv);
  ::gflags::ParseCommandLineFlags(&argc, &argv, false);

  folly::EventBase evb;
  if (argc < 2) {
    std::cerr << "usage: http_client <url>" << std::endl;
    return 1;
  }
  co_withExecutor(&evb, fetchUrl(&evb, argv[1])).start();
  evb.loop();
}
