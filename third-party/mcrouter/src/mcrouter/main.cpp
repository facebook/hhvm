/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <unordered_map>

#include <glog/logging.h>

#include <folly/Singleton.h>

#include "mcrouter/StandaloneUtils.h"
#include "mcrouter/options.h"
#include "mcrouter/standalone_options.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

namespace {

McrouterOptions opts;
McrouterStandaloneOptions standaloneOpts;

} // anonymous namespace

int main(int argc, char** argv) {
  folly::SingletonVault::singleton()->registrationComplete();
  FLAGS_v = 1;
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  CmdLineOptions cmdLineOpts =
      parseCmdLineOptions(argc, argv, MCROUTER_PACKAGE_STRING);

  std::unordered_map<std::string, std::string> optionsDict;
  std::unordered_map<std::string, std::string> standaloneOptionsDict;
  getFlavorOptionsAndApplyOverrides(
      cmdLineOpts, optionsDict, standaloneOptionsDict);

  setupStandaloneMcrouter(
      "mcrouter" /* default serviceName */,
      cmdLineOpts,
      optionsDict,
      standaloneOptionsDict,
      opts,
      standaloneOpts);

  runStandaloneMcrouter(cmdLineOpts, opts, standaloneOpts);

  return 0;
}
