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

#include <thrift/compiler/whisker/object.h>

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// The source code in the following namespace was once based on the original
// code by Daniel Sipka (https://github.com/no1msd/mstch).
//
// However, a "ship of Theseus" situation has happened over the past decade (or
// more) such that nothing is left from the original code.
namespace apache::thrift::mstch {

namespace internal {

template <class N>
class object_t {
 public:
  using node_ref = std::reference_wrapper<const N>;
  using lookup_result = std::variant<whisker::object, node_ref>;

  lookup_result at(std::string_view name) const {
    assert(has(name));
    const auto& [_, do_lookup] = *methods_.find(name);
    return do_lookup();
  }

  bool has(std::string_view name) const {
    return methods_.find(name) != methods_.end();
  }

  std::set<std::string> property_names() const {
    std::set<std::string> result;
    for (const auto& [name, _] : methods_) {
      result.insert(name);
    }
    return result;
  }

 protected:
  // Uncached (formerly known as volatile) methods are re-invoked every time
  // their value is needed during a template evaluation.
  //
  // This is potentially useful if mutating state during evaluation, but has a
  // performance cost. There are usually better ways to express such logic.
  struct with_no_caching_t {};
  static constexpr with_no_caching_t with_no_caching{};

  using property_dispatcher = std::function<lookup_result()>;

  template <typename Self>
  struct property_descriptor {
    using binder = std::function<property_dispatcher(Self*)>;
    binder bind;

    /* implicit */ property_descriptor(whisker::i64 (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(whisker::f64 (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(whisker::boolean (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(whisker::string (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(whisker::array::ptr (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(whisker::map::ptr (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(whisker::object (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(
        whisker::native_function::ptr (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(
        whisker::native_handle<> (Self::*method)())
        : bind(cached_whisker_object(method)) {}
    /* implicit */ property_descriptor(whisker::object::ptr (Self::*method)())
        : bind(cached_whisker_object(method)) {}

    /* implicit */ property_descriptor(N (Self::*method)())
        : bind([method](Self* self) -> property_dispatcher {
            return [self,
                    method = std::move(method),
                    cache = std::optional<N>()]() mutable -> lookup_result {
              if (!cache) {
                cache = (self->*method)();
              }
              return node_ref(*cache);
            };
          }) {}

    property_descriptor(with_no_caching_t, N (Self::*method)())
        : bind([method](Self* self) -> property_dispatcher {
            return [self,
                    method = std::move(method),
                    uncache = std::optional<N>()]() mutable -> lookup_result {
              uncache = (self->*method)();
              return node_ref(*uncache);
            };
          }) {}

    template <
        typename F,
        std::enable_if_t<std::is_same_v<std::invoke_result_t<F>, N>>* = nullptr>
    /* implicit */ property_descriptor(F&& method)
        : bind(
              [method = std::function<N()>(std::forward<F>(method))](
                  Self*) -> property_dispatcher {
                return [method = std::move(method),
                        cache = std::optional<N>()]() mutable -> lookup_result {
                  if (!cache) {
                    cache = method();
                  }
                  return node_ref(*cache);
                };
              }) {}

    template <
        typename F,
        std::enable_if_t<std::is_same_v<std::invoke_result_t<F>, N>>* = nullptr>
    property_descriptor(with_no_caching_t, F&& method)
        : bind(
              [method = std::function<N()>(std::forward<F>(method))](
                  Self*) -> property_dispatcher {
                return
                    [method = std::move(method),
                     uncache = std::optional<N>()]() mutable -> lookup_result {
                      uncache = method();
                      return node_ref(*uncache);
                    };
              }) {}

   private:
    template <typename T>
    static binder cached_whisker_object(T (Self::*method)()) {
      return [method](Self* self) -> property_dispatcher {
        return
            [self, method, cache = std::optional<whisker::object>()]() mutable
                -> lookup_result {
              if (!cache) {
                if constexpr (std::is_same_v<T, whisker::object::ptr>) {
                  cache = (self->*method)();
                } else {
                  cache = whisker::object((self->*method)());
                }
              }
              return *cache;
            };
      };
    }
  };

  void register_method(
      std::string name, property_descriptor<std::monostate> method) {
    do_register_method(std::move(name), method.bind(nullptr));
  }

  template <class Self>
  void register_methods(
      Self* self,
      const std::unordered_map<std::string, property_descriptor<Self>>&
          methods) {
    for (const auto& method : methods) {
      do_register_method(method.first, method.second.bind(self));
    }
  }

 private:
  void do_register_method(std::string name, property_dispatcher method) {
    auto result = methods_.emplace(std::move(name), std::move(method));
    if (!result.second) {
      throw std::runtime_error(
          "Method already registered: " + result.first->first);
    }
  }

  // Before C++20, std::unordered_map does not support heterogenous lookups
  std::map<std::string, property_dispatcher, std::less<>> methods_;
};

template <typename Node>
using node_base = std::variant<
    std::nullptr_t,
    std::string,
    int,
    double,
    bool,
    std::shared_ptr<internal::object_t<Node>>,
    std::map<const std::string, Node, std::less<>>,
    std::vector<Node>>;

} // namespace internal

struct node : internal::node_base<node> {
  using base = internal::node_base<node>;

  using base::base;
  /* implicit */ node(const char* s) : base(std::string(s)) {}
  /* implicit */ node(std::string_view sv) : base(std::string(sv)) {}
  /* implicit */ node(std::size_t i);

  // Equivalent to the int constructor brought in by `using base::base`, but
  // having this here forces integer callers which are neither int nor size_t to
  // intentionally convert to one or the other. Without the following line,
  // calls like node(int64_t) would silently disambiguate to the size_t
  // constructor, which is potentially surprising.
  /* implicit */ node(int i) : base(i) {}
};

template <typename Node, typename... A>
node make_shared_node(A&&... a) {
  return node(std::make_shared<Node>(static_cast<A&&>(a)...));
}

using object = internal::object_t<node>;
using map = std::map<const std::string, node, std::less<>>;
using array = std::vector<node>;

} // namespace apache::thrift::mstch

namespace whisker {

/**
 * A mstch::object is analogous to whisker::map.
 */
using mstch_object = apache::thrift::mstch::object;
/**
 * A mstch::map is analogous to whisker::map.
 */
using mstch_map = apache::thrift::mstch::map;
/**
 * A mstch::array is analogous to whisker::array.
 */
using mstch_array = apache::thrift::mstch::array;
/**
 * A mstch::node is analogous to whisker::object.
 */
using mstch_node = apache::thrift::mstch::node;

/**
 * Creates a whisker::object that proxies property and array lookups to the
 * provided mstch::node.
 *
 * For existing mstch::node objects, property lookups will be functionally
 * identical when performed on the returned whisker::object instead.
 *
 * Marshaling of "complex" data types between mstch and Whisker occurs lazily as
 * lookups are performed. The lazily marshaled types are:
 *   - mstch_array (all elements are marshaled on first access)
 *   - mstch_map (every key is marshaled lazily)
 *
 * mstch_object is marshaled lazily by construction since its properties are not
 * finitely enumerable and their values may be volatile (change between
 * invocations).
 *
 * The smaller data types (such as i64, f64, string etc.) are eagerly marshaled.
 * Note that, such data contained within a mstch_array or mstch_map will be
 * lazily marshaled. In other words, the eager marshaling only applies to values
 * in the object tree at depth 0.
 */
object from_mstch(mstch_node, diagnostics_engine&);

} // namespace whisker
