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

#include <cassert>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace apache::thrift::compiler::detail {

/**
 * A simple reimplementation of Thrift's PluggableFunction using only
 * dependencies allowed in the compiler.
 *
 * Tag argument must have a static member function `default_impl` that is called
 * when the function is not set.
 *
 * Not thread-safe.
 *
 * Example:
 *   struct MyTag {
 *     static void default_impl(int) {}
 *   };
 *   pluggable_functions().set<MyTag>(+[](int){});
 *   pluggable_functions().call<MyTag>(42);
 */
class pluggable_function_set {
  template <typename Tag>
  using signature = decltype(Tag::default_impl);

 public:
  template <typename Tag>
  void set(signature<Tag>* fn) {
    std::type_index id = typeid(Tag);
    assert(!funcs.count(id));
    funcs[id] = reinterpret_cast<erased_fn>(fn);
  }

  template <typename Tag, typename... Args>
  std::invoke_result_t<signature<Tag>, Args...> call(Args&&... args) {
    return get<Tag>()(std::forward<Args>(args)...);
  }

 private:
  using erased_fn = void (*)(void);

  std::unordered_map<std::type_index, erased_fn> funcs;

  template <typename Tag>
  signature<Tag>* get() {
    std::type_index id = typeid(Tag);
    if (auto fn = funcs.find(id); fn != funcs.end()) {
      return reinterpret_cast<signature<Tag>*>(fn->second);
    }
    return Tag::default_impl;
  }
};

pluggable_function_set& pluggable_functions();

} // namespace apache::thrift::compiler::detail
