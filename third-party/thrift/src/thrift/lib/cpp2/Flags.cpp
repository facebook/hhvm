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

#include <thrift/lib/cpp2/Flags.h>

#include <map>

#include <folly/MapUtil.h>

namespace apache {
namespace thrift {
namespace detail {
namespace {

class FlagsBackendDummy : public apache::thrift::detail::FlagsBackend {
 public:
  folly::observer::Observer<std::optional<bool>> getFlagObserverBool(
      std::string_view name) override {
    static const folly::Indestructible<std::map<std::string, bool>>
        oss_defaults = std::map<std::string, bool>{
            {"server_rocket_upgrade_enabled", false},
            {"server_header_reject_framed", false},
            {"server_header_reject_unframed", false},
            {"server_header_reject_all", false}};
    return folly::observer::makeObserver(
        [name = std::string(name)]() -> std::optional<bool> {
          return folly::get_optional<std::optional>(*oss_defaults, name);
        });
  }

  folly::observer::Observer<std::optional<int64_t>> getFlagObserverInt64(
      std::string_view) override {
    return folly::observer::makeObserver(
        []() -> std::optional<int64_t> { return std::nullopt; });
  }

  folly::observer::Observer<std::optional<std::string>> getFlagObserverString(
      std::string_view) override {
    return folly::observer::makeObserver(
        []() -> std::optional<std::string> { return std::nullopt; });
  }
};
} // namespace

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<FlagsBackend>, createFlagsBackend) {
  return {};
}

apache::thrift::detail::FlagsBackend& getFlagsBackend() {
  static auto& obj = *[] {
    auto backend = createFlagsBackend();
    if (!backend) {
      backend = std::make_unique<FlagsBackendDummy>();
    }
    return backend.release();
  }();
  return obj;
}
} // namespace detail
} // namespace thrift
} // namespace apache
