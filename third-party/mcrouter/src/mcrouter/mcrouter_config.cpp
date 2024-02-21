/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>

#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/json/json.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McrouterLogger.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/config.h"
#include "mcrouter/flavor.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/standalone_options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

bool readLibmcrouterFlavor(
    folly::StringPiece flavor,
    std::unordered_map<std::string, std::string>& options) {
  std::unordered_map<std::string, std::string> standaloneOptions;
  return read_standalone_flavor(flavor.str(), options, standaloneOptions);
}

bool read_standalone_flavor(
    const std::string& flavor,
    std::unordered_map<std::string, std::string>& option_dict,
    std::unordered_map<std::string, std::string>& st_option_dict) {
  if (!readFlavor(flavor, st_option_dict, option_dict)) {
    LOG(ERROR) << "CRITICAL: Couldn't initialize from standalone flavor file "
               << flavor;
    return false;
  }
  return true;
}

std::unique_ptr<ConfigApi> createConfigApi(const McrouterOptions& opts) {
  return std::make_unique<ConfigApi>(opts);
}

std::string performOptionSubstitution(std::string str) {
  return str;
}

std::unique_ptr<McrouterLogger> createMcrouterLogger(
    CarbonRouterInstanceBase& router) {
  return std::make_unique<McrouterLogger>(router);
}

void extraValidateOptions(const McrouterOptions& opts) {
  if (!opts.config.empty()) {
    // If config option is used, other options are superseded
    if (!opts.config_file.empty() || !opts.config_str.empty()) {
      VLOG(1) << "config option will supersede config-file"
                 " and config-str options";
    }
    return;
  }

  size_t numSources = 0;
  if (!opts.config_file.empty()) {
    ++numSources;
  }
  if (!opts.config_str.empty()) {
    ++numSources;
  }
  if (numSources == 0) {
    throw std::logic_error("No configuration source");
  } else if (numSources > 1) {
    throw std::logic_error("More than one configuration source");
  }
}

void applyTestMode(McrouterOptions& opts) {
  opts.enable_failure_logging = false;
  opts.stats_logging_interval = 0;
}

McrouterOptions defaultTestOptions() {
  auto opts = McrouterOptions();
  applyTestMode(opts);
  return opts;
}

std::vector<std::string> defaultTestCommandLineArgs() {
  return {"--disable-failure-logging", "--stats-logging-interval=0"};
}

void logTkoEvent(ProxyBase& proxy, const TkoLog& tkoLog) {}

void logFailover(ProxyBase& proxy, const FailoverContext& failoverContext) {}

void initFailureLogger() {}

bool initCompression(CarbonRouterInstanceBase&) {
  return false;
}

void scheduleSingletonCleanup() {}

std::unordered_map<std::string, folly::dynamic> additionalConfigParams() {
  return std::unordered_map<std::string, folly::dynamic>();
}

void insertCustomStartupOpts(folly::dynamic& options) {}

std::string getBinPath(folly::StringPiece name) {
  if (name == "mcrouter") {
    return "./mcrouter/mcrouter";
  } else if (name == "mockmc") {
    return "./mcrouter/lib/network/test/mock_mc_server";
  }
  return "unknown";
}

void finalizeOptions(McrouterOptions&) {}

folly::dynamic readStaticJsonFile(folly::StringPiece file) {
  std::string contents;
  if (!folly::readFile(file.str().c_str(), contents)) {
    LOG(ERROR) << "Failed to open pool-stats-config-file " << file.str();
    return nullptr;
  }
  return folly::parseJson(contents);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
