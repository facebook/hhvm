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

#include <folly/Overload.h>
#include <folly/String.h>
#include <thrift/lib/cpp2/async/InterceptorFlags.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorControl.h>

namespace apache::thrift {

namespace {

// Represents a flag value that is a fully qualifiedName, e.g
// MyModule.MyInterceptor
struct DisabledQualifiedName {
  std::string_view moduleName;
  std::string_view interceptorName;
};

struct DisabledModuleName {
  std::string_view moduleName;
};

using DisabledName = std::variant<DisabledQualifiedName, DisabledModuleName>;

std::vector<DisabledName> parseDisabledInterceptorsFlag(
    const std::string& disabledInterceptorFlagValue) {
  std::vector<std::string_view> disabledNames;
  folly::split(',', disabledInterceptorFlagValue, disabledNames);

  std::vector<DisabledName> output;
  output.reserve(disabledNames.size());
  for (const auto& disabledName : disabledNames) {
    std::string_view moduleName, interceptorName;
    if (folly::split</*exact*/ true>(
            '.', disabledName, moduleName, interceptorName)) {
      CHECK(!moduleName.empty() && !interceptorName.empty())
          << "Thrift flag disabled_service_interceptors is malformed - empty moduleName and/or interceptorName. Value: "
          << disabledInterceptorFlagValue;
      output.emplace_back(
          DisabledQualifiedName{
              .moduleName = moduleName, .interceptorName = interceptorName});
    } else {
      // Basically if there is no '.', we assume it's a module name
      CHECK(!disabledName.empty())
          << "Thrift flag disabled_service_interceptors is malformed - empty moduleName. Value: "
          << disabledInterceptorFlagValue;
      output.emplace_back(DisabledModuleName{.moduleName = disabledName});
    }
  }
  return output;
}

const folly::observer::Observer<std::vector<DisabledName>>&
getDisabledNamesObserver() {
  static folly::observer::Observer<std::vector<DisabledName>>
      disabledNamesObserver = folly::observer::makeObserver(
          [flagObserver = THRIFT_FLAG_OBSERVE(disabled_service_interceptors)] {
            auto flagValue = **flagObserver;
            if (flagValue.empty()) {
              return std::vector<DisabledName>();
            }
            return parseDisabledInterceptorsFlag(**flagObserver);
          });
  return disabledNamesObserver;
}

bool interceptorIsDisabled(
    const std::vector<DisabledName>& disabledNames,
    const ServiceInterceptorQualifiedName& name) {
  for (const auto& disabledName : disabledNames) {
    if (folly::variant_match(
            disabledName,
            [&](const DisabledQualifiedName& disabledQualifiedName) {
              return disabledQualifiedName.moduleName == name.getModuleName() &&
                  disabledQualifiedName.interceptorName ==
                  name.getInterceptorName();
            },
            [&](const DisabledModuleName& disabledModuleName) {
              return disabledModuleName.moduleName == name.getModuleName();
            })) {
      return true;
    }
  }
  return false;
}

} // namespace

ServiceInterceptorControl::ServiceInterceptorControl(
    const ServiceInterceptorQualifiedName& name)
    : observer_{folly::observer::makeAtomicObserver(
          [&name, disabledNamesObserver = getDisabledNamesObserver()]() {
            std::vector<DisabledName> disabledNames = **disabledNamesObserver;
            return interceptorIsDisabled(disabledNames, name);
          })} {}

bool ServiceInterceptorControl::isDisabled() const {
  return THRIFT_FLAG(disable_all_service_interceptors) || *observer_;
}

} // namespace apache::thrift
