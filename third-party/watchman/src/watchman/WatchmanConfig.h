/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Portability.h>

#include "watchman/thirdparty/jansson/jansson.h"

class w_string;

void cfg_shutdown();
void cfg_load_global_config_file();
w_string cfg_get_global_config_file_path();
std::optional<json_ref> cfg_get_json(const char* name);
const char* cfg_get_string(const char* name, const char* defval);
json_int_t cfg_get_int(const char* name, json_int_t defval);
bool cfg_get_bool(const char* name, bool defval);
double cfg_get_double(const char* name, double defval);
#ifndef _WIN32
mode_t cfg_get_perms(const char* name, bool write_bits, bool execute_bits);
#endif
const char* cfg_get_trouble_url();
std::optional<json_ref> cfg_compute_root_files(bool* enforcing);

// Convert root files to comma delimited string for error message
std::string cfg_pretty_print_root_files(const json_ref& root_files);

namespace watchman {

// Folly signal handling will be limited to Linux for now. We eventually want
// to move all platforms to folly signal handling.
constexpr bool kUseFollySignalHandler = folly::kIsLinux;

class Configuration {
 public:
  Configuration();
  explicit Configuration(std::optional<json_ref> local);

  std::optional<json_ref> get(const char* name) const;
  const char* getString(const char* name, const char* defval) const;
  json_int_t getInt(const char* name, json_int_t defval) const;
  bool getBool(const char* name, bool defval) const;
  double getDouble(const char* name, double defval) const;

 private:
  std::optional<json_ref> local_;
};

} // namespace watchman
