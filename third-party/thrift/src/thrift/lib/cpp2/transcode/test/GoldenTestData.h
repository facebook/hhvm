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

#include <thrift/lib/cpp2/transcode/test/gen-cpp2/GoldenFixtures_types.h>

#include <string>

namespace apache::thrift::transcode::golden_test {

namespace fixture = apache::thrift::transcode::test;

template <typename T>
struct PreEncodedJson {
  std::string bytes;
  T value;
};

template <typename T>
struct Generate;

template <>
struct Generate<fixture::ScalarShapes> {
  fixture::ScalarShapes operator()() const;
};

void expectScalarShapes(
    const fixture::ScalarShapes& output, const fixture::ScalarShapes& input);

template <>
struct Generate<fixture::NonUtf8StringShapes> {
  fixture::NonUtf8StringShapes operator()() const;
};

template <>
struct Generate<PreEncodedJson<fixture::NonUtf8StringShapes>> {
  PreEncodedJson<fixture::NonUtf8StringShapes> operator()() const;
};

template <>
struct Generate<fixture::SpecialFloatShapes> {
  fixture::SpecialFloatShapes operator()() const;
};

void expectSpecialFloatShapes(
    const fixture::SpecialFloatShapes& output,
    const fixture::SpecialFloatShapes& input);

template <>
struct Generate<fixture::NegativeFieldIdShapes> {
  fixture::NegativeFieldIdShapes operator()() const;
};

template <>
struct Generate<fixture::PresenceShapes> {
  fixture::PresenceShapes operator()(bool withOptional) const;
};

template <>
struct Generate<fixture::SequenceShapes> {
  fixture::SequenceShapes operator()() const;
};

template <>
struct Generate<fixture::MapShapes> {
  fixture::MapShapes operator()() const;
};

template <>
struct Generate<fixture::NestedShapes> {
  fixture::NestedShapes operator()() const;
};

template <>
struct Generate<fixture::UnionShapes> {
  fixture::UnionShapes operator()() const;
};

template <>
struct Generate<fixture::ExceptionShapes> {
  fixture::ExceptionShapes operator()() const;
};

template <>
struct Generate<fixture::GoldenStruct> {
  fixture::GoldenStruct operator()(bool withOptional) const;
};

template <typename T>
inline constexpr Generate<T> generate{};

} // namespace apache::thrift::transcode::golden_test
