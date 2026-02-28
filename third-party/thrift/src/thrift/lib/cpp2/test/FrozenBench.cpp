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

#include <random>

#include <folly/Benchmark.h>
#include <folly/Conv.h>
#include <folly/hash/Hash.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/FrozenTypes_types.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using std::string;
using std::vector;

Team testValue() {
  Team team;
  auto hasher = std::hash<int64_t>();
  team.peopleById() = {};
  team.peopleByName() = {};
  for (int i = 1; i <= 500; ++i) {
    auto id = hasher(i);
    Person p;
    *p.id() = id;
    p.nums().ensure();
    p.nums()->insert(i);
    p.nums()->insert(-i);
    folly::toAppend("Person ", i, &(*p.name()));
    (*team.peopleById())[*p.id()] = p;
    auto& peopleByNameEntry = (*team.peopleByName())[*p.name()];
    peopleByNameEntry = std::move(p);
  }
  team.projects() = {};
  team.projects()->insert("alpha");
  team.projects()->insert("beta");
  return team;
}

auto team = testValue();
auto frozen = freeze(team);

BENCHMARK(Freeze, iters) {
  vector<byte> buffer;
  while (iters--) {
    size_t bytes = frozenSize(team);
    if (buffer.size() < bytes) {
      buffer.resize(bytes);
    }
    byte* p = buffer.data();
    auto frozen = freeze(team, p);
    (void)frozen;
  }
}

BENCHMARK_RELATIVE(FreezePreallocated, iters) {
  vector<byte> buffer(102400);
  while (iters--) {
    byte* p = buffer.data();
    auto frozen = freeze(team, p);
    (void)frozen;
    assert(p <= &buffer.back());
  }
}

template <class Serializer>
void benchmarkSerializer(int iters) {
  while (iters--) {
    string serialized;
    Serializer::serialize(team, &serialized);
  }
}

BENCHMARK_RELATIVE(SerializerCompact, iters) {
  benchmarkSerializer<apache::thrift::CompactSerializer>(iters);
}

BENCHMARK_RELATIVE(SerializerBinary, iters) {
  benchmarkSerializer<apache::thrift::BinarySerializer>(iters);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Thaw, iters) {
  Team thawed;
  while (iters--) {
    thaw(*frozen, thawed);
  }
}

template <class Serializer>
void benchmarkDeserializer(int iters) {
  string serialized;
  Serializer::serialize(team, &serialized);

  while (iters--) {
    Team obj;
    Serializer::deserialize(serialized, obj);
  }
}

BENCHMARK_RELATIVE(DeserializerCompact, iters) {
  benchmarkDeserializer<apache::thrift::CompactSerializer>(iters);
}

BENCHMARK_RELATIVE(DeserializerBinary, iters) {
  benchmarkDeserializer<apache::thrift::BinarySerializer>(iters);
}

BENCHMARK_DRAW_LINE();

const int nEntries = 10000000;
auto entries = [] {
  std::mt19937 gen;
  std::vector<std::pair<int, int>> entries(nEntries);
  for (size_t i = 0; i < nEntries; ++i) {
    entries[i].first = i;
  }
  std::shuffle(entries.begin(), entries.end(), gen);
  for (int i = 0; i < nEntries; ++i) {
    entries[i].second = i;
  }
  return entries;
}();
auto shuffled = [] {
  std::mt19937 gen;
  auto shuffled = entries;
  std::shuffle(shuffled.begin(), shuffled.end(), gen);
  return shuffled;
}();
auto hmap = [] {
  std::unordered_map<int, int> ret;
  for (auto& e : entries) {
    if (e.second % 2 == 0) {
      CHECK(ret.insert(e).second) << e.second;
    }
  }
  return ret;
}();
std::map<int, int> map(hmap.begin(), hmap.end());
auto fmap = freeze(map);
auto fhmap = freeze(hmap);

BENCHMARK(thawedMap, iters) {
  while (iters--) {
    for (auto& e : shuffled) {
      auto found = map.find(e.first);
      if (found != map.end()) {
        CHECK_EQ(found->second, e.second);
      } else {
        CHECK(e.second % 2) << e.second;
      }
    }
  }
}

BENCHMARK_RELATIVE(frozenMap, iters) {
  while (iters--) {
    for (auto& e : shuffled) {
      auto found = fmap->find(e.first);
      if (found != fmap->end()) {
        CHECK_EQ(found->second, e.second) << e.first;
      } else {
        CHECK(e.second % 2) << e.second;
      }
    }
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(thawedHashMap, iters) {
  while (iters--) {
    for (auto& e : shuffled) {
      auto found = hmap.find(e.first);
      if (found != hmap.end()) {
        CHECK_EQ(found->second, e.second);
      } else {
        CHECK(e.second % 2) << e.second;
      }
    }
  }
}

BENCHMARK_RELATIVE(frozenHashMap, iters) {
  while (iters--) {
    for (auto& e : shuffled) {
      auto found = fhmap->find(e.first);
      if (found != fhmap->end()) {
        CHECK_EQ(found->second, e.second) << e.first;
      } else {
        CHECK(e.second % 2) << e.second;
      }
    }
  }
}

BENCHMARK_DRAW_LINE();

auto bigMap = [] {
  std::map<std::string, std::pair<std::string, std::string>> bigMap;
  const int kSalt = 0x619abc7e;
  const int nKeys = 500000;
  for (int i = 0; i < nKeys; ++i) {
    auto h = folly::hash::jenkins_rev_mix32(i ^ kSalt);
    auto key = folly::to<std::string>(h);
    bigMap[key] = std::make_pair(h * h, h * h * h);
  }
  return bigMap;
}();
std::map<std::string, std::pair<std::string, std::string>> bigHashMap(
    bigMap.begin(), bigMap.end());

BENCHMARK(FreezeBigMap, iters) {
  int k = 0;
  while (iters--) {
    auto f = freeze(bigMap);
    k ^= f->size();
  }
  folly::doNotOptimizeAway(k);
}

BENCHMARK_RELATIVE(FreezeBigHashMap, iters) {
  int k = 0;
  while (iters--) {
    auto f = freeze(bigHashMap);
    k ^= f->size();
  }
  folly::doNotOptimizeAway(k);
}

/*
============================================================================
thrift/lib/cpp/test/FrozenBench.cpp             relative  time/iter  iters/s
============================================================================
Freeze                                                     357.35us    2.80K
FreezePreallocated                               174.97%   204.23us    4.90K
SerializerCompact                                228.34%   156.50us    6.39K
SerializerBinary                                 365.94%    97.65us   10.24K
----------------------------------------------------------------------------
Thaw                                                       620.40us    1.61K
DeserializerCompact                               62.20%   997.36us    1.00K
DeserializerBinary                                73.01%   849.74us    1.18K
----------------------------------------------------------------------------
thawedMap                                                    11.50s   86.98m
frozenMap                                        257.34%      4.47s  223.83m
----------------------------------------------------------------------------
thawedHashMap                                                 1.00s  998.40m
frozenHashMap                                    141.39%   708.39ms     1.41
----------------------------------------------------------------------------
FreezeBigMap                                               224.39ms     4.46
FreezeBigHashMap                                 227.49%    98.63ms    10.14
============================================================================
 */
int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();

  return 0;
}
