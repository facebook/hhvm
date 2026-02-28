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

#include <folly/Benchmark.h>
#include <folly/Random.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/DeserializationBench_types.h>

folly::Random::DefaultGenerator rng_(12345);
const int32_t kNumOfInserts = 250;

std::vector<int32_t> getRandomVector() {
  std::vector<int32_t> v;
  for (int i = 0; i < kNumOfInserts; ++i) {
    v.push_back(i);
  }
  return v;
}

std::set<int32_t> getRandomSet() {
  std::set<int32_t> s;
  for (int i = 0; i < kNumOfInserts; ++i) {
    s.insert(i);
  }
  return s;
}

std::map<int32_t, std::string> getRandomMap() {
  std::map<int32_t, std::string> m;
  for (int i = 0; i < kNumOfInserts; ++i) {
    m.emplace(i, std::to_string(i));
  }
  return m;
}

folly::sorted_vector_set<int32_t> getRandomFollySet() {
  folly::sorted_vector_set<int32_t> s;
  for (int i = 0; i < kNumOfInserts; ++i) {
    s.insert(i);
  }
  return s;
}

folly::sorted_vector_map<int32_t, std::string> getRandomFollyMap() {
  folly::sorted_vector_map<int32_t, std::string> m;
  for (int i = 0; i < kNumOfInserts; ++i) {
    m[i] = std::to_string(i);
  }
  return m;
}

void buildRandomStructA(cpp2::StructA& obj) {
  *obj.fieldA() = folly::Random::rand32(rng_) % 2;
  *obj.fieldB() = folly::Random::rand32(rng_);
  *obj.fieldC() = std::to_string(folly::Random::rand32(rng_));
  *obj.fieldD() = getRandomVector();
  *obj.fieldE() = getRandomSet();
  *obj.fieldF() = getRandomMap();

  for (int32_t i = 0; i < kNumOfInserts; ++i) {
    std::vector<std::vector<int32_t>> g1;
    std::set<std::set<int32_t>> h1;
    std::vector<std::set<int32_t>> j1;
    std::set<std::vector<int32_t>> j2;
    for (int32_t j = 0; j < kNumOfInserts; ++j) {
      g1.push_back(getRandomVector());
      h1.insert(getRandomSet());
      j1.push_back(getRandomSet());
      j2.insert(getRandomVector());
    }
    obj.fieldG()->push_back(g1);
    obj.fieldH()->insert(h1);
    obj.fieldI()->emplace(getRandomMap(), getRandomMap());
    obj.fieldJ()->emplace(j1, j2);
  }
}

void buildRandomStructB(cpp2::StructB& obj) {
  *obj.fieldA() = folly::Random::rand32(rng_) % 2;
  *obj.fieldB() = folly::Random::rand32(rng_);
  *obj.fieldC() = std::to_string(folly::Random::rand32(rng_));
  *obj.fieldD() = getRandomVector();
  *obj.fieldE() = getRandomFollySet();
  *obj.fieldF() = getRandomFollyMap();

  for (int32_t i = 0; i < kNumOfInserts; ++i) {
    std::vector<std::vector<int32_t>> g1;
    folly::sorted_vector_set<folly::sorted_vector_set<int32_t>> h1;
    std::vector<folly::sorted_vector_set<int32_t>> j1;
    folly::sorted_vector_set<std::vector<int32_t>> j2;
    for (int32_t j = 0; j < kNumOfInserts; ++j) {
      g1.push_back(getRandomVector());
      h1.insert(getRandomFollySet());
      j1.push_back(getRandomFollySet());
      j2.insert(getRandomVector());
    }
    obj.fieldG()->push_back(g1);
    obj.fieldH()->insert(h1);
    obj.fieldI()[getRandomFollyMap()] = getRandomFollyMap();
    obj.fieldJ()[std::move(j1)] = j2;
  }
}

