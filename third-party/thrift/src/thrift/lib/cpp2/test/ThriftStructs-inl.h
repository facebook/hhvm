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
#include <string>
#include <vector>

#include <folly/Random.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ProtocolBenchData_layouts.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ProtocolBenchData_types.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ProtocolBenchData_types_custom_protocol.h>

// template specifications for thrift struct

template <>
inline thrift::benchmark::Empty create<thrift::benchmark::Empty>() {
  return thrift::benchmark::Empty();
}

template <>
inline thrift::benchmark::OpEmpty create<thrift::benchmark::OpEmpty>() {
  return thrift::benchmark::OpEmpty();
}

template <>
inline thrift::benchmark::SmallInt create<thrift::benchmark::SmallInt>() {
  thrift::benchmark::SmallInt d;
  d.smallint() = 5;
  return d;
}

template <>
inline thrift::benchmark::OpSmallInt create<thrift::benchmark::OpSmallInt>() {
  thrift::benchmark::OpSmallInt d;
  d.smallint() = 5;
  return d;
}

template <>
inline thrift::benchmark::BigInt create<thrift::benchmark::BigInt>() {
  thrift::benchmark::BigInt d;
  d.bigint() = 0x1234567890abcdefL;
  return d;
}

template <>
inline thrift::benchmark::OpBigInt create<thrift::benchmark::OpBigInt>() {
  thrift::benchmark::OpBigInt d;
  d.bigint() = 0x1234567890abcdefL;
  return d;
}

template <>
inline thrift::benchmark::SmallString create<thrift::benchmark::SmallString>() {
  thrift::benchmark::SmallString d;
  d.str() = "small string";
  return d;
}

template <>
inline thrift::benchmark::OpSmallString
create<thrift::benchmark::OpSmallString>() {
  thrift::benchmark::OpSmallString d;
  d.str() = "small string";
  return d;
}

