/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "hphp/runtime/ext/watcher/watcher-options.h"

#include <string>
#include <vector>

#include <folly/dynamic.h>

#include "hphp/util/optional.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/type-variant.h"

#include "hphp/runtime/ext/watcher/watcher-clock.h"

namespace HPHP {
namespace Watcher {
namespace {

const auto* kIncludePaths = "include_paths";
const auto* kIncludeExtensions = "include_extensions";
const auto* kExcludePaths = "exclude_paths";
const auto* kExcludeExtensions = "exclude_extensions";
const auto* kRoot = "repo_root";
const auto* kRelativeRoot = "relative_root";
const auto* kSocketPath = "socket_path";
const auto* kFields = "fields";

folly::dynamic make_list(const std::vector<std::string>& values) {
  if (values.size() == 1) {
    return folly::dynamic(values.front());
  } else {
    folly::dynamic list = folly::dynamic::array();
    for (const auto& value : values) {
      list.push_back(value);
    }
    return list;
  }
}

folly::dynamic anyof(
    const std::string& key,
    const std::vector<std::string>& values) {
  if (values.size() == 1) {
    return folly::dynamic::array(key, values.front());
  } else {
    folly::dynamic list = folly::dynamic::array("anyof");
    for (const auto& value : values) {
      list.push_back(folly::dynamic::array(key, value));
    }
    return list;
  }
}

folly::dynamic negate(folly::dynamic expression) {
  return folly::dynamic::array("not", expression);
}

const HPHP::Optional<std::vector<std::string>> vectorOfStringsFromArray(const TypedValue& tv) {
    if (!tvIsArrayLike(tv) || tv.m_data.parr == nullptr) {
      return std::nullopt;
    } 

    std::vector<std::string> values;
    IterateV(tv.m_data.parr, [&](TypedValue v) {
      if (tvIsString(v)) {
        values.push_back(v.m_data.pstr->slice().str());
      }
    });
    return values;
}

const HPHP::Optional<std::vector<std::string>> fieldListFromArray(const TypedValue& tv) {
  std::map<std::string, std::string> watcherFieldMap {
    {"name", "name"}, 
    {"sha1hex", "content.sha1hex"},
  };

  auto requested_fields = vectorOfStringsFromArray(tv);
  if (!requested_fields.has_value()) {
    return std::nullopt;
  }

  std::vector<std::string> fields;
  for (const auto& v : *requested_fields) {
    auto iter = watcherFieldMap.find(v);
    if (iter != watcherFieldMap.end()) {
      fields.push_back(iter->second);
    }
  }
  return fields;
}
  
} // namespace

WatcherOptions::WatcherOptions(const HPHP::Variant& options, const HPHP::Variant& clock) {
  auto* option_data = options.isArray() ? options.getArrayDataOrNull() : nullptr;
  if (option_data == nullptr) {
    return;
  }

  this->clock = WatcherClock::fromVariant(clock);

  IterateKV(option_data, [&](TypedValue k, TypedValue tv) {
    if (!tvIsString(k)) {
      return;
    }

    auto key = k.m_data.pstr->slice();
    if (key == kIncludePaths) {
      include_paths = vectorOfStringsFromArray(tv);
    } else if (key == kIncludeExtensions) {
      include_extensions = vectorOfStringsFromArray(tv);
    } else if (key == kExcludePaths) {
      exclude_paths = vectorOfStringsFromArray(tv);
    } else if (key == kExcludeExtensions) {
      exclude_extensions = vectorOfStringsFromArray(tv);
    } else if (key == kRelativeRoot) {
      if (tvIsString(tv)) {
        relative_root = tv.m_data.pstr->slice();
      }
    } else if (key == kRoot) {
      if (tvIsString(tv)) {
        root = tv.m_data.pstr->slice();
      }
    } else if (key == kSocketPath) {
      if (tvIsString(tv)) {
        socket_path = tv.m_data.pstr->slice();
      }
    } else if (key == kFields) {
      fields = fieldListFromArray(tv);
    }
  });
}
  
folly::dynamic WatcherOptions::watchmanQuery() {
  folly::dynamic expression = folly::dynamic::array("allof");
  expression.push_back(folly::dynamic::array("type", "f"));
  if (include_paths.has_value()) {
    expression.push_back(anyof("dirname", *include_paths));
  }
  if (include_extensions.has_value()) {
    expression.push_back(anyof("suffix", *include_extensions));
  }
  if (exclude_paths.has_value()) {
    expression.push_back(negate(anyof("dirname", *exclude_paths)));
  }
  if (exclude_extensions.has_value()) {
    expression.push_back(negate(anyof("suffix", *exclude_extensions)));
  }

  folly::dynamic query = folly::dynamic::object;
  if (expression.size() > 2) {
    query.insert("expression", expression);
  }
  if (include_extensions.has_value()) {
    query.insert("suffix", make_list(*include_extensions));
  }
  if (relative_root.has_value()) {
    query.insert("relative_root", *relative_root);
  }
  if (fields.has_value()) {
    query.insert("fields", make_list(*fields));
  } else {
    query.insert("fields", folly::dynamic::array("name"));
  }

  switch(clock.clock_type) {
    case WatcherClockType::SINCE:
      query.insert("since", *clock.clock_value);
      break;
    case WatcherClockType::MERGEBASE:
      query.insert("since", 
                    folly::dynamic::object(
                    "scm", 
                    folly::dynamic::object(
                      "mergebase-with", 
                      *clock.clock_value)));
      break;
    case WatcherClockType::NONE:
    default:
      // Do Nothing.
      break;
  }

  return folly::dynamic::array("query", root.has_value() ? *root : ".", query);
}

}  // namespace Watcher
}  // namespace HPHP

