/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <unordered_map>

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook::memcache::mcrouter {

/**
 * Reads mcrouter flavor file and fills the given dictionaries.
 * Note: The user must specify the flavor without any modifications/suffixes
 * and this method will locate and load the libmcrouter flavor and, if the
 * standalone flavor (flavor + "-standalone") file is present, the options under
 * the "libmcrouter_options" in this file will take precedence over the options
 * specified if the libmcrouter flavor file.
 *
 * @param flavor            Path of the flavor file without any suffixes.
 * @param standalone_opts   Output parameter for standalone options.
 * @param libmcrouter_opts  Output parameter for libmcrouter options.
 *
 * @returns                 True if successfully loaded. False otherwise (i.e.
 *                          any error is found)
 */
bool readFlavor(
    const std::string& flavor,
    std::unordered_map<std::string, std::string>& standalone_opts,
    std::unordered_map<std::string, std::string>& libmcrouter_opts);

/**
 * Parse the field especified by "field_name" in the json file and sets it's
 * contents in the "opts" dictionary.
 *
 * @param json        The json file contents.
 * @param field_name  The name of the field in the json file to parse.
 * @param opts        Output parameter that will hold the contents of the field.
 *
 * @returns           True if successfully parsed. False otherwise.
 */
bool parse_json_options(
    const folly::dynamic& json,
    const std::string& field_name,
    std::unordered_map<std::string, std::string>& opts);
} // namespace facebook::memcache::mcrouter
