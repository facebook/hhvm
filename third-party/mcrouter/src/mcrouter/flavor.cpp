/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "flavor.h"

#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <glog/logging.h>

#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/json/json.h>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

const char kStandaloneSuffix[] = "-standalone";

std::string getRouterNameFromFlavor(const std::string& flavor) {
  try {
    boost::filesystem::path path(flavor);
    return path.replace_extension().filename().string();
  } catch (...) {
    return flavor;
  }
}

/**
 * Gets the standalone flavor filename from the specified flavor. The
 * standalone flavor filename has the following formart:
 * [flavor_file_without_extension]-standalone[flavor_file_extension].
 */
std::string getStandaloneFlavor(const std::string& flavor) {
  try {
    boost::filesystem::path path(flavor);
    auto extension = path.extension();
    path.replace_extension();

    return path.string() + kStandaloneSuffix + extension.string();
  } catch (...) {
    return flavor;
  }
}

/**
 * Reads a libmcrouter flavor file and fills the given dictionary.
 *
 * @param flavor_json Json of the libmcrouter flavor.
 * @param opts        Output parameter the options in the flavor file.
 *
 * @returns           True if the config was successfully loaded.
 *                    False otherwise.
 */
bool readLibmcrouterFlavor(
    const std::string& flavor_json,
    std::unordered_map<std::string, std::string>& opts) {
  try {
    auto json = parseJsonString(folly::json::stripComments(flavor_json));
    return parse_json_options(json, "options", opts);
  } catch (...) {
    return false;
  }
}

/**
 * Fills the given dictionaries with the standalone json file.
 *
 * @param standalone_flavor_json  Json of the standalone flavor file.
 * @param standalone_opts         Output parameter for standalone_options.
 * @param libmcrouter_opts        Output parameter for libmcrouter_options.
 *
 * @returns                       True if the config was successfully loaded.
 *                                False otherwise.
 */
bool readStandaloneFlavor(
    const std::string& standalone_flavor_json,
    std::unordered_map<std::string, std::string>& standalone_opts,
    std::unordered_map<std::string, std::string>& libmcrouter_opts) {
  try {
    auto json =
        parseJsonString(folly::json::stripComments(standalone_flavor_json));

    if (!parse_json_options(json, "standalone_options", standalone_opts)) {
      return false;
    }

    const char kLibmcrouterOptions[] = "libmcrouter_options";
    if (json.count(kLibmcrouterOptions) > 0) {
      return parse_json_options(json, kLibmcrouterOptions, libmcrouter_opts);
    }

    return true;
  } catch (...) {
    return false;
  }
}

} // anonymous namespace

bool readFlavor(
    const std::string& flavor,
    std::unordered_map<std::string, std::string>& standalone_opts,
    std::unordered_map<std::string, std::string>& libmcrouter_opts) {
  // Reads libmcrouter flavor.
  std::string libmcrouterFlavorJson;
  if (folly::readFile(flavor.data(), libmcrouterFlavorJson)) {
    if (!readLibmcrouterFlavor(libmcrouterFlavorJson, libmcrouter_opts)) {
      return false;
    }
  }

  // Reads standalone mcrouter flavor.
  std::string standaloneFlavor = getStandaloneFlavor(flavor);
  std::string standaloneFlavorJson;
  if (folly::readFile(standaloneFlavor.data(), standaloneFlavorJson)) {
    if (!readStandaloneFlavor(
            standaloneFlavorJson, standalone_opts, libmcrouter_opts)) {
      return false;
    }
  }

  libmcrouter_opts["router_name"] = getRouterNameFromFlavor(flavor);
  try {
    libmcrouter_opts["flavor_name"] =
        boost::filesystem::absolute(flavor).string();
  } catch (const boost::filesystem::filesystem_error& e) {
    LOG(ERROR) << "Error getting the absolute path of flavor. Exception: "
               << e.what();
    return false;
  }

  return true;
}

bool parse_json_options(
    const folly::dynamic& json,
    const std::string& field_name,
    std::unordered_map<std::string, std::string>& opts) {
  auto it = json.find(field_name);
  if (it == json.items().end()) {
    return false;
  }

  const auto& jopts = it->second;
  if (!jopts.isObject()) {
    LOG(ERROR) << "Error parsing flavor config: " << field_name
               << " is not an object";
    return false;
  }

  try {
    for (auto& jiter : jopts.items()) {
      opts[jiter.first.asString()] = jiter.second.asString();
    }
  } catch (...) {
    return false;
  }

  return true;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
