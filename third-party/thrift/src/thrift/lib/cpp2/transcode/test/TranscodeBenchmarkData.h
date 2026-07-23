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

#include <cstdint>
#include <string>
#include <utility>

namespace apache::thrift::transcode::bench_data {

namespace fixture = apache::thrift::transcode::test;

inline fixture::GoldenInner inner(int32_t n, std::string label) {
  fixture::GoldenInner value;
  value.n() = n;
  value.label() = std::move(label);
  return value;
}

inline fixture::GoldenChoice nameChoice(std::string name) {
  fixture::GoldenChoice value;
  value.name() = std::move(name);
  return value;
}

inline fixture::GoldenStruct goldenStruct() {
  fixture::GoldenStruct value;
  value.flag() = true;
  value.b() = static_cast<int8_t>(-7);
  value.s() = -1234;
  value.i() = 1234567;
  value.l() = -9876543210LL;
  value.f() = 1.25f;
  value.d() = -3.5;
  value.text() = "hello\njson";
  value.data() = std::string("\x00hi\xff", 4);
  value.ints() = {1, -2, 3};
  value.tags() = {"blue", "green"};
  value.counts() = {{"one", 1}, {"minus", -1}};
  value.inner() = inner(42, "nested");
  value.maybe_text() = "present";
  value.status() = fixture::GoldenEnum::Active;
  value.choice() = nameChoice("chosen");
  return value;
}

} // namespace apache::thrift::transcode::bench_data
