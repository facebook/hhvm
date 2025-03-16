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

#include <thrift/compiler/whisker/dsl.h>

#include <thrift/compiler/whisker/detail/overload.h>

#include <set>

#include <fmt/ranges.h>

namespace whisker::dsl {

std::size_t array_like::size() const {
  return whisker::detail::variant_match(
      which_, [](const auto& arr) -> std::size_t { return arr->size(); });
}

object array_like::at(std::size_t index) const {
  return whisker::detail::variant_match(
      which_,
      [&](const native_object::array_like::ptr& arr) -> object {
        return arr->at(index);
      },
      [&](const managed_array& arr) -> object { return (*arr)[index]; });
}

/* static */ std::optional<array_like> array_like::try_from(const object& o) {
  if (o.is_array()) {
    return array_like(o.as_array());
  }
  if (o.is_native_object()) {
    if (native_object::array_like::ptr arr =
            o.as_native_object()->as_array_like()) {
      return array_like(std::move(arr));
    }
  }
  return std::nullopt;
}

std::optional<object> map_like::lookup_property(
    std::string_view identifier) const {
  using result = std::optional<object>;
  return whisker::detail::variant_match(
      which_,
      [&](const native_object::map_like::ptr& m) -> result {
        return m->lookup_property(identifier);
      },
      [&](const managed_map& m) -> result {
        if (auto it = m->find(identifier); it != m->end()) {
          return it->second;
        }
        return std::nullopt;
      });
}

std::optional<std::set<std::string>> map_like::keys() const {
  using result = std::optional<std::set<std::string>>;
  return whisker::detail::variant_match(
      which_,
      [&](const native_object::map_like::ptr& m) -> result {
        return m->keys();
      },
      [&](const managed_map& m) -> result {
        std::set<std::string> keys;
        for (const auto& [key, _] : *m) {
          keys.insert(key);
        }
        return keys;
      });
}

/* static */ std::optional<map_like> map_like::try_from(const object& o) {
  if (o.is_map()) {
    return map_like(o.as_map());
  }
  if (o.is_native_object()) {
    if (native_object::map_like::ptr m = o.as_native_object()->as_map_like()) {
      return map_like(std::move(m));
    }
  }
  return std::nullopt;
}

std::optional<object> function::context::named_argument(
    std::string_view name, named_argument_presence presence) const {
  auto arg = raw().named_arguments().find(name);
  if (arg == raw().named_arguments().end()) {
    if (presence == named_argument_presence::optional) {
      return std::nullopt;
    }
    throw make_error("Missing named argument '{}'.", name);
  }
  return arg->second;
}

void function::context::do_warning(std::string msg) const {
  raw().diagnostics().warning(raw().location().begin, "{}", std::move(msg));
}

void function::context::declare_arity(std::size_t expected) const {
  if (arity() != expected) {
    throw make_error("Expected {} argument(s) but got {}", expected, arity());
  }
}

void function::context::declare_named_arguments(
    std::initializer_list<std::string_view> expected) const {
  // Using std::set so that the error message is deterministic.
  std::set<std::string_view> names;
  for (const auto& [name, _] : raw().named_arguments()) {
    names.insert(name);
  }
  for (const auto& name : expected) {
    names.erase(name);
  }
  if (!names.empty()) {
    throw make_error(
        "Unknown named argument(s) provided: {}.", fmt::join(names, ", "));
  }
}

object function::invoke(raw_context raw) {
  return this->invoke(context{std::move(raw)});
}

} // namespace whisker::dsl
