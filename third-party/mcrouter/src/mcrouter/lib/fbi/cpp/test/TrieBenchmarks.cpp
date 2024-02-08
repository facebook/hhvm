/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <vector>

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <folly/Benchmark.h>
#include <folly/container/F14Map.h>
#include <folly/init/Init.h>

#include "mcrouter/lib/fbi/cpp/Trie.h"

using facebook::memcache::Trie;

namespace {

template <class Value>
class KeyPrefixMap : public folly::F14NodeMap<std::string, Value> {
  using Base = folly::F14NodeMap<std::string, Value>;

 public:
  using iterator = typename Base::iterator;

  std::pair<iterator, bool> emplace(folly::StringPiece key, Value v) {
    auto it = std::lower_bound(
        prefixLength_.begin(), prefixLength_.end(), key.size());
    if (it == prefixLength_.end() || *it != key.size()) {
      prefixLength_.insert(it, key.size());
    }
    return Base::emplace(key, std::move(v));
  }

  iterator findPrefix(folly::StringPiece key) {
    auto result = end();
    for (auto len : prefixLength_) {
      if (len > key.size()) {
        return result;
      }
      auto it = find(key.subpiece(0, len));
      if (it != end()) {
        result = it;
      }
    }
    return result;
  }

  using Base::begin;
  using Base::end;
  using Base::find;

 private:
  std::vector<size_t> prefixLength_;
};

std::vector<std::string> keysToGet[3];

Trie<int> randTrie[3];
KeyPrefixMap<int> randMap[3];
int x = 0;

void prepareRand() {
  std::vector<std::string> keys[3] = {
      {
          "abacaba",
          "abacabadabacaba",
          "b123",
          "qwerty:qwerty:qwerty:123456",
      },
      {
          "AMC", "ayk", "brq", "bxj", "fgn", "fkr", "fm0", "gig", "gtg",
          "gtm", "iag", "kkb", "kki", "kkx", "kkz", "kqf", "kqg", "mbf",
          "mft", "mgg", "mgj", "mgr", "mhk", "mun", "rmg", "rak", "rdk",
          "rxg", "tm2", "tzb", "tzh", "zbg", "zgq", "zug",
      },
      {
          "hsdfbfda.ghu",
          "hsdfbfda.abc",
          "rbfdhkjs.abc",
          "rbjfyvbl.abc",
          "rbl.fsgjhdfb",
          "rbl.fdnolfbv",
          "rblkmvnf.abc",
          "rblplmbf.ghu",
          "rblplmbf.abc",
          "rubajvnr.ghu",
          "rubajvnr.abc",
      }};

  std::string missKeys[] = {"zahskjsdf", "aba", "", "z", "asdjl:dafnsjsdf"};

  for (int i = 0; i < 3; ++i) {
    for (size_t j = 0; j < keys[i].size(); ++j) {
      randTrie[i].emplace(keys[i][j], i + j + 1);
      randMap[i].emplace(keys[i][j], i + j + 1);
    }

    for (size_t j = 0; j < keys[i].size(); ++j) {
      keysToGet[i].push_back(keys[i][j] + ":hit");
    }

    for (size_t j = 0; j < 5; ++j) {
      keysToGet[i].push_back(missKeys[j]);
    }

    LOG(INFO) << "#" << i << " uses " << keysToGet[i].size() << " keys";
  }
}

template <class Container>
void runGet(Container& c, int id) {
  auto& keys = keysToGet[id];
  for (size_t i = 0; i < keys.size(); ++i) {
    auto r = c.find(keys[i]);
    x += r == c.end() ? 0 : r->second;
  }
}

template <class Container>
void runGetPrefix(Container& c, int id) {
  auto& keys = keysToGet[id];
  for (size_t i = 0; i < keys.size(); ++i) {
    auto r = c.findPrefix(keys[i]);
    x += r == c.end() ? 0 : r->second;
  }
}

} // anonymous namespace

BENCHMARK(Trie_get0) {
  runGet(randTrie[0], 0);
}

BENCHMARK_RELATIVE(Map_get0) {
  runGet(randMap[0], 0);
}

BENCHMARK(Trie_get1) {
  runGet(randTrie[1], 1);
}

BENCHMARK_RELATIVE(Map_get1) {
  runGet(randMap[1], 1);
}

BENCHMARK(Trie_get2) {
  runGet(randTrie[2], 2);
}

BENCHMARK_RELATIVE(Map_get2) {
  runGet(randMap[2], 2);
}

BENCHMARK(Trie_get_prefix0) {
  runGetPrefix(randTrie[0], 0);
}

BENCHMARK_RELATIVE(Map_get_prefix0) {
  runGet(randMap[0], 0);
}

BENCHMARK(Trie_get_prefix1) {
  runGetPrefix(randTrie[1], 1);
}

BENCHMARK_RELATIVE(Map_get_prefix1) {
  runGet(randMap[1], 1);
}

BENCHMARK(Trie_get_prefix2) {
  runGetPrefix(randTrie[2], 2);
}

BENCHMARK_RELATIVE(Map_get_prefix2) {
  runGet(randMap[2], 2);
}

int main(int argc, char** argv) {
  folly::init(&argc, &argv, true /* removeFlags */);
  prepareRand();
  folly::runBenchmarks();
  LOG(INFO) << "check: " << x;
  return 0;
}

/*
 * ============================================================================
 * mcrouter/lib/fbi/cpp/test/TrieBenchmarks.cpp    relative  time/iter  iters/s
 * ============================================================================
 * Trie_get0                                                  184.44ns    5.42M
 * Map_get0                                          62.97%   292.89ns    3.41M
 * Trie_get1                                                  354.96ns    2.82M
 * Map_get1                                          29.92%     1.19us  842.95K
 * Trie_get2                                                  403.62ns    2.48M
 * Map_get2                                          69.33%   582.21ns    1.72M
 * Trie_get_prefix0                                           212.50ns    4.71M
 * Map_get_prefix0                                   72.62%   292.61ns    3.42M
 * Trie_get_prefix1                                           453.76ns    2.20M
 * Map_get_prefix1                                   38.27%     1.19us  843.42K
 * Trie_get_prefix2                                           482.68ns    2.07M
 * Map_get_prefix2                                   82.98%   581.71ns    1.72M
 * ============================================================================
 */
