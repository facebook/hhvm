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

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

#include <thrift/lib/cpp2/server/PreprocessParams.h>
#include <thrift/lib/cpp2/server/PreprocessResult.h>

namespace apache::thrift {

class PreprocessFunctionSet {
 public:
  using Func =
      folly::Function<PreprocessResult(const server::PreprocessParams&) const>;

  void add(const std::string& name, Func&& function) {
    if (std::find_if(functions_.begin(), functions_.end(), [&](const auto& f) {
          return f.first == name;
        }) != functions_.end()) {
      throw std::invalid_argument(fmt::format(
          "PreprocessFunction with name {} already exists in this Thrift Server",
          name));
    }

    functions_.emplace_back(name, std::move(function));
  }

  void deprecatedSet(Func&& function) {
    constexpr auto kName = "UnnamedUserDefinedPreprocessFunction";

    if (deprecatedSetCalled) {
      functions_.erase(functions_.begin());
    }
    deprecatedSetCalled = true;
    functions_.insert(functions_.begin(), {kName, std::move(function)});
  }

  void clear() { functions_.clear(); }

  size_t size() { return functions_.size(); }

  std::vector<std::string> getFunctionsNames() {
    std::vector<std::string> names;
    for (const auto& [name, _] : functions_) {
      names.push_back(name);
    }
    return names;
  }

  PreprocessResult run(const server::PreprocessParams& params) const {
    for (auto& [_, function] : functions_) {
      auto result = function(params);
      if (!std::holds_alternative<std::monostate>(result)) {
        return result;
      }
    }
    return {};
  }

 private:
  std::vector<std::pair<std::string, Func>> functions_;
  bool deprecatedSetCalled{false};
};

using PreprocessFunc = PreprocessFunctionSet::Func;

} // namespace apache::thrift
