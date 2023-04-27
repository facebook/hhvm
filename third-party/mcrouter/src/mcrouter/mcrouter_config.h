/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#ifndef HAVE_CONFIG_H
static_assert(false, "mcrouter: invalid build");
#endif

/**
 * This header contains features specific for open source
 */
#include <time.h>

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include <folly/Range.h>
#include <folly/experimental/observer/Observer.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/carbon/NoopAdditionalLogger.h"
#include "mcrouter/lib/carbon/NoopExternalConnectionLogger.h"
#include "mcrouter/lib/network/Transport.h"

#define MCROUTER_RUNTIME_VARS_DEFAULT ""
#define MCROUTER_STATS_ROOT_DEFAULT "/var/mcrouter/stats"
#define DEBUG_FIFO_ROOT_DEFAULT "/var/mcrouter/fifos"
#define CONFIG_DUMP_ROOT_DEFAULT "/var/mcrouter/config"
#define MCROUTER_DEFAULT_CA_PATH ""

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

class McrouterOptions;
struct MemcacheRouterInfo;

namespace mcrouter {

class CarbonRouterInstanceBase;
class ConfigApi;
class McrouterLogger;
struct FailoverContext;
class ProxyBase;
struct RequestLoggerContext;
struct TkoLog;

struct ProxyStatsContainer {
  explicit ProxyStatsContainer(ProxyBase&) {}
};

class AdditionalProxyRequestLogger : public carbon::NoopAdditionalLogger {
 public:
  explicit AdditionalProxyRequestLogger(
      const ProxyRequestContext& proxyRequestContext)
      : NoopAdditionalLogger(proxyRequestContext) {}
};

class AdditionalExternalConnectionLogger
    : public carbon::NoopExternalConnectionAdditionalLogger {
 public:
  explicit AdditionalExternalConnectionLogger(
      carbon::ExternalCarbonConnectionLoggerOptions& options)
      : NoopExternalConnectionAdditionalLogger(options) {}
};

inline bool alwaysSendToMainShardSplit(uint64_t /* flags */) {
  return false;
}

/**
 * @return monotonic time suitable for measuring intervals in microseconds.
 */
inline int64_t nowUs() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

/**
 * @return monotonic time suitable for measuring intervals in seconds.
 */
inline double nowSec() {
  return nowUs() / 1000000.0;
}

/**
 * getCurrentTimeInMs - returns current time in milliseconds since epoch
 */
inline uint64_t getCurrentTimeInMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

/**
 * @return wall clock time since epoch in seconds.
 */
inline time_t nowWallSec() {
  return time(nullptr);
}

bool readLibmcrouterFlavor(
    folly::StringPiece flavor,
    std::unordered_map<std::string, std::string>& options);

bool read_standalone_flavor(
    const std::string& flavor,
    std::unordered_map<std::string, std::string>& option_dict,
    std::unordered_map<std::string, std::string>& st_option_dict);

std::unique_ptr<ConfigApi> createConfigApi(const McrouterOptions& opts);

std::string performOptionSubstitution(std::string str);

std::unique_ptr<McrouterLogger> createMcrouterLogger(
    CarbonRouterInstanceBase& router);

/**
 * @throws logic_error on invalid options
 */
void extraValidateOptions(const McrouterOptions& opts);

void applyTestMode(McrouterOptions& opts);

McrouterOptions defaultTestOptions();

std::vector<std::string> defaultTestCommandLineArgs();

void logTkoEvent(ProxyBase& proxy, const TkoLog& tkoLog);

void logFailover(ProxyBase& proxy, const FailoverContext& failoverContext);

void initFailureLogger();

/**
 * Initializes compression dictionaries for the given mcrouter instance
 */
bool initCompression(CarbonRouterInstanceBase& router);

void scheduleSingletonCleanup();

std::unordered_map<std::string, folly::dynamic> additionalConfigParams();

inline bool isMetagetAvailable() {
  return false;
}

void insertCustomStartupOpts(folly::dynamic& options);

std::string getBinPath(folly::StringPiece name);

void finalizeOptions(McrouterOptions& options);

/**
 * Reads a static json file. Do not monitor for changes.
 * May throw if there's an error while parsing file contents.
 *
 * @params file   The path of the json file.
 *
 * @return        folly::dynamic with the contents of the file.
 *                nullptr if cannot open/read the file
 *                may throw exception if invalid json
 *
 */
folly::dynamic readStaticJsonFile(folly::StringPiece file);

#ifndef MCROUTER_PACKAGE_STRING
#define MCROUTER_PACKAGE_STRING "1.0.0 mcrouter"
#endif

inline folly::Optional<folly::observer::Observer<std::string>>
startObservingRuntimeVarsFileCustom(
    folly::StringPiece file,
    std::function<void(std::string)> onUpdate) {
  return folly::none;
}

inline bool isInLocalDatacenter(const std::string& /* host */) {
  return false;
}

inline Transport::SvcIdentAuthCallbackFunc getAuthChecker(
    const McrouterOptions& opts) {
  return nullptr;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
