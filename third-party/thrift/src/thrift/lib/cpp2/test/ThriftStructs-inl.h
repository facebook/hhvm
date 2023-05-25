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
inline thrift::benchmark::SmallInt create<thrift::benchmark::SmallInt>() {
  thrift::benchmark::SmallInt d;
  *d.smallint_ref() = 5;
  return d;
}

template <>
inline thrift::benchmark::BigInt create<thrift::benchmark::BigInt>() {
  thrift::benchmark::BigInt d;
  *d.bigint_ref() = 0x1234567890abcdefL;
  return d;
}

template <>
inline thrift::benchmark::SmallString create<thrift::benchmark::SmallString>() {
  thrift::benchmark::SmallString d;
  *d.str_ref() = "small string";
  return d;
}

template <>
inline thrift::benchmark::BigString create<thrift::benchmark::BigString>() {
  thrift::benchmark::BigString d;
  *d.str_ref() = std::string(10'000, 'a');
  return d;
}

template <>
inline thrift::benchmark::BigBinary create<thrift::benchmark::BigBinary>() {
  auto buf = folly::IOBuf::create(10'000);
  buf->append(10'000);
  thrift::benchmark::BigBinary d;
  *d.bin_ref() = std::move(buf);
  return d;
}

template <>
inline thrift::benchmark::LargeBinary create<thrift::benchmark::LargeBinary>() {
  auto buf = folly::IOBuf::create(10'000'000);
  buf->append(10'000'000);
  thrift::benchmark::LargeBinary d;
  *d.bin_ref() = std::move(buf);
  return d;
}

template <>
inline thrift::benchmark::Mixed create<thrift::benchmark::Mixed>() {
  thrift::benchmark::Mixed d;
  *d.int32_ref() = 5;
  *d.int64_ref() = 12345;
  *d.b_ref() = true;
  *d.str_ref() = "hellohellohellohello";
  return d;
}

template <>
inline thrift::benchmark::SmallListInt
create<thrift::benchmark::SmallListInt>() {
  std::srand(1);
  std::vector<int> vec;
  for (int i = 0; i < 10; i++) {
    vec.push_back(folly::Random::rand32());
  }
  thrift::benchmark::SmallListInt d;
  *d.lst_ref() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::BigListInt create<thrift::benchmark::BigListInt>() {
  std::srand(1);
  std::vector<int> vec;
  for (int i = 0; i < 10'000; i++) {
    vec.push_back(folly::Random::rand32());
  }
  thrift::benchmark::BigListInt d;
  *d.lst_ref() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::BigListMixed
create<thrift::benchmark::BigListMixed>() {
  std::vector<thrift::benchmark::Mixed> vec(
      10'000, create<thrift::benchmark::Mixed>());
  thrift::benchmark::BigListMixed d;
  *d.lst_ref() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::LargeListMixed
create<thrift::benchmark::LargeListMixed>() {
  std::vector<thrift::benchmark::Mixed> vec(
      1'000'000, create<thrift::benchmark::Mixed>());
  thrift::benchmark::LargeListMixed d;
  *d.lst_ref() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::LargeSetInt create<thrift::benchmark::LargeSetInt>() {
  std::srand(1);
  thrift::benchmark::LargeSetInt l;
  for (int i = 0; i < 1'000'000; i++) {
    l.s()->insert(folly::Random::rand32());
  }
  return l;
}

template <>
inline thrift::benchmark::LargeMapInt create<thrift::benchmark::LargeMapInt>() {
  std::srand(1);
  thrift::benchmark::LargeMapInt l;
  for (int i = 0; i < 1'000'000; i++) {
    l.m_ref()[i] = folly::Random::rand32();
  }
  return l;
}

template <>
inline thrift::benchmark::NestedMapRaw
create<thrift::benchmark::NestedMapRaw>() {
  thrift::benchmark::NestedMapRaw map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    map.m_ref()[i][j][k][l][m] = v;
  });
  return map;
}

template <>
inline thrift::benchmark::SortedVecNestedMapRaw
create<thrift::benchmark::SortedVecNestedMapRaw>() {
  thrift::benchmark::SortedVecNestedMapRaw map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    map.m_ref()[i][j][k][l][m] = v;
  });
  return map;
}

template <>
inline thrift::benchmark::NestedMap create<thrift::benchmark::NestedMap>() {
  thrift::benchmark::NestedMap map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    map.m_ref()[i].m_ref()[j].m_ref()[k].m_ref()[l].m_ref()[m] = v;
  });
  return map;
}

template <>
inline thrift::benchmark::SortedVecNestedMap
create<thrift::benchmark::SortedVecNestedMap>() {
  thrift::benchmark::SortedVecNestedMap map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    map.m_ref()[i].m_ref()[j].m_ref()[k].m_ref()[l].m_ref()[m] = v;
  });
  return map;
}

