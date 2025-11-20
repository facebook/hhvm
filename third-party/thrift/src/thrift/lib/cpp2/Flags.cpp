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
#include <folly/Synchronized.h>

#include <folly/MapUtil.h>

namespace {
using namespace apache::thrift;
using namespace apache::thrift::detail;

class FlagsRegistry {
 public:
  void registerInt64Flag(
      const std::string_view& flagName, FlagWrapper<int64_t>* flag) {
    int64Flags_.wlock()->emplace(flagName, flag);
  }
  void registerBoolFlag(
      const std::string_view& flagName, FlagWrapper<bool>* flag) {
    boolFlags_.wlock()->emplace(flagName, flag);
  }
  void registerStringFlag(
      const std::string_view& flagName, FlagWrapper<std::string>* flag) {
    stringFlags_.wlock()->emplace(flagName, flag);
  }

  std::vector<ThriftFlagInfo> getAllThriftFlags() {
    std::vector<ThriftFlagInfo> flags;
    int64Flags_.withRLock([&](auto& locked) {
      for (auto& [name, flagWrapper] : locked) {
        flags.push_back(
            {std::string{name}, std::to_string(flagWrapper->get())});
      }
    });

    boolFlags_.withRLock([&](auto& locked) {
      for (auto& [name, value] : locked) {
        flags.push_back({std::string{name}, value->get() ? "true" : "false"});
      }
    });

    stringFlags_.withRLock([&](auto& locked) {
      for (auto& [name, value] : locked) {
        flags.push_back({std::string{name}, value->get()});
      }
    });

    return flags;
  }

 private:
  folly::Synchronized<std::map<std::string_view, FlagWrapper<int64_t>*>>
      int64Flags_;
  folly::Synchronized<std::map<std::string_view, FlagWrapper<bool>*>>
      boolFlags_;
  folly::Synchronized<std::map<std::string_view, FlagWrapper<std::string>*>>
      stringFlags_;
};

FlagsRegistry* getFlagsRegistry() {
  static folly::Indestructible<FlagsRegistry> registry;
  return registry.get();
}
} // namespace

namespace apache::thrift {
namespace detail {
namespace {

class FlagsBackendDummy : public apache::thrift::detail::FlagsBackend {
 public:
  folly::observer::Observer<std::optional<bool>> getFlagObserverBool(
      std::string_view name) override {
    static const folly::Indestructible<std::map<std::string, bool>>
        oss_defaults = std::map<std::string, bool>{
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

template <>
void registerFlagWrapper<int64_t>(
    std::string_view name, FlagWrapper<int64_t>* wrapper) {
  getFlagsRegistry()->registerInt64Flag(name, wrapper);
}

template <>
void registerFlagWrapper<bool>(
    std::string_view name, FlagWrapper<bool>* wrapper) {
  getFlagsRegistry()->registerBoolFlag(name, wrapper);
}

template <>
void registerFlagWrapper<std::string>(
    std::string_view name, FlagWrapper<std::string>* wrapper) {
  getFlagsRegistry()->registerStringFlag(name, wrapper);
}
} // namespace detail

std::vector<ThriftFlagInfo> getAllThriftFlags() {
  return getFlagsRegistry()->getAllThriftFlags();
}

} // namespace apache::thrift