template <>
inline thrift::benchmark::BigString create<thrift::benchmark::BigString>() {
  thrift::benchmark::BigString d;
  d.str() = std::string(10'000, 'a');
  return d;
}

template <>
inline thrift::benchmark::OpBigString create<thrift::benchmark::OpBigString>() {
  thrift::benchmark::OpBigString d;
  d.str() = std::string(10'000, 'a');
  return d;
}

template <>
inline thrift::benchmark::BigBinary create<thrift::benchmark::BigBinary>() {
  auto buf = folly::IOBuf::create(10'000);
  buf->append(10'000);
  thrift::benchmark::BigBinary d;
  d.bin() = std::move(buf);
  return d;
}

template <>
inline thrift::benchmark::OpBigBinary create<thrift::benchmark::OpBigBinary>() {
  auto buf = folly::IOBuf::create(10'000);
  buf->append(10'000);
  thrift::benchmark::OpBigBinary d;
  d.bin() = std::move(buf);
  return d;
}

template <>
inline thrift::benchmark::LargeBinary create<thrift::benchmark::LargeBinary>() {
  auto buf = folly::IOBuf::create(10'000'000);
  buf->append(10'000'000);
  thrift::benchmark::LargeBinary d;
  d.bin() = std::move(buf);
  return d;
}

template <>
inline thrift::benchmark::OpLargeBinary
create<thrift::benchmark::OpLargeBinary>() {
  auto buf = folly::IOBuf::create(10'000'000);
  buf->append(10'000'000);
  thrift::benchmark::OpLargeBinary d;
  d.bin() = std::move(buf);
  return d;
}

template <>
inline thrift::benchmark::Mixed create<thrift::benchmark::Mixed>() {
  thrift::benchmark::Mixed d;
  d.int32() = 5;
  d.int64() = 12345;
  d.b() = true;
  d.str() = "hellohellohellohello";
  return d;
}

template <>
inline thrift::benchmark::OpMixed create<thrift::benchmark::OpMixed>() {
  thrift::benchmark::OpMixed d;
  d.int32() = 5;
  d.int64() = 12345;
  d.b() = true;
  d.str() = "hellohellohellohello";
  return d;
}

template <>
inline thrift::benchmark::MixedUnion create<thrift::benchmark::MixedUnion>() {
  thrift::benchmark::MixedUnion d;
  d.int64() = 12345;
  return d;
}

template <>
inline thrift::benchmark::OpMixedUnion
create<thrift::benchmark::OpMixedUnion>() {
  thrift::benchmark::OpMixedUnion d;
  d.int64() = 12345;
  return d;
}

template <class T>
inline T createList(int size) {
  std::mt19937 rng;
  T d;
  d.lst()->resize(size);
  for (int i = 0; i < size; ++i) {
    d.lst()->at(i) = rng();
  }
  return d;
}

template <>
inline thrift::benchmark::SmallListInt
create<thrift::benchmark::SmallListInt>() {
  return createList<thrift::benchmark::SmallListInt>(10);
}

template <>
inline thrift::benchmark::OpSmallListInt
create<thrift::benchmark::OpSmallListInt>() {
  return createList<thrift::benchmark::OpSmallListInt>(10);
}

template <>
inline thrift::benchmark::BigListByte create<thrift::benchmark::BigListByte>() {
  return createList<thrift::benchmark::BigListByte>(10000);
}

template <>
inline thrift::benchmark::OpBigListByte
create<thrift::benchmark::OpBigListByte>() {
  return createList<thrift::benchmark::OpBigListByte>(10000);
}

template <>
inline thrift::benchmark::BigListShort
create<thrift::benchmark::BigListShort>() {
  return createList<thrift::benchmark::BigListShort>(10000);
}

template <>
inline thrift::benchmark::OpBigListShort
create<thrift::benchmark::OpBigListShort>() {
  return createList<thrift::benchmark::OpBigListShort>(10000);
}

template <>
inline thrift::benchmark::BigListInt create<thrift::benchmark::BigListInt>() {
  return createList<thrift::benchmark::BigListInt>(10000);
}

template <>
inline thrift::benchmark::OpBigListInt
create<thrift::benchmark::OpBigListInt>() {
  return createList<thrift::benchmark::OpBigListInt>(10000);
}

template <>
inline thrift::benchmark::BigListBigInt
create<thrift::benchmark::BigListBigInt>() {
  return createList<thrift::benchmark::BigListBigInt>(10000);
}

template <>
inline thrift::benchmark::OpBigListBigInt
create<thrift::benchmark::OpBigListBigInt>() {
  return createList<thrift::benchmark::OpBigListBigInt>(10000);
}

template <>
inline thrift::benchmark::BigListFloat
create<thrift::benchmark::BigListFloat>() {
  return createList<thrift::benchmark::BigListFloat>(10000);
}

template <>
inline thrift::benchmark::OpBigListFloat
create<thrift::benchmark::OpBigListFloat>() {
  return createList<thrift::benchmark::OpBigListFloat>(10000);
}

template <>
inline thrift::benchmark::BigListDouble
create<thrift::benchmark::BigListDouble>() {
  return createList<thrift::benchmark::BigListDouble>(10000);
}

template <>
inline thrift::benchmark::OpBigListDouble
create<thrift::benchmark::OpBigListDouble>() {
  return createList<thrift::benchmark::OpBigListDouble>(10000);
}

template <>
inline thrift::benchmark::BigListMixed
create<thrift::benchmark::BigListMixed>() {
  std::vector<thrift::benchmark::Mixed> vec(
      10'000, create<thrift::benchmark::Mixed>());
  thrift::benchmark::BigListMixed d;
  d.lst() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::OpBigListMixed
create<thrift::benchmark::OpBigListMixed>() {
  std::vector<thrift::benchmark::OpMixed> vec(
      10'000, create<thrift::benchmark::OpMixed>());
  thrift::benchmark::OpBigListMixed d;
  d.lst() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::LargeListMixed
create<thrift::benchmark::LargeListMixed>() {
  std::vector<thrift::benchmark::Mixed> vec(
      1'000'000, create<thrift::benchmark::Mixed>());
  thrift::benchmark::LargeListMixed d;
  d.lst() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::OpLargeListMixed
create<thrift::benchmark::OpLargeListMixed>() {
  std::vector<thrift::benchmark::OpMixed> vec(
      1'000'000, create<thrift::benchmark::OpMixed>());
  thrift::benchmark::OpLargeListMixed d;
  d.lst() = std::move(vec);
  return d;
}

template <class T>
inline T createSet(int size) {
  std::mt19937 rng;
  T d;
  while (size-- != 0) {
    d.s()->insert(rng());
  }
  return d;
}

template <>
inline thrift::benchmark::LargeSetInt create<thrift::benchmark::LargeSetInt>() {
  return createSet<thrift::benchmark::LargeSetInt>(1000000);
}

template <>
inline thrift::benchmark::OpLargeSetInt
create<thrift::benchmark::OpLargeSetInt>() {
  return createSet<thrift::benchmark::OpLargeSetInt>(1000000);
}

template <>
inline thrift::benchmark::UnorderedSetInt
create<thrift::benchmark::UnorderedSetInt>() {
  return createSet<thrift::benchmark::UnorderedSetInt>(1000000);
}

template <>
inline thrift::benchmark::OpUnorderedSetInt
create<thrift::benchmark::OpUnorderedSetInt>() {
  return createSet<thrift::benchmark::OpUnorderedSetInt>(1000000);
}

template <>
inline thrift::benchmark::SortedVecSetInt
create<thrift::benchmark::SortedVecSetInt>() {
  return createSet<thrift::benchmark::SortedVecSetInt>(1000000);
}

template <>
inline thrift::benchmark::OpSortedVecSetInt
create<thrift::benchmark::OpSortedVecSetInt>() {
  return createSet<thrift::benchmark::OpSortedVecSetInt>(1000000);
}

template <class T>
inline T createMap(int size) {
  std::mt19937 rng;
  T d;
  while (size-- != 0) {
    d.m()[size] = rng();
  }
  return d;
}

template <class T>
inline T createMapMixed(int size) {
  T d;
  for (int i = 0; i < size; i++) {
    d.m()[i] = create<typename decltype(d.m())::value_type::mapped_type>();
  }
  return d;
}

template <>
inline thrift::benchmark::LargeMapInt create<thrift::benchmark::LargeMapInt>() {
  return createMap<thrift::benchmark::LargeMapInt>(1000000);
}

template <>
inline thrift::benchmark::OpLargeMapInt
create<thrift::benchmark::OpLargeMapInt>() {
  return createMap<thrift::benchmark::OpLargeMapInt>(1000000);
}

template <>
inline thrift::benchmark::LargeMapMixed
create<thrift::benchmark::LargeMapMixed>() {
  return createMapMixed<thrift::benchmark::LargeMapMixed>(1000000);
}

template <>
inline thrift::benchmark::OpLargeMapMixed
create<thrift::benchmark::OpLargeMapMixed>() {
  return createMapMixed<thrift::benchmark::OpLargeMapMixed>(1000000);
}

template <>
inline thrift::benchmark::LargeUnorderedMapMixed
create<thrift::benchmark::LargeUnorderedMapMixed>() {
  return createMapMixed<thrift::benchmark::LargeUnorderedMapMixed>(1000000);
}

template <>
inline thrift::benchmark::OpLargeUnorderedMapMixed
create<thrift::benchmark::OpLargeUnorderedMapMixed>() {
  return createMapMixed<thrift::benchmark::OpLargeUnorderedMapMixed>(1000000);
}

template <>
inline thrift::benchmark::LargeSortedVecMapMixed
create<thrift::benchmark::LargeSortedVecMapMixed>() {
  return createMapMixed<thrift::benchmark::LargeSortedVecMapMixed>(1000000);
}

template <>
inline thrift::benchmark::OpLargeSortedVecMapMixed
create<thrift::benchmark::OpLargeSortedVecMapMixed>() {
  return createMapMixed<thrift::benchmark::OpLargeSortedVecMapMixed>(1000000);
}

template <>
inline thrift::benchmark::UnorderedMapInt
create<thrift::benchmark::UnorderedMapInt>() {
  return createMap<thrift::benchmark::UnorderedMapInt>(1000000);
}

template <>
inline thrift::benchmark::OpUnorderedMapInt
create<thrift::benchmark::OpUnorderedMapInt>() {
  return createMap<thrift::benchmark::OpUnorderedMapInt>(1000000);
}

template <>
inline thrift::benchmark::NestedMapRaw
create<thrift::benchmark::NestedMapRaw>() {
  thrift::benchmark::NestedMapRaw map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    map.m()[i][j][k][l][m] = v;
  });
  return map;
}

template <>
inline thrift::benchmark::OpNestedMapRaw
create<thrift::benchmark::OpNestedMapRaw>() {
  thrift::benchmark::OpNestedMapRaw map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    map.m()[i][j][k][l][m] = v;
  });
  return map;
}

template <>
inline thrift::benchmark::SortedVecNestedMapRaw
create<thrift::benchmark::SortedVecNestedMapRaw>() {
  thrift::benchmark::SortedVecNestedMapRaw map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    map.m()[i][j][k][l][m] = v;
  });
  return map;
}

template <>
inline thrift::benchmark::OpSortedVecNestedMapRaw
create<thrift::benchmark::OpSortedVecNestedMapRaw>() {
  thrift::benchmark::OpSortedVecNestedMapRaw map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    map.m()[i][j][k][l][m] = v;
  });
  return map;
}

template <class T>
inline T createNestedMap() {
  T d;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    d.m()[i].m()[j].m()[k].m()[l].m()[m] = v;
  });
  return d;
}

template <>
inline thrift::benchmark::NestedMap create<thrift::benchmark::NestedMap>() {
  return createNestedMap<thrift::benchmark::NestedMap>();
}

template <>
inline thrift::benchmark::OpNestedMap create<thrift::benchmark::OpNestedMap>() {
  return createNestedMap<thrift::benchmark::OpNestedMap>();
}

template <>
inline thrift::benchmark::SortedVecNestedMap
create<thrift::benchmark::SortedVecNestedMap>() {
  return createNestedMap<thrift::benchmark::SortedVecNestedMap>();
}

template <>
inline thrift::benchmark::OpSortedVecNestedMap
create<thrift::benchmark::OpSortedVecNestedMap>() {
  return createNestedMap<thrift::benchmark::OpSortedVecNestedMap>();
}

template <class T>
inline T createLargeMixed() {
  T d;
  d.var1() = 5;
  d.var2() = 12345;
  d.var3() = true;
  d.var4() = "hello";
  d.var5() = 5;
  d.var6() = 12345;
  d.var7() = true;
  d.var8() = "hello";
  d.var9() = 5;
  d.var10() = 12345;
  d.var11() = true;
  d.var12() = "hello";
  d.var13() = 5;
  d.var14() = 12345;
  d.var15() = true;
  d.var16() = "hello";
  d.var17() = 5;
  d.var18() = 12345;
  d.var19() = true;
  d.var20() = "hello";
  d.var21() = 5;
  d.var22() = 12345;
  d.var23() = true;
  d.var24() = "hello";
  d.var25() = 5;
  d.var26() = 12345;
  d.var27() = true;
  d.var28() = "hello";
  d.var29() = 5;
  d.var30() = 12345;
  d.var31() = true;
  d.var32() = "hello";
  d.var33() = 5;
  d.var34() = 12345;
  d.var35() = true;
  d.var36() = "hello";
  d.var37() = 5;
  d.var38() = 12345;
  d.var39() = true;
  d.var40() = "hello";
  d.var41() = 5;
  d.var42() = 12345;
  d.var43() = true;
  d.var44() = "hello";
  d.var45() = 5;
  d.var46() = 12345;
  d.var47() = true;
  d.var48() = "hello";
  d.var49() = 5;
  d.var50() = 12345;
  d.var51() = true;
  d.var52() = "hello";
  d.var53() = 5;
  d.var54() = 12345;
  d.var55() = true;
  d.var56() = "hello";
  d.var57() = 5;
  d.var58() = 12345;
  d.var59() = true;
  d.var60() = "hello";
  d.var61() = 5;
  d.var62() = 12345;
  d.var63() = true;
  d.var64() = "hello";
  d.var65() = 5;
  d.var66() = 12345;
  d.var67() = true;
  d.var68() = "hello";
  d.var69() = 5;
  d.var70() = 12345;
  d.var71() = true;
  d.var72() = "hello";
  d.var73() = 5;
  d.var74() = 12345;
  d.var75() = true;
  d.var76() = "hello";
  d.var77() = 5;
  d.var78() = 12345;
  d.var79() = true;
  d.var80() = "hello";
  d.var81() = 5;
  d.var82() = 12345;
  d.var83() = true;
  d.var84() = "hello";
  d.var85() = 5;
  d.var86() = 12345;
  d.var87() = true;
  d.var88() = "hello";
  d.var89() = 5;
  d.var90() = 12345;
  d.var91() = true;
  d.var92() = "hello";
  d.var93() = 5;
  d.var94() = 12345;
  d.var95() = true;
  d.var96() = "hello";
  d.var97() = 5;
  d.var98() = 12345;
  d.var99() = true;
  d.var100() = "hello";
  return d;
}

template <>
inline thrift::benchmark::LargeMixed create<thrift::benchmark::LargeMixed>() {
  return createLargeMixed<thrift::benchmark::LargeMixed>();
}

template <>
inline thrift::benchmark::OpLargeMixed
create<thrift::benchmark::OpLargeMixed>() {
  return createLargeMixed<thrift::benchmark::OpLargeMixed>();
}

template <class T>
inline T createLargeMixedSparse() {
  T d;

  // Set exactly 10% of the 100 fields (10 fields) with a good distribution
  d.var3() = true;
  d.var11() = "hello";
  d.var23() = true;
  d.var31() = true;
  d.var42() = 12345;
  d.var57() = 5;
  d.var68() = "hello";
  d.var74() = 12345;
  d.var89() = 5;
  d.var95() = true;

  return d;
}

template <>
inline thrift::benchmark::LargeMixedSparse
create<thrift::benchmark::LargeMixedSparse>() {
  return createLargeMixedSparse<thrift::benchmark::LargeMixedSparse>();
}

template <>
inline thrift::benchmark::OpLargeMixedSparse
create<thrift::benchmark::OpLargeMixedSparse>() {
  return createLargeMixedSparse<thrift::benchmark::OpLargeMixedSparse>();
}

template <class T>
inline T createMixedInt() {
  std::mt19937 rng;
  T d;
  d.var1() = rng();
  d.var2() = rng();
  d.var3() = rng();
  d.var4() = rng();
  d.var5() = rng();
  d.var6() = rng();
  d.var7() = rng();
  d.var8() = rng();
  d.var9() = rng();
  d.varx() = rng();
  d.vary() = rng();
  d.varz() = rng();
  return d;
}

template <>
inline thrift::benchmark::MixedInt create<thrift::benchmark::MixedInt>() {
  return createMixedInt<thrift::benchmark::MixedInt>();
}

template <>
inline thrift::benchmark::OpMixedInt create<thrift::benchmark::OpMixedInt>() {
  return createMixedInt<thrift::benchmark::OpMixedInt>();
}

template <>
inline thrift::benchmark::BigListMixedInt
create<thrift::benchmark::BigListMixedInt>() {
  std::vector<thrift::benchmark::MixedInt> vec(
      10'000, create<thrift::benchmark::MixedInt>());
  thrift::benchmark::BigListMixedInt d;
  d.lst() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::OpBigListMixedInt
create<thrift::benchmark::OpBigListMixedInt>() {
  std::vector<thrift::benchmark::OpMixedInt> vec(
      10'000, create<thrift::benchmark::OpMixedInt>());
  thrift::benchmark::OpBigListMixedInt d;
  d.lst() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::ComplexStruct
create<thrift::benchmark::ComplexStruct>() {
  thrift::benchmark::ComplexStruct d;
  d.var1() = create<thrift::benchmark::Empty>();
  d.var2() = create<thrift::benchmark::SmallInt>();
  d.var3() = create<thrift::benchmark::BigInt>();
  d.var4() = create<thrift::benchmark::SmallString>();
  d.var5() = create<thrift::benchmark::BigString>();
  d.var6() = create<thrift::benchmark::Mixed>();
  d.var7() = create<thrift::benchmark::SmallListInt>();
  d.var8() = create<thrift::benchmark::BigListInt>();
  d.var9() = create<thrift::benchmark::LargeListMixed>();
  d.var10() = create<thrift::benchmark::LargeMapInt>();
  d.var11() = create<thrift::benchmark::LargeMixed>();
  d.var12() = create<thrift::benchmark::NestedMap>();
  return d;
}

template <>
inline thrift::benchmark::OpComplexStruct
create<thrift::benchmark::OpComplexStruct>() {
  thrift::benchmark::OpComplexStruct d;
  d.var1() = create<thrift::benchmark::OpEmpty>();
  d.var2() = create<thrift::benchmark::OpSmallInt>();
  d.var3() = create<thrift::benchmark::OpBigInt>();
  d.var4() = create<thrift::benchmark::OpSmallString>();
  d.var5() = create<thrift::benchmark::OpBigString>();
  d.var6() = create<thrift::benchmark::OpMixed>();
  d.var7() = create<thrift::benchmark::OpSmallListInt>();
  d.var8() = create<thrift::benchmark::OpBigListInt>();
  d.var9() = create<thrift::benchmark::OpLargeListMixed>();
  d.var10() = create<thrift::benchmark::OpLargeMapInt>();
  d.var11() = create<thrift::benchmark::OpLargeMixed>();
  d.var12() = create<thrift::benchmark::OpNestedMap>();
  return d;
}

template <>
inline thrift::benchmark::ComplexUnion
create<thrift::benchmark::ComplexUnion>() {
  thrift::benchmark::ComplexUnion d;
  d.var6_ref() = create<thrift::benchmark::Mixed>();
  return d;
}

template <>
inline thrift::benchmark::OpComplexUnion
create<thrift::benchmark::OpComplexUnion>() {
  thrift::benchmark::OpComplexUnion d;
  d.var6_ref() = create<thrift::benchmark::OpMixed>();
  return d;
}
