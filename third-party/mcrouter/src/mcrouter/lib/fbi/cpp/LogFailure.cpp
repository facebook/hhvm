/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LogFailure.h"

#include <unistd.h>

#include <chrono>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <folly/Format.h>
#include <folly/Singleton.h>
#include <folly/Synchronized.h>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {
namespace failure {

namespace {

std::string createMessage(
    folly::StringPiece file,
    int line,
    folly::StringPiece service,
    folly::StringPiece category,
    folly::StringPiece msg,
    const std::map<std::string, std::string>& contexts) {
  auto nowUs = std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
  auto result = folly::sformat(
      "FAILURE {}.{} {} [{}] [{}] [{}] {}:{}] {}\n",
      nowUs / 1000000,
      nowUs % 1000000,
      getpid(),
      service,
      category,
      getThreadName(),
      file,
      line,
      msg);

  auto contextIt = contexts.find(service.str());
  if (contextIt != contexts.end()) {
    result += folly::sformat("\"{}\": {}", contextIt->first, contextIt->second);
  } else {
    for (const auto& it : contexts) {
      result += folly::sformat("\"{}\": {}\n", it.first, it.second);
    }
  }
  return result;
}

void vlogErrorImpl(
    folly::StringPiece file,
    int line,
    folly::StringPiece service,
    folly::StringPiece category,
    folly::StringPiece msg,
    const std::map<std::string, std::string>& contexts) {
  if (VLOG_IS_ON(1)) {
    google::LogMessage(
        __FILE__, google::LogMessage::kNoLogPrefix, google::GLOG_INFO)
            .stream()
        << createMessage(file, line, service, category, msg, contexts);
  }
}

void logToStdErrorImpl(
    folly::StringPiece file,
    int line,
    folly::StringPiece service,
    folly::StringPiece category,
    folly::StringPiece msg,
    const std::map<std::string, std::string>& contexts) {
  google::LogMessage(
      __FILE__, google::LogMessage::kNoLogPrefix, google::GLOG_ERROR)
          .stream()
      << createMessage(file, line, service, category, msg, contexts);
}

template <class Error>
void throwErrorImpl(
    folly::StringPiece file,
    int line,
    folly::StringPiece service,
    folly::StringPiece category,
    folly::StringPiece msg,
    const std::map<std::string, std::string>& contexts) {
  throw Error(createMessage(file, line, service, category, msg, contexts));
}

struct StaticContainer {
  // service name => contex
  std::map<std::string, std::string> contexts;

  // { handler name, handler func }
  std::vector<std::pair<std::string, HandlerFunc>> handlers = {
      handlers::verboseLogToStdError()};
};

folly::Singleton<folly::Synchronized<StaticContainer>> containerSingleton;

} // anonymous namespace

namespace handlers {

std::pair<std::string, HandlerFunc> verboseLogToStdError() {
  return std::make_pair<std::string, HandlerFunc>(
      "logToStdError", &vlogErrorImpl);
}

std::pair<std::string, HandlerFunc> logToStdError() {
  return std::make_pair<std::string, HandlerFunc>(
      "logToStdError", &logToStdErrorImpl);
}

std::pair<std::string, HandlerFunc> throwLogicError() {
  return std::make_pair<std::string, HandlerFunc>(
      "throwLogicError", &throwErrorImpl<std::logic_error>);
}

} // namespace handlers

const char* const Category::kBadEnvironment = "bad-environment";
const char* const Category::kInvalidOption = "invalid-option";
const char* const Category::kInvalidConfig = "invalid-config";
const char* const Category::kInvalidDynamicSampler = "invalid-dynamic-sampler";
const char* const Category::kOutOfResources = "out-of-resources";
const char* const Category::kBrokenLogic = "broken-logic";
const char* const Category::kSystemError = "system-error";
const char* const Category::kOther = "other";

bool addHandler(std::pair<std::string, HandlerFunc> handler) {
  if (auto container = containerSingleton.try_get()) {
    return container->withWLock([&](auto& c) {
      for (const auto& it : c.handlers) {
        if (it.first == handler.first) {
          return false;
        }
      }
      c.handlers.push_back(std::move(handler));
      return true;
    });
  }
  return false;
}

bool setHandler(std::pair<std::string, HandlerFunc> handler) {
  if (auto container = containerSingleton.try_get()) {
    return container->withWLock([&](auto& c) {
      for (auto& it : c.handlers) {
        if (it.first == handler.first) {
          it.second = std::move(handler.second);
          return true;
        }
      }
      c.handlers.push_back(std::move(handler));
      return true;
    });
  }
  return false;
}

bool removeHandler(folly::StringPiece handlerName) {
  if (auto container = containerSingleton.try_get()) {
    return container->withWLock([&](auto& c) {
      auto& handlers = c.handlers;
      for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        if (it->first == handlerName) {
          handlers.erase(it);
          return true;
        }
      }
      return false;
    });
  }
  return false;
}

void setServiceContext(folly::StringPiece service, std::string context) {
  if (auto container = containerSingleton.try_get()) {
    container->withWLock(
        [&](auto& c) { c.contexts[service.str()] = std::move(context); });
  }
}

namespace detail {

void log(
    folly::StringPiece file,
    int line,
    folly::StringPiece service,
    folly::StringPiece category,
    folly::StringPiece msg) {
  std::map<std::string, std::string> contexts;
  std::vector<std::pair<std::string, HandlerFunc>> handlers;
  if (auto container = containerSingleton.try_get()) {
    container->withRLock([&](const auto& c) {
      contexts = c.contexts;
      handlers = c.handlers;
    });
  }
  for (auto& handler : handlers) {
    handler.second(file, line, service, category, msg, contexts);
  }
}

} // namespace detail
} // namespace failure
} // namespace memcache
} // namespace facebook
