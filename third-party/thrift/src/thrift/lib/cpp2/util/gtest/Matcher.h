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

#include <ostream>

#include <gmock/gmock.h>
#include <thrift/lib/cpp2/FieldRef.h>

#include <thrift/lib/cpp2/util/gtest/Printer.h>

namespace apache::thrift::test {

namespace detail {

template <typename FieldTag, typename InnerMatcher>
class ThriftFieldMatcher;

template <typename FieldTag, typename InnerMatcher>
class IsThriftUnionWithMatcher;

} // namespace detail

// A Thrift field matcher.
//
// Given a Thrift struct
//
//   struct Person {
//     1: string name;
//     2: optional int id;
//   }
//
// the matcher can be used as follows:
//
//   auto p = Person();
//   p.name() = "Zaphod";
//   p.id() = 42
//   EXPECT_THAT(p, ThriftField<name>(Eq("Zaphod")));
//   EXPECT_THAT(p. ThriftField<id>(Optional(Eq("Zaphod"))))
//
// The `name` and `id` types are declared inside the `apache::thrift::ident`
// namespace in the `_types.h` generated file.
// A useful pattern is to define `namespace field = apache::thrift::ident;`, and
// refer to the fields as `field::field_name`, e.g.
// `ThriftField<field::id>(_)`.
//
// Alternatively, the following also works (but doesn't print the field name on
// failure):
//
//   EXPECT_THAT(p, ThriftField(&Person::name<>, Eq("Zaphod")));
//   EXPECT_THAT(p. ThriftField(&Person::id<>, Optional(Eq("Zaphod"))))
//
template <typename T, typename Struct, typename Matcher>
auto ThriftField(
    apache::thrift::field_ref<const T&> (Struct::*ref)() const&,
    const Matcher& matcher) {
  return testing::ResultOf(
      [=](const Struct& s) { return *(s.*ref)(); }, matcher);
}

template <typename T, typename Struct, typename Matcher>
auto ThriftField(
    apache::thrift::optional_field_ref<const T&> (Struct::*ref)() const&,
    const Matcher& matcher) {
  return testing::ResultOf(
      [=](const Struct& s) { return (s.*ref)(); }, matcher);
}

template <typename FieldTag, typename Matcher>
detail::ThriftFieldMatcher<FieldTag, Matcher> ThriftField(Matcher matcher) {
  return detail::ThriftFieldMatcher<FieldTag, Matcher>(matcher);
}

// A Thrift union(variant) matcher.
//
// Given a Thrift union
//
//   union Result {
//     1: int success;
//     2: string error;
//   }
//
// the matcher can be used as follows:
//
//   auto r = Result();
//   r.set_success(1);
//   EXPECT_THAT(p, IsThriftUnionWith<success>(Eq(1)));
//   EXPECT_THAT(p, Not(IsThriftUnionWith<error>(_)));
//
// The `success` and `error` types are declared inside the
// `apache::thrift::ident` namespace in the `_types.h` generated file. A useful
// pattern is to define `namespace field = apache::thrift::ident;`, and refer to
// the fields as `field::field_name`, e.g.
// `IsThriftUnionWith<field::success>(_)`.
//
template <typename FieldTag, typename Matcher>
detail::IsThriftUnionWithMatcher<FieldTag, Matcher> IsThriftUnionWith(
    Matcher matcher) {
  return detail::IsThriftUnionWithMatcher<FieldTag, Matcher>(matcher);
}

} // namespace apache::thrift::test

#include <thrift/lib/cpp2/util/gtest/Matcher-inl.h>
