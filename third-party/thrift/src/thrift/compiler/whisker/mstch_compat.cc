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

#include <boost/algorithm/string/predicate.hpp>
#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/eval_context.h>
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
  explicit mstch_array_proxy(mstch_array&& array, diagnostics_engine& diags)
      : proxied_(std::move(array)), diags_(diags) {}

 private:
  std::size_t size() const override { return proxied_.size(); }

  object at(std::size_t index) const override {
    assert(index < proxied_.size());
    // Only allocate the converted vector when the array is used
    if (converted_.size() == 0) {
      converted_.reserve(proxied_.size());
      for (auto& node : proxied_) {
        converted_.emplace_back(from_mstch(std::move(node), diags_));
      }
    }
    return converted_[index];
  }

  void print_to(tree_printer::scope& scope, const object_print_options& options)
      const override {
    default_print_to("mstch::array", scope, options);
  }

  mutable mstch_array proxied_;
  mutable std::vector<object> converted_;
  diagnostics_engine& diags_;
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
  explicit mstch_map_proxy(mstch_map&& map, diagnostics_engine& diags)
      : proxied_(std::move(map)), diags_(diags) {}

 private:
  std::optional<object> lookup_property(std::string_view id) const override {
    if (auto cached = converted_.find(id); cached != converted_.end()) {
      return cached->second;
    }
    if (auto property = proxied_.find(id); property != proxied_.end()) {
      auto [result, inserted] = converted_.insert(
          {std::string(id), from_mstch(std::move(property->second), diags_)});
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

  void print_to(tree_printer::scope& scope, const object_print_options& options)
      const override {
    default_print_to("mstch::map", *keys(), scope, options);
  }

  mutable mstch_map proxied_;
  mutable std::map<std::string, object, std::less<>> converted_;
  diagnostics_engine& diags_;
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
  explicit mstch_object_proxy(
      std::shared_ptr<mstch_object>&& obj, diagnostics_engine& diags)
      : proxied_(std::move(obj)), diags_(diags) {}

  std::optional<object> lookup_property(std::string_view id) const override {
    if (!proxied_->has(id)) {
      return lookup_property_through_self(id);
    }

    return detail::variant_match(
        proxied_->at(id),
        [&](const mstch_node& node) -> object {
          object::ptr converted =
              manage_owned<object>(from_mstch(node, diags_));
          return manage_derived(shared_from_this(), std::move(converted));
        },
        [](const object& o) -> object { return o; });
  }

  std::optional<std::set<std::string>> keys() const override {
    return proxied_->property_names();
  }

  void print_to(
      tree_printer::scope& scope,
      [[maybe_unused]] const object_print_options& options) const override {
    assert(scope.depth() <= options.max_depth);
    scope.print("mstch::object");

    for (const auto& key : proxied_->property_names()) {
      // It's not safe to access the mstch::object properties since they can
      // have side-effects. So we can only report property names.
      scope.make_child("'{}' → ...", key);
    }
  }

 private:
  std::shared_ptr<mstch_object> proxied_;
  diagnostics_engine& diags_;

  // For a lookup of the form "foo:bar", returns a pair of ["foo:self", "bar"]
  // Returns nullopt if property can't be split to a self lookup, or is already
  // a self lookup.
  // Otherwise returns a pair of the self property and the nested property name.
  std::optional<std::pair<std::string, std::string_view>>
  get_self_property_name(const std::string_view& id) const {
    if (boost::algorithm::ends_with(id, ":self")) {
      // Already a self lookup that was tried - don't recurse
      return std::nullopt;
    }

    const size_t delimiter_pos = id.find_last_of(':');
    if (delimiter_pos == std::string::npos) {
      return std::nullopt;
    }

    return std::make_optional(std::make_pair(
        fmt::format("{}:self", id.substr(0, delimiter_pos)),
        id.substr(delimiter_pos + 1)));
  }

  std::optional<object> lookup_property_through_self(
      std::string_view id) const {
    const std::optional<std::pair<std::string, std::string_view>>
        self_property = get_self_property_name(id);
    if (!self_property.has_value() || !proxied_->has(self_property->first)) {
      return std::nullopt;
    }

    const mstch_object::lookup_result self_value =
        proxied_->at(self_property->first);
    if (const object* self_whisker_obj = std::get_if<object>(&self_value)) {
      return detail::find_property(
          diags_,
          *self_whisker_obj,
          ast::identifier{source_range{}, std::string(self_property->second)});
    }

    return std::nullopt;
  }
};

} // namespace

object from_mstch(mstch_node node, diagnostics_engine& diags) {
  return detail::variant_match(
      static_cast<mstch_node::base&&>(node),
      [](std::nullptr_t) { return w::null; },
      [](std::string&& str) { return w::string(std::move(str)); },
      [](int value) { return w::i64(value); },
      [](double value) { return w::f64(value); },
      [](bool value) { return w::boolean(value); },
      [&](std::shared_ptr<mstch_object>&& mstch_obj) -> object {
        return w::make_map<mstch_object_proxy>(std::move(mstch_obj), diags);
      },
      [&](mstch_map&& map) -> object {
        return w::make_map<mstch_map_proxy>(std::move(map), diags);
      },
      [&](mstch_array&& array) -> object {
        return w::make_array<mstch_array_proxy>(std::move(array), diags);
      });
}

} // namespace whisker
