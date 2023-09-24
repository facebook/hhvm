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
#include <string>
#include <type_traits>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/test/gen-cpp2/StructPatchTest_types.h>

namespace apache::thrift::test::patch {

using ListPatch = folly::remove_cvref_t<
    decltype(*std::declval<MyStructFieldPatch>()->optListVal())>;
using ListDequePatch = folly::remove_cvref_t<
    decltype(*std::declval<MyStructFieldPatch>()->longList())>;
using SetPatch = folly::remove_cvref_t<
    decltype(*std::declval<MyStructFieldPatch>()->optSetVal())>;
using MapPatch = folly::remove_cvref_t<
    decltype(*std::declval<MyStructFieldPatch>()->optMapVal())>;
using ListStringPatch = folly::remove_cvref_t<
    decltype(*std::declval<StringsFieldPatch>()->strings())>;

// We apply/merge the patch N times to get a fairly complex patching.
// This is also used as the max index and key in list/set/map patch.
constexpr int N = 10;

std::mt19937 rng;
auto randInt() {
  return rng() % N;
}
auto randStr() {
  return std::to_string(randInt());
}
auto randLongStr() {
  return std::string(randInt() * 10, '0');
}

template <class PatchGenerator>
static inline const auto patches = std::invoke([] {
  rng.seed(0);
  using Patch = std::invoke_result_t<PatchGenerator>;
  std::array<Patch, N> ret;
  std::generate_n(ret.begin(), N, PatchGenerator{});
  return ret;
});

template <class PatchGenerator>
void benchmarkApply() {
  typename std::invoke_result_t<PatchGenerator>::value_type value;
  for (auto i = 0; i < N; i++) {
    patches<PatchGenerator>[i].apply(value);
  }
}

template <class PatchGenerator>
void benchmarkMerge() {
  std::invoke_result_t<PatchGenerator> patch;
  for (auto i = 0; i < N; i++) {
    patch.merge(patches<PatchGenerator>[i]);
  }
}

struct GenListPatch {
  ListPatch operator()() {
    ListPatch p;
    p.push_back(randInt());

    return p;
  }
};

BENCHMARK(ApplyListPatch) {
  benchmarkApply<GenListPatch>();
}
BENCHMARK(MergeListPatch) {
  benchmarkMerge<GenListPatch>();
}

struct GenListDequePatch {
  ListDequePatch operator()() {
    ListDequePatch p;
    p.push_back(randInt());
    return p;
  }
};

BENCHMARK(ApplyListDequePatch) {
  benchmarkApply<GenListDequePatch>();
}
BENCHMARK(MergeListDequePatch) {
  benchmarkMerge<GenListDequePatch>();
}

struct GenSetPatch {
  SetPatch operator()() {
    SetPatch p;
    p.insert(randStr());
    p.erase(randStr());
    return p;
  }
};

BENCHMARK(ApplySetPatch) {
  benchmarkApply<GenSetPatch>();
}
BENCHMARK(MergeSetPatch) {
  benchmarkMerge<GenSetPatch>();
}

struct GenMapPatch {
  MapPatch operator()() {
    MapPatch p;
    p.erase(randStr());
    p.patchByKey(randStr()) += randStr();
    p.ensureAndPatchByKey(randStr()) += randStr();
    return p;
  }
};

BENCHMARK(ApplyMapPatch) {
  benchmarkApply<GenMapPatch>();
}
BENCHMARK(MergeMapPatch) {
  benchmarkMerge<GenMapPatch>();
}

void patchIfSetNonOptionalFields(MyStructPatch& result) {
  result.patchIfSet<ident::boolVal>() = !op::BoolPatch{};
  result.patchIfSet<ident::byteVal>() += 1;
  result.patchIfSet<ident::i16Val>() += 2;
  result.patchIfSet<ident::i32Val>() += 3;
  result.patchIfSet<ident::i64Val>() += 4;
  result.patchIfSet<ident::floatVal>() += 5;
  result.patchIfSet<ident::doubleVal>() += 6;
  result.patchIfSet<ident::stringVal>() = "(" + op::StringPatch{} + ")";
  result.patchIfSet<ident::binaryVal>() = "<" + op::BinaryPatch{} + ">";
  result.patchIfSet<ident::enumVal>() = MyEnum::MyValue9;
  result.patchIfSet<ident::structVal>().patchIfSet<ident::data1>().append("X");
  result.patchIfSet<ident::unionVal>().patchIfSet<ident::option1>().append("Y");
  result.patchIfSet<ident::longList>() = GenListDequePatch{}();
}

void patchIfSetOptionalFields(MyStructPatch& result) {
  result.patchIfSet<ident::optBoolVal>() = !op::BoolPatch{};
  result.patchIfSet<ident::optByteVal>() += 1;
  result.patchIfSet<ident::optI16Val>() += 2;
  result.patchIfSet<ident::optI32Val>() += 3;
  result.patchIfSet<ident::optI64Val>() += 4;
  result.patchIfSet<ident::optFloatVal>() += 5;
  result.patchIfSet<ident::optDoubleVal>() += 6;
  result.patchIfSet<ident::optStringVal>() = "(" + op::StringPatch{} + ")";
  result.patchIfSet<ident::optBinaryVal>() = "<" + op::BinaryPatch{} + ">";
  result.patchIfSet<ident::optEnumVal>() = MyEnum::MyValue9;
  result.patchIfSet<ident::optStructVal>().patchIfSet<ident::data1>().append(
      "X");
  result.patchIfSet<ident::optListVal>() = GenListPatch{}();
  result.patchIfSet<ident::optSetVal>() = GenSetPatch{}();
  result.patchIfSet<ident::optMapVal>() = GenMapPatch{}();
}

void ensureNonOptionalFields(MyStructPatch& result) {
  result.ensure<ident::boolVal>();
  result.ensure<ident::byteVal>();
  result.ensure<ident::i16Val>();
  result.ensure<ident::i32Val>();
  result.ensure<ident::i64Val>();
  result.ensure<ident::floatVal>();
  result.ensure<ident::doubleVal>();
  result.ensure<ident::stringVal>();
  result.ensure<ident::binaryVal>();
  result.ensure<ident::enumVal>();
  result.ensure<ident::structVal>();
  result.ensure<ident::unionVal>();
  result.ensure<ident::longList>();
}

void ensureOptionalFields(MyStructPatch& result) {
  result.ensure<ident::optBoolVal>(true);
  result.ensure<ident::optByteVal>(1);
  result.ensure<ident::optI16Val>(2);
  result.ensure<ident::optI32Val>(3);
  result.ensure<ident::optI64Val>(4);
  result.ensure<ident::optFloatVal>(5);
  result.ensure<ident::optDoubleVal>(6);
  result.ensure<ident::optStringVal>("7");
  result.ensure<ident::optBinaryVal>(folly::IOBuf::wrapBufferAsValue("8", 1));
  result.ensure<ident::optEnumVal>(MyEnum::MyValue9);
  result.ensure<ident::optStructVal>([] {
    MyData data;
    data.data1() = "10";
    return data;
  }());
  result.ensure<ident::optListVal>({11});
  result.ensure<ident::optSetVal>({"10", "20"});
  result.ensure<ident::optMapVal>({{"10", "1"}, {"20", "2"}});
}

struct GenComplexPatch {
  MyStructPatch operator()() {
    MyStructPatch patch;
    patchIfSetNonOptionalFields(patch);
    patchIfSetOptionalFields(patch);
    ensureNonOptionalFields(patch);
    ensureOptionalFields(patch);
    patchIfSetNonOptionalFields(patch);
    patchIfSetOptionalFields(patch);
    return patch;
  }
};

BENCHMARK(ApplyComplexPatch) {
  benchmarkApply<GenComplexPatch>();
}

BENCHMARK(MergeComplexPatch) {
  benchmarkMerge<GenComplexPatch>();
}

struct GenListLongStringPatch {
  ListStringPatch operator()() {
    ListStringPatch p;
    p.push_back(randLongStr());
    return p;
  }
};

BENCHMARK(ApplyListLongStringPatch) {
  benchmarkApply<GenListLongStringPatch>();
}

BENCHMARK(MergeListLongStringPatch) {
  benchmarkApply<GenListLongStringPatch>();
}

} // namespace apache::thrift::test::patch

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  folly::runBenchmarks();
}