template <>
inline thrift::benchmark::LargeMixed create<thrift::benchmark::LargeMixed>() {
  thrift::benchmark::LargeMixed d;
  *d.var1_ref() = 5;
  *d.var2_ref() = 12345;
  *d.var3_ref() = true;
  *d.var4_ref() = "hello";
  *d.var5_ref() = 5;
  *d.var6_ref() = 12345;
  *d.var7_ref() = true;
  *d.var8_ref() = "hello";
  *d.var9_ref() = 5;
  *d.var10_ref() = 12345;
  *d.var11_ref() = true;
  *d.var12_ref() = "hello";
  *d.var13_ref() = 5;
  *d.var14_ref() = 12345;
  *d.var15_ref() = true;
  *d.var16_ref() = "hello";
  *d.var17_ref() = 5;
  *d.var18_ref() = 12345;
  *d.var19_ref() = true;
  *d.var20_ref() = "hello";
  *d.var21_ref() = 5;
  *d.var22_ref() = 12345;
  *d.var23_ref() = true;
  *d.var24_ref() = "hello";
  *d.var25_ref() = 5;
  *d.var26_ref() = 12345;
  *d.var27_ref() = true;
  *d.var28_ref() = "hello";
  *d.var29_ref() = 5;
  *d.var30_ref() = 12345;
  *d.var31_ref() = true;
  *d.var32_ref() = "hello";
  *d.var33_ref() = 5;
  *d.var34_ref() = 12345;
  *d.var35_ref() = true;
  *d.var36_ref() = "hello";
  *d.var37_ref() = 5;
  *d.var38_ref() = 12345;
  *d.var39_ref() = true;
  *d.var40_ref() = "hello";
  *d.var41_ref() = 5;
  *d.var42_ref() = 12345;
  *d.var43_ref() = true;
  *d.var44_ref() = "hello";
  *d.var45_ref() = 5;
  *d.var46_ref() = 12345;
  *d.var47_ref() = true;
  *d.var48_ref() = "hello";
  *d.var49_ref() = 5;
  *d.var50_ref() = 12345;
  *d.var51_ref() = true;
  *d.var52_ref() = "hello";
  *d.var53_ref() = 5;
  *d.var54_ref() = 12345;
  *d.var55_ref() = true;
  *d.var56_ref() = "hello";
  *d.var57_ref() = 5;
  *d.var58_ref() = 12345;
  *d.var59_ref() = true;
  *d.var60_ref() = "hello";
  *d.var61_ref() = 5;
  *d.var62_ref() = 12345;
  *d.var63_ref() = true;
  *d.var64_ref() = "hello";
  *d.var65_ref() = 5;
  *d.var66_ref() = 12345;
  *d.var67_ref() = true;
  *d.var68_ref() = "hello";
  *d.var69_ref() = 5;
  *d.var70_ref() = 12345;
  *d.var71_ref() = true;
  *d.var72_ref() = "hello";
  *d.var73_ref() = 5;
  *d.var74_ref() = 12345;
  *d.var75_ref() = true;
  *d.var76_ref() = "hello";
  *d.var77_ref() = 5;
  *d.var78_ref() = 12345;
  *d.var79_ref() = true;
  *d.var80_ref() = "hello";
  *d.var81_ref() = 5;
  *d.var82_ref() = 12345;
  *d.var83_ref() = true;
  *d.var84_ref() = "hello";
  *d.var85_ref() = 5;
  *d.var86_ref() = 12345;
  *d.var87_ref() = true;
  *d.var88_ref() = "hello";
  *d.var89_ref() = 5;
  *d.var90_ref() = 12345;
  *d.var91_ref() = true;
  *d.var92_ref() = "hello";
  *d.var93_ref() = 5;
  *d.var94_ref() = 12345;
  *d.var95_ref() = true;
  *d.var96_ref() = "hello";
  *d.var97_ref() = 5;
  *d.var98_ref() = 12345;
  *d.var99_ref() = true;
  *d.var100_ref() = "hello";
  return d;
}

template <>
inline thrift::benchmark::MixedInt create<thrift::benchmark::MixedInt>() {
  std::srand(1);
  thrift::benchmark::MixedInt d;
  *d.var1_ref() = folly::Random::rand32();
  *d.var2_ref() = folly::Random::rand32();
  *d.var3_ref() = folly::Random::rand32();
  *d.var4_ref() = folly::Random::rand32();
  *d.var5_ref() = folly::Random::rand32();
  *d.var6_ref() = folly::Random::rand32();
  *d.var7_ref() = folly::Random::rand32();
  *d.var8_ref() = folly::Random::rand32();
  *d.var9_ref() = folly::Random::rand32();
  *d.varx_ref() = folly::Random::rand32();
  *d.vary_ref() = folly::Random::rand32();
  *d.varz_ref() = folly::Random::rand32();
  return d;
}

template <>
inline thrift::benchmark::BigListMixedInt
create<thrift::benchmark::BigListMixedInt>() {
  std::vector<thrift::benchmark::MixedInt> vec(
      10'000, create<thrift::benchmark::MixedInt>());
  thrift::benchmark::BigListMixedInt d;
  *d.lst_ref() = std::move(vec);
  return d;
}

template <>
inline thrift::benchmark::ComplexStruct
create<thrift::benchmark::ComplexStruct>() {
  thrift::benchmark::ComplexStruct d;
  *d.var1_ref() = create<thrift::benchmark::Empty>();
  *d.var2_ref() = create<thrift::benchmark::SmallInt>();
  *d.var3_ref() = create<thrift::benchmark::BigInt>();
  *d.var4_ref() = create<thrift::benchmark::SmallString>();
  *d.var5_ref() = create<thrift::benchmark::BigString>();
  *d.var6_ref() = create<thrift::benchmark::Mixed>();
  *d.var7_ref() = create<thrift::benchmark::SmallListInt>();
  *d.var8_ref() = create<thrift::benchmark::BigListInt>();
  *d.var9_ref() = create<thrift::benchmark::LargeListMixed>();
  *d.var10_ref() = create<thrift::benchmark::LargeMapInt>();
  *d.var11_ref() = create<thrift::benchmark::LargeMixed>();
  *d.var12_ref() = create<thrift::benchmark::NestedMap>();
  return d;
}
