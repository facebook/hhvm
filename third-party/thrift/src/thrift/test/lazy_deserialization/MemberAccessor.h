/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace apache::thrift::test {

template <template <auto> class Tag, auto kMemberPtr, class U>
constexpr auto&& member_accessor(Tag<kMemberPtr>, U&& u) {
  return static_cast<U&&>(u).*kMemberPtr;
}

#define FBTHRIFT_DEFINE_MEMBER_ACCESSOR(FUNC, CLASS, MEMBER)               \
  constexpr auto&& FUNC(const CLASS&);                                     \
  template <auto>                                                          \
  struct FBTHRIFT_MEMBER_ACCESSOR_TAG;                                     \
  template <>                                                              \
  struct FBTHRIFT_MEMBER_ACCESSOR_TAG<&CLASS::__fbthrift_field_##MEMBER> { \
    friend constexpr auto&& FUNC(const CLASS& obj) {                       \
      return ::apache::thrift::test::member_accessor(                      \
          FBTHRIFT_MEMBER_ACCESSOR_TAG{}, obj);                            \
    }                                                                      \
  }

} // namespace apache::thrift::test