BENCHMARK(CompactSerialization_custom_container, iters) {
  using serializer = apache::thrift::CompactSerializer;

  folly::BenchmarkSuspender braces; // stop the clock by default

  for (size_t i = 0; i < iters; ++i) {
    // Prep, untimed:
    cpp2::StructB obj;
    buildRandomStructB(obj);

    // Serialize, timed:
    braces.dismissing([&] { serializer::serialize<folly::IOBufQueue>(obj); });
  }
}

BENCHMARK(CompactDeserialization_custom_container, iters) {
  using serializer = apache::thrift::CompactSerializer;

  folly::BenchmarkSuspender braces; // stop the clock by default

  for (size_t i = 0; i < iters; ++i) {
    // Prep, untimed:
    cpp2::StructB obj;
    buildRandomStructB(obj);

    // Serialize, untimed:
    auto buf = serializer::serialize<folly::IOBufQueue>(obj).move();
    buf->coalesce(); // so we can ignore serialization artifacts later

    // Deserialize, timed:
    cpp2::StructB obj2;
    braces.dismissing([&] { serializer::deserialize(buf.get(), obj2); });
  }
}

BENCHMARK(CompactSerialization, iters) {
  using serializer = apache::thrift::CompactSerializer;

  folly::BenchmarkSuspender braces; // stop the clock by default

  for (size_t i = 0; i < iters; ++i) {
    // Prep, untimed:
    cpp2::StructA obj;
    buildRandomStructA(obj);

    // Serialize, timed:
    braces.dismissing([&] { serializer::serialize<folly::IOBufQueue>(obj); });
  }
}

BENCHMARK(CompactDeserialization, iters) {
  using serializer = apache::thrift::CompactSerializer;

  folly::BenchmarkSuspender braces; // stop the clock by default

  for (size_t i = 0; i < iters; ++i) {
    // Prep, untimed:
    cpp2::StructA obj;
    buildRandomStructA(obj);

    // Serialize, untimed:
    auto buf = serializer::serialize<folly::IOBufQueue>(obj).move();
    buf->coalesce(); // so we can ignore serialization artifacts later

    // Deserialize, timed:
    cpp2::StructA obj2;
    braces.dismissing([&] { serializer::deserialize(buf.get(), obj2); });
  }
}

BENCHMARK(JsonSerialization, iters) {
  using serializer = apache::thrift::SimpleJSONSerializer;

  folly::BenchmarkSuspender braces; // stop the clock by default

  for (size_t i = 0; i < iters; ++i) {
    // Prep, untimed:
    cpp2::StructA obj;
    buildRandomStructA(obj);

    // Serialize, timed:
    braces.dismissing([&] { serializer::serialize<folly::IOBufQueue>(obj); });
  }
}

BENCHMARK(JsonDeserialization, iters) {
  using serializer = apache::thrift::SimpleJSONSerializer;

  folly::BenchmarkSuspender braces; // stop the clock by default

  for (size_t i = 0; i < iters; ++i) {
    // Prep, untimed:
    cpp2::StructA obj;
    buildRandomStructA(obj);

    // Serialize, untimed:
    auto buf = serializer::serialize<folly::IOBufQueue>(obj).move();
    buf->coalesce(); // so we can ignore serialization artifacts later

    // Deserialize, timed:
    cpp2::StructA obj2;
    braces.dismissing([&] { serializer::deserialize(buf.get(), obj2); });
  }
}

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}

/*
============================================================================
thrift/test/DeserializationBench.cpp            relative  time/iter  iters/s
============================================================================
CompactSerialization_custom_container                         2.76s  362.93m
CompactDeserialization_custom_container                    728.59ms     1.37
CompactSerialization                                          2.71s  368.92m
CompactDeserialization                                     760.46ms     1.31
JsonSerialization                                             6.02s  166.06m
JsonDeserialization                                          11.17s   89.49m
============================================================================
*/
