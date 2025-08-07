/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdexcept>
#include <fmt/format.h>
#include <thrift/compiler/sema/sema_context.h>

namespace apache::thrift::compiler {

sema_params::validation_level sema_params::parse_validation_level(
    const std::string& name) {
  static const std::map<std::string, validation_level> kValidationLevelNames{
      {"none", validation_level::none},
      {"warn", validation_level::warn},
      {"error", validation_level::error}};

  if (auto it = kValidationLevelNames.find(name);
      it != kValidationLevelNames.end()) {
    return it->second;
  }

  throw std::runtime_error(fmt::format("Unknown validation level: '{}'", name));
}

std::string sema_params::validation_level_to_string(validation_level lvl) {
  switch (lvl) {
    case validation_level::none:
      return "none";
    case validation_level::warn:
      return "warn";
    case validation_level::error:
      return "error";
    default:
      return fmt::format(
          "<unknown validation_level: {}>", fmt::underlying(lvl));
  }
}

} // namespace apache::thrift::compiler
