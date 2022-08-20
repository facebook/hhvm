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

#include <thrift/conformance/data/Constants.h>

#include <fmt/core.h>

namespace apache::thrift::conformance::data {
namespace {
std::string genUri(const char* package, const char* name) {
  return fmt::format("{}/{}", package, name);
}
} // namespace

std::vector<std::string> genGoodDefUris() {
  std::vector<std::string> result;
  for (const auto& package : kGoodPackageNames) {
    for (const auto& name : kGoodDefNames) {
      result.emplace_back(genUri(package, name));
    }
  }
  return result;
}

std::vector<std::string> genBadDefUris() {
  std::vector<std::string> result;
  for (const auto& package : kGoodPackageNames) {
    for (const auto& name : kBadDefNames) {
      result.emplace_back(genUri(package, name));
    }
  }
  for (const auto& package : kBadPackageNames) {
    for (const auto& name : kGoodDefNames) {
      result.emplace_back(genUri(package, name));
    }
  }
  for (const auto& package : kBadPackageNames) {
    for (const auto& name : kBadDefNames) {
      result.emplace_back(genUri(package, name));
    }
  }
  return result;
}

} // namespace apache::thrift::conformance::data
