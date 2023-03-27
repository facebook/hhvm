/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "options.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <folly/Conv.h>
#include <folly/String.h>

#include "mcrouter/Proxy.h"
#include "mcrouter/lib/fbi/cpp/LogFailure.h"
#include "mcrouter/lib/fbi/cpp/util.h"

using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace folly {

namespace {

template <class T>
struct IsVector : public std::false_type {};
template <class T>
struct IsVector<vector<T>> : public std::true_type {};

} // anonymous namespace

template <class Tgt>
typename std::enable_if<IsVector<Tgt>::value, Tgt>::type to(const string& str) {
  Tgt res;
  vector<string> parts;
  folly::split(',', str, parts, /* ignoreEmpty= */ true);
  for (const auto& it : parts) {
    res.push_back(folly::to<typename Tgt::value_type>(it));
  }
  return res;
}

template <class Tgt, class Src>
typename std::enable_if<IsVector<Src>::value && IsSomeString<Tgt>::value, Tgt>::
    type
    to(const Src& value) {
  return folly::join(",", value);
}

template <class Tgt>
typename std::enable_if<std::is_same<string, Tgt>::value, Tgt>::type to(
    const facebook::memcache::mcrouter::RoutingPrefix& prefix) {
  return prefix;
}

template <class Tgt>
typename std::enable_if<
    std::is_same<facebook::memcache::mcrouter::RoutingPrefix, Tgt>::value,
    facebook::memcache::mcrouter::RoutingPrefix>::type
to(const string& value) {
  return facebook::memcache::mcrouter::RoutingPrefix(value);
}

template <class Tgt>
typename std::enable_if<std::is_same<string, Tgt>::value, Tgt>::type to(
    const unordered_map<string, string>& m) {
  vector<string> result;
  for (const auto& it : m) {
    result.push_back(folly::to<string>(it.first, ":", it.second));
  }
  return folly::join(",", result);
}

template <class Tgt>
typename std::enable_if<
    std::is_same<unordered_map<string, string>, Tgt>::value,
    unordered_map<string, string>>::type
to(const string& s) {
  vector<folly::StringPiece> pairs;
  folly::split(',', s, pairs);
  unordered_map<string, string> result;
  for (const auto& it : pairs) {
    string key;
    string value;
    facebook::memcache::checkLogic(
        folly::split(':', it, key, value),
        "Invalid string map pair: '{}'. Expected name:value.",
        it);
    result.emplace(std::move(key), std::move(value));
  }
  return result;
}

} // namespace folly

namespace facebook {
namespace memcache {

namespace {

const char* const kTempCpuCores = "%CPU_CORES%";

template <class T>
bool tryToString(const boost::any& value, string& res) {
  if (boost::any_cast<T*>(&value) != nullptr) {
    res = folly::to<string>(*boost::any_cast<T*>(value));
    return true;
  }
  return false;
}

string toString(const boost::any& value) {
  string res;
  bool ok = tryToString<int64_t>(value, res) || tryToString<int>(value, res) ||
      tryToString<uint32_t>(value, res) || tryToString<size_t>(value, res) ||
      tryToString<uint16_t>(value, res) ||
      tryToString<unsigned int>(value, res) ||
      tryToString<double>(value, res) || tryToString<bool>(value, res) ||
      tryToString<vector<uint16_t>>(value, res) ||
      tryToString<vector<int16_t>>(value, res) ||
      tryToString<string>(value, res) ||
      tryToString<vector<string>>(value, res) ||
      tryToString<mcrouter::RoutingPrefix>(value, res) ||
      tryToString<unordered_map<string, string>>(value, res);
  checkLogic(ok, "Unsupported option type: {}", value.type().name());
  return res;
}

template <class T>
bool tryFromString(const string& str, const boost::any& value) {
  auto ptr = boost::any_cast<T*>(&value);
  if (ptr != nullptr) {
    **ptr = folly::to<T>(str);
    return true;
  }
  return false;
}

void fromString(const string& str, const boost::any& value) {
  bool ok = tryFromString<int64_t>(str, value) ||
      tryFromString<int>(str, value) || tryFromString<uint32_t>(str, value) ||
      tryFromString<uint16_t>(str, value) ||
      tryFromString<size_t>(str, value) ||
      tryFromString<unsigned int>(str, value) ||
      tryFromString<double>(str, value) || tryFromString<bool>(str, value) ||
      tryFromString<string>(str, value) ||
      tryFromString<vector<uint16_t>>(str, value) ||
      tryFromString<vector<string>>(str, value) ||
      tryFromString<mcrouter::RoutingPrefix>(str, value) ||
      tryFromString<unordered_map<string, string>>(str, value);

  checkLogic(ok, "Unsupported option type: {}", value.type().name());
}

string optionTypeToString(McrouterOptionData::Type type) {
  switch (type) {
    case McrouterOptionData::Type::integer:
    case McrouterOptionData::Type::toggle:
      return "integer";
    case McrouterOptionData::Type::double_precision:
      return "double";
    case McrouterOptionData::Type::string:
      return "string";
    case McrouterOptionData::Type::routing_prefix:
      return "routing prefix";
    case McrouterOptionData::Type::string_map:
      return "string map";
    default:
      return "unknown";
  }
}

} // anonymous namespace

unordered_map<string, string> McrouterOptionsBase::toDict() const {
  unordered_map<string, string> ret;

  forEach([&ret](
              const string& name,
              McrouterOptionData::Type,
              const boost::any& value) { ret[name] = toString(value); });

  return ret;
}

vector<McrouterOptionError> McrouterOptionsBase::updateFromDict(
    const unordered_map<string, string>& new_opts) {
  vector<McrouterOptionError> errors;
  unordered_set<string> seen;

  forEach([&errors, &seen, &new_opts](
              const string& name,
              McrouterOptionData::Type type,
              const boost::any& value) {
    auto it = new_opts.find(name);
    if (it != new_opts.end()) {
      auto subValue = options::substituteTemplates(it->second);
      try {
        fromString(subValue, value);
      } catch (const std::exception& ex) {
        McrouterOptionError e;
        e.requestedName = name;
        e.requestedValue = subValue;
        e.errorMsg = "couldn't convert value to " + optionTypeToString(type) +
            ". Exception: " + ex.what();
        errors.push_back(std::move(e));
      } catch (...) {
        McrouterOptionError e;
        e.requestedName = name;
        e.requestedValue = subValue;
        e.errorMsg = "couldn't convert value to " + optionTypeToString(type);
        errors.push_back(std::move(e));
      }
      seen.insert(it->first);
    }
  });

  /* Don't throw on invalid options, the most likely cause is old code
     during new option roll out */
  for (const auto& kv : new_opts) {
    if (seen.find(kv.first) == seen.end()) {
      LOG_FAILURE(
          "mcrouter",
          failure::Category::kInvalidOption,
          "Unknown option name: {}={}",
          kv.first,
          kv.second);
    }
  }

  return errors;
}

namespace options {

string substituteTemplates(string str) {
  if (str.find(kTempCpuCores) != string::npos) {
    auto c = std::thread::hardware_concurrency();
    if (c == 0) {
      LOG_FAILURE(
          "mcrouter",
          failure::Category::kSystemError,
          "Can not get number of CPU cores. Using 1 instead.");
      c = 1;
    }
    str = replaceAll(std::move(str), kTempCpuCores, folly::to<string>(c));
  }
  return mcrouter::performOptionSubstitution(std::move(str));
}

} // namespace options
} // namespace memcache
} // namespace facebook
