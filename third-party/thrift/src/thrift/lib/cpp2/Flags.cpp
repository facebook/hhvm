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
  folly::observer::Observer<folly::Optional<bool>> getFlagObserverBool(
      folly::StringPiece name) override {
    static const folly::Indestructible<std::map<std::string, bool>>
        oss_defaults = std::map<std::string, bool>{
            {"server_rocket_upgrade_enabled", false},
            {"server_header_reject_framed", false},
            {"server_header_reject_unframed", false},
            {"server_header_reject_all", false}};
    return folly::observer::makeObserver(
        [name = std::string(name)]() -> folly::Optional<bool> {
          return folly::get_optional(*oss_defaults, name);
        });
  }

  folly::observer::Observer<folly::Optional<int64_t>> getFlagObserverInt64(
      folly::StringPiece) override {
    return folly::observer::makeObserver(
        []() -> folly::Optional<int64_t> { return folly::none; });
  }

  folly::observer::Observer<folly::Optional<std::string>> getFlagObserverString(
      folly::StringPiece) override {
    return folly::observer::makeObserver(
        []() -> folly::Optional<std::string> { return folly::none; });
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
