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

#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/mstch_compat.h>

#include <cassert>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include <fmt/core.h>

namespace apache::thrift::mstch {
node::node(std::size_t i) : base(static_cast<int>(i)) {
  if (i > static_cast<unsigned int>(std::numeric_limits<int>::max())) {
    throw std::overflow_error(
        fmt::format("size_t greater than int max: {}", i));
  }
}
} // namespace apache::thrift::mstch

namespace w = whisker::make;

namespace whisker {

namespace {

/**
 * A proxying whisker::object that wraps a mstch::array.
 *
 * All array elements are marshaled to whisker::object on first use.
 */
class mstch_array_proxy final : public array {
 public:
  explicit mstch_array_proxy(mstch_array&& array)
      : proxied_(std::move(array)) {}

 private:
  std::size_t size() const override { return proxied_.size(); }

  object at(std::size_t index) const override {
    assert(index < proxied_.size());
    // Only allocate the converted vector when the array is used
    if (converted_.size() == 0) {
      converted_.reserve(proxied_.size());
      for (auto& node : proxied_) {
        converted_.emplace_back(from_mstch(std::move(node)));
      }
    }
    return converted_[index];
  }

  void print_to(tree_printer::scope scope, const object_print_options& options)
      const override {
    default_print_to("mstch::array", std::move(scope), options);
  }

  mutable mstch_array proxied_;
  mutable std::vector<object> converted_;
};

/**
 * A proxying whisker::object that wraps a mstch::map.
 *
 * Properties are lazily marshaled to whisker::object on first access by name.
 */
class mstch_map_proxy final
    : public map,
      public std::enable_shared_from_this<mstch_map_proxy> {
 public:
  explicit mstch_map_proxy(mstch_map&& map) : proxied_(std::move(map)) {}

 private:
  std::optional<object> lookup_property(std::string_view id) const override {
    if (auto cached = converted_.find(id); cached != converted_.end()) {
      return cached->second;
    }
    if (auto property = proxied_.find(id); property != proxied_.end()) {
      auto [result, inserted] = converted_.insert(
          {std::string(id), from_mstch(std::move(property->second))});
      assert(inserted);
      return result->second;
    }
    return std::nullopt;
  }

  std::optional<std::set<std::string>> keys() const override {
    std::set<std::string> property_names;
    for (const auto& [key, _] : proxied_) {
      property_names.insert(key);
    }
    return property_names;
  }

  void print_to(tree_printer::scope scope, const object_print_options& options)
      const override {
    default_print_to("mstch::map", *keys(), std::move(scope), options);
  }

  mutable mstch_map proxied_;
  mutable std::map<std::string, object, std::less<>> converted_;
};

/**
 * A proxy whisker::object that wraps a mstch::object.
 *
 * Property lookups are NOT cached as the underlying property on the
 * mstch::object may be volatile.
 */
class mstch_object_proxy
    : public map,
      public std::enable_shared_from_this<mstch_object_proxy> {
 public:
  explicit mstch_object_proxy(std::shared_ptr<mstch_object>&& obj)
      : proxied_(std::move(obj)) {}

  std::optional<object> lookup_property(std::string_view id) const override {
    if (!proxied_->has(id)) {
      return std::nullopt;
    }

    return detail::variant_match(
        proxied_->at(id),
        [&](const mstch_node& node) -> object {
          object::ptr converted = manage_owned<object>(from_mstch(node));
          return manage_derived(shared_from_this(), std::move(converted));
        },
        [](const object& o) -> object { return o; });
  }

  std::optional<std::set<std::string>> keys() const override {
    return proxied_->property_names();
  }

  void print_to(
      tree_printer::scope scope,
      [[maybe_unused]] const object_print_options& options) const override {
    assert(scope.semantic_depth() <= options.max_depth);
    scope.println("mstch::object");

    for (const auto& key : proxied_->property_names()) {
      auto element_scope = scope.open_transparent_property();
      element_scope.println("'{}'", key);
      // It's not safe to access the mstch::object properties since they can
      // have side-effects. So we can only report property names.
      element_scope.open_node().println("...");
    }
  }

 private:
  std::shared_ptr<mstch_object> proxied_;
};

} // namespace

object from_mstch(mstch_node node) {
  return detail::variant_match(
      static_cast<mstch_node::base&&>(node),
      [](std::nullptr_t) { return w::null; },
      [](std::string&& str) { return w::string(std::move(str)); },
      [](int value) { return w::i64(value); },
      [](double value) { return w::f64(value); },
      [](bool value) { return w::boolean(value); },
      [](std::shared_ptr<mstch_object>&& mstch_obj) -> object {
        return w::make_map<mstch_object_proxy>(std::move(mstch_obj));
      },
      [](mstch_map&& map) -> object {
        return w::make_map<mstch_map_proxy>(std::move(map));
      },
      [](mstch_array&& array) -> object {
        return w::make_array<mstch_array_proxy>(std::move(array));
      });
}

} // namespace whisker
