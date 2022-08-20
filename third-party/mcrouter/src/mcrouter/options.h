/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/any.hpp>

#include "mcrouter/RoutingPrefix.h"
#include "mcrouter/config.h"

#define DEFAULT_NUM_PROXIES 1

constexpr size_t kListeningSocketsDefault{1};

/**
 * NOTE: must be kept in sync with kLogLifetime in mcreplay2/EventReader.cpp
 */
#define DEFAULT_ASYNCLOG_LIFETIME (15 * 60) // New log every fifteen minutes

namespace facebook {
namespace memcache {

struct McrouterOptionError {
  std::string requestedName;
  std::string requestedValue;
  std::string errorMsg;
};

struct McrouterOptionData {
  enum class Type {
    integer,
    string,
    double_precision,
    toggle,
    routing_prefix,
    string_map,
    other
  };

  Type type;

  /// Field name in the options struct
  std::string name;

  /// Option group this option belongs to
  std::string group;

  /// Default value as a string
  std::string default_value;

  /// Long option (if empty, option can't be set from the command line)
  std::string long_option;

  /// Short option (if '\0', there's no short option)
  char short_option;

  /// Documentation
  std::string docstring;
};

class McrouterOptionsBase {
 public:
  std::unordered_map<std::string, std::string> toDict() const;

  std::vector<McrouterOptionError> updateFromDict(
      const std::unordered_map<std::string, std::string>& new_opts);

  virtual ~McrouterOptionsBase() {}

  virtual void forEach(std::function<void(
                           const std::string& name,
                           McrouterOptionData::Type type,
                           const boost::any& value)> f) const = 0;
};

#define OPTIONS_FILE "mcrouter/mcrouter_options_list.h"
#define OPTIONS_NAME McrouterOptions
#include "mcrouter/options-template.h"

#undef OPTIONS_FILE
#undef OPTIONS_NAME

namespace options {

/**
 * Perform %..% variable substitution on an individual string
 */
std::string substituteTemplates(std::string str);

} // namespace options
} // namespace memcache
} // namespace facebook
