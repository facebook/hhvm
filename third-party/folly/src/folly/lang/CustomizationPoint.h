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

#include <type_traits>

#include <folly/Portability.h>
#include <folly/lang/StaticConst.h>

//  FOLLY_DEFINE_CPO
//
//  Helper for portably defining customization-point objects (CPOs).
//
//  The customization-point object must be placed in a nested namespace to avoid
//  potential conflicts with customizations defined as friend-functions of types
//  defined in the same namespace as the CPO.
//
//  In C++17 and later the object may be defined using 'inline constexpr' to
//  avoid ODR issues. However, prior to that a helper template is required to
//  ensure that there is only a single instance of the CPO created and then a
//  named reference in an anonymous namespace is required to avoid duplicate
//  symbol definitions.
#if FOLLY_HAS_INLINE_VARIABLES
#define FOLLY_DEFINE_CPO(Type, Name) \
  namespace folly_cpo__ {            \
  inline constexpr Type Name{};      \
  }                                  \
  using namespace folly_cpo__;
#else
#define FOLLY_DEFINE_CPO(Type, Name)                        \
  namespace {                                               \
  constexpr auto& Name = ::folly::StaticConst<Type>::value; \
  }
#endif

namespace folly {

// Using 'auto' for non-type template parameters is only possible from C++17
#if FOLLY_CPLUSPLUS >= 201703L

//  cpo_t<CPO>
//
//  Helper type-trait for obtaining the type of customisation point object.
//
//  This can be useful for defining overloads of tag_invoke() where CPOs
//  are defined in terms of ADL calls to tag_invoke().
//
//  For example:
//   FOLLY_DEFINE_CPO(DoSomething_fn, doSomething)
//
//   class SomeClass {
//     ...
//     friend void tag_invoke(folly::cpo_t<doSomething>, const SomeClass& x);
//   };
//
//  See <folly/functional/Invoke.h> for more details.
template <const auto& Tag>
using cpo_t = std::decay_t<decltype(Tag)>;

#endif // FOLLY_CPLUSPLUS >= 201703L

} // namespace folly
