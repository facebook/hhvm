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
#include <map>
#include <memory>
#include <vector>

namespace w = whisker::make;

namespace whisker {

namespace {

/**
 * A proxying whisker::object that wraps a mstch::array.
 *
 * All array elements are marshaled to whisker::object on first use.
 */
class mstch_array_proxy final
    : public native_object,
      public native_object::array_like,
      public std::enable_shared_from_this<mstch_array_proxy> {
 public:
  explicit mstch_array_proxy(mstch_array&& array)
      : proxied_(std::move(array)) {}

 private:
  native_object::array_like::ptr as_array_like() const override {
    return shared_from_this();
  }

  std::size_t size() const override { return proxied_.size(); }

  object::ptr at(std::size_t index) const override {
    assert(index < proxied_.size());
    // Only allocate the converted vector when the array is used
    if (converted_.size() == 0) {
      converted_.reserve(proxied_.size());
      for (auto& node : proxied_) {
        converted_.emplace_back(from_mstch(std::move(node)));
      }
    }
    return manage_derived_ref(shared_from_this(), converted_[index]);
  }

  void print_to(tree_printer::scope scope, const object_print_options& options)
      const override {
    default_print_to("mstch::array", std::move(scope), options);
  }

  bool operator==(const native_object& untyped_other) const override {
    auto* other = dynamic_cast<const mstch_array_proxy*>(&untyped_other);
    if (other == nullptr) {
      return false;
    }
    const std::size_t sz = size();
    if (sz != other->size()) {
      return false;
    }
    for (std::size_t i = 0; i < sz; ++i) {
      if (*at(i) != *other->at(i)) {
        return false;
      }
    }
    return true;
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
    : public native_object,
      public native_object::map_like,
      public std::enable_shared_from_this<mstch_map_proxy> {
 public:
  explicit mstch_map_proxy(mstch_map&& map) : proxied_(std::move(map)) {}

 private:
  native_object::map_like::ptr as_map_like() const override {
    return shared_from_this();
  }

  object::ptr lookup_property(std::string_view id) const override {
    if (auto cached = converted_.find(id); cached != converted_.end()) {
      return manage_derived_ref(shared_from_this(), cached->second);
    }
    // mstch does not support heterogenous lookups, so we need a temporary
    // std::string.
    std::string id_string{id};
    if (auto property = proxied_.find(id_string); property != proxied_.end()) {
      auto [result, inserted] = converted_.insert(
          {std::move(id_string), from_mstch(std::move(property->second))});
      assert(inserted);
      return manage_derived_ref(shared_from_this(), result->second);
    }
    return nullptr;
  }

  std::optional<std::vector<std::string>> keys() const override {
    std::vector<std::string> property_names;
    property_names.reserve(proxied_.size());
    for (const auto& [key, _] : proxied_) {
      property_names.push_back(key);
    }
    return property_names;
  }

  void print_to(tree_printer::scope scope, const object_print_options& options)
      const override {
    default_print_to("mstch::map", *keys(), std::move(scope), options);
  }

  bool operator==(const native_object& untyped_other) const override {
    auto* other = dynamic_cast<const mstch_map_proxy*>(&untyped_other);
    if (other == nullptr) {
      return false;
    }
    if (proxied_.size() != other->proxied_.size()) {
      return false;
    }
    for (const auto& [key, _] : proxied_) {
      auto this_value = lookup_property(key);
      auto other_value = other->lookup_property(key);
      if (*this_value != *other_value) {
        return false;
      }
    }
    return true;
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
    : public native_object,
      public native_object::map_like,
      public std::enable_shared_from_this<mstch_object_proxy> {
 public:
  explicit mstch_object_proxy(std::shared_ptr<mstch_object>&& obj)
      : proxied_(std::move(obj)) {}

  native_object::map_like::ptr as_map_like() const override {
    return shared_from_this();
  }

  object::ptr lookup_property(std::string_view id) const override {
    // mstch does not support heterogenous lookups, so we need a temporary
    // std::string.
    std::string id_string{id};
    if (!proxied_->has(id_string)) {
      return nullptr;
    }

    object::ptr converted =
        manage_owned<object>(from_mstch(proxied_->at(id_string)));
    return manage_derived(shared_from_this(), std::move(converted));
  }

  std::optional<std::vector<std::string>> keys() const override {
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

  bool operator==(const native_object& untyped_other) const override {
    auto* other = dynamic_cast<const mstch_object_proxy*>(&untyped_other);
    // mstch::object has identity equality
    return other->proxied_ == proxied_;
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
        return w::make_native_object<mstch_object_proxy>(std::move(mstch_obj));
      },
      [](mstch_map&& map) -> object {
        return w::make_native_object<mstch_map_proxy>(std::move(map));
      },
      [](mstch_array&& array) -> object {
        return w::make_native_object<mstch_array_proxy>(std::move(array));
      });
}

bool is_mstch_object(const object& o) {
  return o.is_native_object() &&
      dynamic_cast<mstch_object_proxy*>(o.as_native_object().get()) != nullptr;
}

bool is_mstch_map(const object& o) {
  return o.is_native_object() &&
      dynamic_cast<mstch_map_proxy*>(o.as_native_object().get()) != nullptr;
}

bool is_mstch_array(const object& o) {
  return o.is_native_object() &&
      dynamic_cast<mstch_array_proxy*>(o.as_native_object().get()) != nullptr;
}

} // namespace whisker
