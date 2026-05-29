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
#include <folly/init/Init.h>

#include <thrift/lib/cpp2/fast_thrift/common/CompactVariant.h>

#include <memory>
#include <variant>

using namespace apache::thrift::fast_thrift;

namespace {

// =============================================================================
// Test types — mirrors the ThriftResponseMessage payload types.
// =============================================================================

struct SmallType {
  uint64_t value{0};
};

struct LargeType {
  char data[48]{};
  std::unique_ptr<int> ptr;
};

using StdV = std::variant<SmallType, LargeType, std::unique_ptr<int>>;
using CompactV = CompactVariant<SmallType, LargeType, std::unique_ptr<int>>;

// =============================================================================
// Construction
// =============================================================================

BENCHMARK(StdVariant_ConstructSmall, n) {
  for (unsigned i = 0; i < n; ++i) {
    StdV v(SmallType{42});
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_RELATIVE(CompactVariant_ConstructSmall, n) {
  for (unsigned i = 0; i < n; ++i) {
    CompactV v(SmallType{42});
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_ConstructLarge, n) {
  for (unsigned i = 0; i < n; ++i) {
    StdV v(LargeType{});
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_RELATIVE(CompactVariant_ConstructLarge, n) {
  for (unsigned i = 0; i < n; ++i) {
    CompactV v(LargeType{});
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_ConstructMoveOnly, n) {
  for (unsigned i = 0; i < n; ++i) {
    StdV v(std::make_unique<int>(42));
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_RELATIVE(CompactVariant_ConstructMoveOnly, n) {
  for (unsigned i = 0; i < n; ++i) {
    CompactV v(std::make_unique<int>(42));
    folly::doNotOptimizeAway(v);
  }
}

// =============================================================================
// Emplace
// =============================================================================

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_Emplace, n) {
  StdV v(SmallType{0});
  for (unsigned i = 0; i < n; ++i) {
    v.emplace<SmallType>(SmallType{i});
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_RELATIVE(CompactVariant_Emplace, n) {
  CompactV v(SmallType{0});
  for (unsigned i = 0; i < n; ++i) {
    v.emplace<SmallType>(SmallType{i});
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_EmplaceSwitchType, n) {
  StdV v(SmallType{0});
  for (unsigned i = 0; i < n; ++i) {
    if (i % 2 == 0) {
      v.emplace<SmallType>(SmallType{i});
    } else {
      v.emplace<LargeType>();
    }
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_RELATIVE(CompactVariant_EmplaceSwitchType, n) {
  CompactV v(SmallType{0});
  for (unsigned i = 0; i < n; ++i) {
    if (i % 2 == 0) {
      v.emplace<SmallType>(SmallType{i});
    } else {
      v.emplace<LargeType>();
    }
    folly::doNotOptimizeAway(v);
  }
}

// =============================================================================
// Get
// =============================================================================

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_Get, n) {
  StdV v(SmallType{42});
  for (unsigned i = 0; i < n; ++i) {
    auto& val = std::get<SmallType>(v);
    folly::doNotOptimizeAway(val);
  }
}

BENCHMARK_RELATIVE(CompactVariant_Get, n) {
  CompactV v(SmallType{42});
  for (unsigned i = 0; i < n; ++i) {
    auto& val = v.get<SmallType>();
    folly::doNotOptimizeAway(val);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_GetLargeType, n) {
  StdV v(LargeType{});
  for (unsigned i = 0; i < n; ++i) {
    auto& val = std::get<LargeType>(v);
    folly::doNotOptimizeAway(val);
  }
}

BENCHMARK_RELATIVE(CompactVariant_GetLargeType, n) {
  CompactV v(LargeType{});
  for (unsigned i = 0; i < n; ++i) {
    auto& val = v.get<LargeType>();
    folly::doNotOptimizeAway(val);
  }
}

// =============================================================================
// holds_alternative / is
// =============================================================================

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_HoldsAlternative, n) {
  StdV v(SmallType{42});
  for (unsigned i = 0; i < n; ++i) {
    bool result = std::holds_alternative<SmallType>(v);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(CompactVariant_HoldsAlternative, n) {
  CompactV v(SmallType{42});
  for (unsigned i = 0; i < n; ++i) {
    bool result = holds_alternative<SmallType>(v);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_HoldsAlternativeMiss, n) {
  StdV v(LargeType{});
  for (unsigned i = 0; i < n; ++i) {
    bool result = std::holds_alternative<SmallType>(v);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(CompactVariant_HoldsAlternativeMiss, n) {
  CompactV v(LargeType{});
  for (unsigned i = 0; i < n; ++i) {
    bool result = holds_alternative<SmallType>(v);
    folly::doNotOptimizeAway(result);
  }
}

// =============================================================================
// Move Construction
// =============================================================================

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_MoveConstructSmall, n) {
  for (unsigned i = 0; i < n; ++i) {
    StdV src(SmallType{42});
    StdV dst(std::move(src));
    folly::doNotOptimizeAway(dst);
  }
}

BENCHMARK_RELATIVE(CompactVariant_MoveConstructSmall, n) {
  for (unsigned i = 0; i < n; ++i) {
    CompactV src(SmallType{42});
    CompactV dst(std::move(src));
    folly::doNotOptimizeAway(dst);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_MoveConstructLarge, n) {
  for (unsigned i = 0; i < n; ++i) {
    StdV src(LargeType{});
    StdV dst(std::move(src));
    folly::doNotOptimizeAway(dst);
  }
}

BENCHMARK_RELATIVE(CompactVariant_MoveConstructLarge, n) {
  for (unsigned i = 0; i < n; ++i) {
    CompactV src(LargeType{});
    CompactV dst(std::move(src));
    folly::doNotOptimizeAway(dst);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_MoveConstructMoveOnly, n) {
  for (unsigned i = 0; i < n; ++i) {
    StdV src(std::make_unique<int>(42));
    StdV dst(std::move(src));
    folly::doNotOptimizeAway(dst);
  }
}

BENCHMARK_RELATIVE(CompactVariant_MoveConstructMoveOnly, n) {
  for (unsigned i = 0; i < n; ++i) {
    CompactV src(std::make_unique<int>(42));
    CompactV dst(std::move(src));
    folly::doNotOptimizeAway(dst);
  }
}

// =============================================================================
// Move Assignment
// =============================================================================

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_MoveAssign, n) {
  StdV dst(SmallType{0});
  for (unsigned i = 0; i < n; ++i) {
    StdV src(SmallType{i});
    dst = std::move(src);
    folly::doNotOptimizeAway(dst);
  }
}

BENCHMARK_RELATIVE(CompactVariant_MoveAssign, n) {
  CompactV dst(SmallType{0});
  for (unsigned i = 0; i < n; ++i) {
    CompactV src(SmallType{i});
    dst = std::move(src);
    folly::doNotOptimizeAway(dst);
  }
}

// =============================================================================
// Assignment from value
// =============================================================================

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_AssignValue, n) {
  StdV v(SmallType{0});
  for (unsigned i = 0; i < n; ++i) {
    v = SmallType{i};
    folly::doNotOptimizeAway(v);
  }
}

BENCHMARK_RELATIVE(CompactVariant_AssignValue, n) {
  CompactV v(SmallType{0});
  for (unsigned i = 0; i < n; ++i) {
    v = SmallType{i};
    folly::doNotOptimizeAway(v);
  }
}

// =============================================================================
// sizeof comparison
// =============================================================================

BENCHMARK_DRAW_LINE();

BENCHMARK(StdVariant_Sizeof) {
  folly::doNotOptimizeAway(sizeof(StdV));
}

BENCHMARK(CompactVariant_Sizeof) {
  folly::doNotOptimizeAway(sizeof(CompactV));
}

} // namespace

// =============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  LOG(INFO) << "sizeof(std::variant):    " << sizeof(StdV);
  LOG(INFO) << "sizeof(CompactVariant):  " << sizeof(CompactV);
  LOG(INFO) << "savings:                 " << (sizeof(StdV) - sizeof(CompactV))
            << " bytes";

  folly::runBenchmarks();
  return 0;
}
