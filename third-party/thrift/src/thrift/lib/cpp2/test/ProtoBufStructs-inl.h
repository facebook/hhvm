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

#include <thrift/lib/cpp2/test/ProtoBufBenchData.pb.h>

// template specifications for protobuf message
template <>
inline protobuf::Empty create<protobuf::Empty>() {
  return protobuf::Empty();
}

template <>
inline protobuf::SmallInt create<protobuf::SmallInt>() {
  protobuf::SmallInt i;
  i.set_smallint(5);
  return i;
}

template <>
inline protobuf::BigInt create<protobuf::BigInt>() {
  protobuf::BigInt i;
  i.set_bigint(0x1234567890abcdefL);
  return i;
}

template <>
inline protobuf::SmallString create<protobuf::SmallString>() {
  protobuf::SmallString s;
  s.set_smallstr("small string");
  return s;
}

template <>
inline protobuf::BigString create<protobuf::BigString>() {
  protobuf::BigString s;
  s.set_bigstr(std::string(10000, 'a'));
  return s;
}

template <>
inline protobuf::Mixed create<protobuf::Mixed>() {
  protobuf::Mixed m;
  m.set_i32(5);
  m.set_i64(12345);
  m.set_b(true);
  m.set_s("hellohellohellohello");
  return m;
}

template <>
inline protobuf::SmallListInt create<protobuf::SmallListInt>() {
  std::srand(1);
  protobuf::SmallListInt l;
  for (int i = 0; i < 10; i++) {
    l.add_lst(std::rand());
  }
  return l;
}

template <>
inline protobuf::BigListInt create<protobuf::BigListInt>() {
  std::srand(1);
  protobuf::BigListInt l;
  for (int i = 0; i < 10000; i++) {
    l.add_lst(std::rand());
  }
  return l;
}

template <>
inline protobuf::BigListMixed create<protobuf::BigListMixed>() {
  protobuf::BigListMixed l;
  for (int i = 0; i < 10000; i++) {
    *l.add_lst() = create<protobuf::Mixed>();
  }
  return l;
}

template <>
inline protobuf::LargeListMixed create<protobuf::LargeListMixed>() {
  protobuf::LargeListMixed l;
  for (int i = 0; i < 1000000; i++) {
    *l.add_lst() = create<protobuf::Mixed>();
  }
  return l;
}

template <>
inline protobuf::LargeMapInt create<protobuf::LargeMapInt>() {
  std::srand(1);
  protobuf::LargeMapInt l;
  auto& m = *l.mutable_m();
  for (int i = 0; i < 1000000; i++) {
    m[i] = std::rand();
  }
  return l;
}

template <>
inline protobuf::NestedMap create<protobuf::NestedMap>() {
  protobuf::NestedMap map;
  populateMap([&](int i, int j, int k, int l, int m, int v) {
    auto& m4 = *map.mutable_m();
    auto& m3 = *m4[i].mutable_m();
    auto& m2 = *m3[j].mutable_m();
    auto& m1 = *m2[k].mutable_m();
    auto& m0 = *m1[l].mutable_m();
    m0[m] = v;
  });
  return map;
}

template <>
inline protobuf::LargeMixed create<protobuf::LargeMixed>() {
  protobuf::LargeMixed l;
  l.set_var0(5);
  l.set_var1(12345);
  l.set_var2(true);
  l.set_var3("hello");
  l.set_var4(5);
  l.set_var5(12345);
  l.set_var6(true);
  l.set_var7("hello");
  l.set_var8(5);
  l.set_var9(12345);
  l.set_var10(true);
  l.set_var11("hello");
  l.set_var12(5);
  l.set_var13(12345);
  l.set_var14(true);
  l.set_var15("hello");
  l.set_var16(5);
  l.set_var17(12345);
  l.set_var18(true);
  l.set_var19("hello");
  l.set_var20(5);
  l.set_var21(12345);
  l.set_var22(true);
  l.set_var23("hello");
  l.set_var24(5);
  l.set_var25(12345);
  l.set_var26(true);
  l.set_var27("hello");
  l.set_var28(5);
  l.set_var29(12345);
  l.set_var30(true);
  l.set_var31("hello");
  l.set_var32(5);
  l.set_var33(12345);
  l.set_var34(true);
  l.set_var35("hello");
  l.set_var36(5);
  l.set_var37(12345);
  l.set_var38(true);
  l.set_var39("hello");
  l.set_var40(5);
  l.set_var41(12345);
  l.set_var42(true);
  l.set_var43("hello");
  l.set_var44(5);
  l.set_var45(12345);
  l.set_var46(true);
  l.set_var47("hello");
  l.set_var48(5);
  l.set_var49(12345);
  l.set_var50(true);
  l.set_var51("hello");
  l.set_var52(5);
  l.set_var53(12345);
  l.set_var54(true);
  l.set_var55("hello");
  l.set_var56(5);
  l.set_var57(12345);
  l.set_var58(true);
  l.set_var59("hello");
  l.set_var60(5);
  l.set_var61(12345);
  l.set_var62(true);
  l.set_var63("hello");
  l.set_var64(5);
  l.set_var65(12345);
  l.set_var66(true);
  l.set_var67("hello");
  l.set_var68(5);
  l.set_var69(12345);
  l.set_var70(true);
  l.set_var71("hello");
  l.set_var72(5);
  l.set_var73(12345);
  l.set_var74(true);
  l.set_var75("hello");
  l.set_var76(5);
  l.set_var77(12345);
  l.set_var78(true);
  l.set_var79("hello");
  l.set_var80(5);
  l.set_var81(12345);
  l.set_var82(true);
  l.set_var83("hello");
  l.set_var84(5);
  l.set_var85(12345);
  l.set_var86(true);
  l.set_var87("hello");
  l.set_var88(5);
  l.set_var89(12345);
  l.set_var90(true);
  l.set_var91("hello");
  l.set_var92(5);
  l.set_var93(12345);
  l.set_var94(true);
  l.set_var95("hello");
  l.set_var96(5);
  l.set_var97(12345);
  l.set_var98(true);
  l.set_var99("hello");
  return l;
}

template <>
inline protobuf::MixedInt create<protobuf::MixedInt>() {
  std::srand(1);
  protobuf::MixedInt l;
  l.set_var1(std::rand());
  l.set_var2(std::rand());
  l.set_var3(std::rand());
  l.set_var4(std::rand());
  l.set_var5(std::rand());
  l.set_var6(std::rand());
  l.set_var7(std::rand());
  l.set_var8(std::rand());
  l.set_var9(std::rand());
  l.set_varx(std::rand());
  l.set_vary(std::rand());
  l.set_varz(std::rand());
  return l;
}

template <>
inline protobuf::ComplexStruct create<protobuf::ComplexStruct>() {
  protobuf::ComplexStruct l;
  *l.mutable_var0() = create<protobuf::Empty>();
  *l.mutable_var1() = create<protobuf::SmallInt>();
  *l.mutable_var2() = create<protobuf::BigInt>();
  *l.mutable_var3() = create<protobuf::SmallString>();
  *l.mutable_var4() = create<protobuf::BigString>();
  *l.mutable_var5() = create<protobuf::Mixed>();
  *l.mutable_var6() = create<protobuf::SmallListInt>();
  *l.mutable_var7() = create<protobuf::BigListInt>();
  *l.mutable_var8() = create<protobuf::LargeListMixed>();
  *l.mutable_var9() = create<protobuf::LargeMapInt>();
  *l.mutable_var10() = create<protobuf::LargeMixed>();
  *l.mutable_var11() = create<protobuf::NestedMap>();
  return l;
}
