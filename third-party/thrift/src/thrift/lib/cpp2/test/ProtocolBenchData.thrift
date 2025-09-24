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

package "meta.com/thrift/test/benchmark"

namespace cpp2 thrift.benchmark

cpp_include "folly/sorted_vector_types.h"

include "thrift/annotation/cpp.thrift"

struct Empty {}

struct SmallInt {
  1: i32 smallint;
}

struct BigInt {
  1: i64 bigint;
}

struct SmallString {
  1: string str;
}

struct BigString {
  1: string str;
}

@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBuf

struct BigBinary {
  1: IOBuf bin;
}

struct LargeBinary {
  1: IOBuf bin;
}

struct Mixed {
  1: i32 int32;
  2: i64 int64;
  3: bool b;
  4: string str;
}

struct SmallListInt {
  1: list<i32> lst;
}

struct BigListByte {
  1: list<byte> lst;
}

struct BigListShort {
  1: list<i16> lst;
}

struct BigListInt {
  1: list<i32> lst;
}

struct BigListBigInt {
  1: list<i64> lst;
}

struct BigListFloat {
  1: list<float> lst;
}

struct BigListDouble {
  1: list<double> lst;
}

struct BigListMixed {
  1: list<Mixed> lst;
}

struct LargeListMixed {
  1: list<Mixed> lst;
}

struct LargeSetInt {
  1: set<i32> s;
}

struct UnorderedSetInt {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> s;
}

struct SortedVecSetInt {
  @cpp.Type{template = "folly::sorted_vector_set"}
  1: set<i32> s;
}

struct LargeMapInt {
  1: map<i32, i32> m;
}

struct LargeMapMixed {
  1: map<i32, Mixed> m;
}

struct LargeUnorderedMapMixed {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, Mixed> m;
}

struct LargeSortedVecMapMixed {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, Mixed> m;
}

struct NestedMapRaw {
  1: map<i32, map<i32, map<i32, map<i32, map<i32, i32>>>>> m;
}

struct NestedMap1 {
  1: map<i32, i32> m;
}

struct NestedMap2 {
  1: map<i32, NestedMap1> m;
}

struct NestedMap3 {
  1: map<i32, NestedMap2> m;
}

struct NestedMap4 {
  1: map<i32, NestedMap3> m;
}

struct NestedMap {
  1: map<i32, NestedMap4> m;
}

@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<i32, i32> Map1
@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<i32, Map1> Map2
@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<i32, Map2> Map3
@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<i32, Map3> Map4
@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<i32, Map4> Map5

struct SortedVecNestedMapRaw {
  1: Map5 m;
}

struct SortedVecNestedMap1 {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, i32> m;
}

struct SortedVecNestedMap2 {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, NestedMap1> m;
}

struct SortedVecNestedMap3 {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, NestedMap2> m;
}

struct SortedVecNestedMap4 {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, NestedMap3> m;
}

struct SortedVecNestedMap {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, NestedMap4> m;
}

struct UnorderedMapInt {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, i32> m;
}

struct LargeMixed {
  1: i32 var1;
  2: i64 var2;
  3: bool var3;
  4: string var4;
  5: i32 var5;
  6: i64 var6;
  7: bool var7;
  8: string var8;
  9: i32 var9;
  10: i64 var10;
  11: bool var11;
  12: string var12;
  13: i32 var13;
  14: i64 var14;
  15: bool var15;
  16: string var16;
  17: i32 var17;
  18: i64 var18;
  19: bool var19;
  20: string var20;
  21: i32 var21;
  22: i64 var22;
  23: bool var23;
  24: string var24;
  25: i32 var25;
  26: i64 var26;
  27: bool var27;
  28: string var28;
  29: i32 var29;
  30: i64 var30;
  31: bool var31;
  32: string var32;
  33: i32 var33;
  34: i64 var34;
  35: bool var35;
  36: string var36;
  37: i32 var37;
  38: i64 var38;
  39: bool var39;
  40: string var40;
  41: i32 var41;
  42: i64 var42;
  43: bool var43;
  44: string var44;
  45: i32 var45;
  46: i64 var46;
  47: bool var47;
  48: string var48;
  49: i32 var49;
  50: i64 var50;
  51: bool var51;
  52: string var52;
  53: i32 var53;
  54: i64 var54;
  55: bool var55;
  56: string var56;
  57: i32 var57;
  58: i64 var58;
  59: bool var59;
  60: string var60;
  61: i32 var61;
  62: i64 var62;
  63: bool var63;
  64: string var64;
  65: i32 var65;
  66: i64 var66;
  67: bool var67;
  68: string var68;
  69: i32 var69;
  70: i64 var70;
  71: bool var71;
  72: string var72;
  73: i32 var73;
  74: i64 var74;
  75: bool var75;
  76: string var76;
  77: i32 var77;
  78: i64 var78;
  79: bool var79;
  80: string var80;
  81: i32 var81;
  82: i64 var82;
  83: bool var83;
  84: string var84;
  85: i32 var85;
  86: i64 var86;
  87: bool var87;
  88: string var88;
  89: i32 var89;
  90: i64 var90;
  91: bool var91;
  92: string var92;
  93: i32 var93;
  94: i64 var94;
  95: bool var95;
  96: string var96;
  97: i32 var97;
  98: i64 var98;
  99: bool var99;
  100: string var100;
}

struct MixedInt {
  1: i32 var1;
  2: i32 var2;
  3: i32 var3;
  4: i32 var4;
  5: i32 var5;
  6: i32 var6;
  7: i32 var7;
  8: i32 var8;
  9: i32 var9;
  10: i32 varx;
  11: i32 vary;
  12: i32 varz;
}

struct BigListMixedInt {
  1: list<MixedInt> lst;
}

struct ComplexStruct {
  1: Empty var1;
  2: SmallInt var2;
  4: BigInt var3;
  7: SmallString var4;
  11: BigString var5;
  16: Mixed var6;
  22: SmallListInt var7;
  29: BigListInt var8;
  37: LargeListMixed var9;
  46: LargeMapInt var10;
  56: LargeMixed var11;
  67: NestedMap var12;
}

union ComplexUnion {
  1: Empty var1;
  2: SmallInt var2;
  4: BigInt var3;
  7: SmallString var4;
  11: BigString var5;
  16: Mixed var6;
  22: SmallListInt var7;
  29: BigListInt var8;
  37: LargeListMixed var9;
  46: LargeMapInt var10;
  56: LargeMixed var11;
  67: NestedMap var12;
}

@cpp.UseOpEncode
struct OpEmpty {}

@cpp.UseOpEncode
struct OpSmallInt {
  1: i32 smallint;
}

@cpp.UseOpEncode
struct OpBigInt {
  1: i64 bigint;
}

@cpp.UseOpEncode
struct OpSmallString {
  1: string str;
}

@cpp.UseOpEncode
struct OpBigString {
  1: string str;
}

@cpp.UseOpEncode
struct OpBigBinary {
  1: IOBuf bin;
}

@cpp.UseOpEncode
struct OpLargeBinary {
  1: IOBuf bin;
}

@cpp.UseOpEncode
struct OpMixed {
  1: i32 int32;
  2: i64 int64;
  3: bool b;
  4: string str;
}

@cpp.UseOpEncode
struct OpSmallListInt {
  1: list<i32> lst;
}

@cpp.UseOpEncode
struct OpBigListByte {
  1: list<byte> lst;
}

@cpp.UseOpEncode
struct OpBigListShort {
  1: list<i16> lst;
}

@cpp.UseOpEncode
struct OpBigListInt {
  1: list<i32> lst;
}

@cpp.UseOpEncode
struct OpBigListBigInt {
  1: list<i64> lst;
}

@cpp.UseOpEncode
struct OpBigListFloat {
  1: list<float> lst;
}

@cpp.UseOpEncode
struct OpBigListDouble {
  1: list<double> lst;
}

@cpp.UseOpEncode
struct OpBigListMixed {
  1: list<OpMixed> lst;
}

@cpp.UseOpEncode
struct OpLargeListMixed {
  1: list<OpMixed> lst;
}

@cpp.UseOpEncode
struct OpLargeSetInt {
  1: set<i32> s;
}

@cpp.UseOpEncode
struct OpUnorderedSetInt {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> s;
}

@cpp.UseOpEncode
struct OpSortedVecSetInt {
  @cpp.Type{template = "folly::sorted_vector_set"}
  1: set<i32> s;
}

@cpp.UseOpEncode
struct OpLargeMapInt {
  1: map<i32, i32> m;
}

@cpp.UseOpEncode
struct OpLargeMapMixed {
  1: map<i32, OpMixed> m;
}

@cpp.UseOpEncode
struct OpLargeUnorderedMapMixed {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, OpMixed> m;
}

@cpp.UseOpEncode
struct OpLargeSortedVecMapMixed {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, OpMixed> m;
}

@cpp.UseOpEncode
struct OpNestedMapRaw {
  1: map<i32, map<i32, map<i32, map<i32, map<i32, i32>>>>> m;
}

@cpp.UseOpEncode
struct OpNestedMap1 {
  1: map<i32, i32> m;
}

@cpp.UseOpEncode
struct OpNestedMap2 {
  1: map<i32, OpNestedMap1> m;
}

@cpp.UseOpEncode
struct OpNestedMap3 {
  1: map<i32, OpNestedMap2> m;
}

@cpp.UseOpEncode
struct OpNestedMap4 {
  1: map<i32, OpNestedMap3> m;
}

@cpp.UseOpEncode
struct OpNestedMap {
  1: map<i32, OpNestedMap4> m;
}

@cpp.UseOpEncode
struct OpSortedVecNestedMapRaw {
  1: Map5 m;
}

@cpp.UseOpEncode
struct OpSortedVecNestedMap1 {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, i32> m;
}

@cpp.UseOpEncode
struct OpSortedVecNestedMap2 {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, OpNestedMap1> m;
}

@cpp.UseOpEncode
struct OpSortedVecNestedMap3 {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, OpNestedMap2> m;
}

@cpp.UseOpEncode
struct OpSortedVecNestedMap4 {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, OpNestedMap3> m;
}

@cpp.UseOpEncode
struct OpSortedVecNestedMap {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, OpNestedMap4> m;
}

@cpp.UseOpEncode
struct OpUnorderedMapInt {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, i32> m;
}

@cpp.UseOpEncode
struct OpLargeMixed {
  1: i32 var1;
  2: i64 var2;
  3: bool var3;
  4: string var4;
  5: i32 var5;
  6: i64 var6;
  7: bool var7;
  8: string var8;
  9: i32 var9;
  10: i64 var10;
  11: bool var11;
  12: string var12;
  13: i32 var13;
  14: i64 var14;
  15: bool var15;
  16: string var16;
  17: i32 var17;
  18: i64 var18;
  19: bool var19;
  20: string var20;
  21: i32 var21;
  22: i64 var22;
  23: bool var23;
  24: string var24;
  25: i32 var25;
  26: i64 var26;
  27: bool var27;
  28: string var28;
  29: i32 var29;
  30: i64 var30;
  31: bool var31;
  32: string var32;
  33: i32 var33;
  34: i64 var34;
  35: bool var35;
  36: string var36;
  37: i32 var37;
  38: i64 var38;
  39: bool var39;
  40: string var40;
  41: i32 var41;
  42: i64 var42;
  43: bool var43;
  44: string var44;
  45: i32 var45;
  46: i64 var46;
  47: bool var47;
  48: string var48;
  49: i32 var49;
  50: i64 var50;
  51: bool var51;
  52: string var52;
  53: i32 var53;
  54: i64 var54;
  55: bool var55;
  56: string var56;
  57: i32 var57;
  58: i64 var58;
  59: bool var59;
  60: string var60;
  61: i32 var61;
  62: i64 var62;
  63: bool var63;
  64: string var64;
  65: i32 var65;
  66: i64 var66;
  67: bool var67;
  68: string var68;
  69: i32 var69;
  70: i64 var70;
  71: bool var71;
  72: string var72;
  73: i32 var73;
  74: i64 var74;
  75: bool var75;
  76: string var76;
  77: i32 var77;
  78: i64 var78;
  79: bool var79;
  80: string var80;
  81: i32 var81;
  82: i64 var82;
  83: bool var83;
  84: string var84;
  85: i32 var85;
  86: i64 var86;
  87: bool var87;
  88: string var88;
  89: i32 var89;
  90: i64 var90;
  91: bool var91;
  92: string var92;
  93: i32 var93;
  94: i64 var94;
  95: bool var95;
  96: string var96;
  97: i32 var97;
  98: i64 var98;
  99: bool var99;
  100: string var100;
}

@cpp.UseOpEncode
struct OpMixedInt {
  1: i32 var1;
  2: i32 var2;
  3: i32 var3;
  4: i32 var4;
  5: i32 var5;
  6: i32 var6;
  7: i32 var7;
  8: i32 var8;
  9: i32 var9;
  10: i32 varx;
  11: i32 vary;
  12: i32 varz;
}

@cpp.UseOpEncode
struct OpBigListMixedInt {
  1: list<OpMixedInt> lst;
}

@cpp.UseOpEncode
struct OpComplexStruct {
  1: OpEmpty var1;
  2: OpSmallInt var2;
  4: OpBigInt var3;
  7: OpSmallString var4;
  11: OpBigString var5;
  16: OpMixed var6;
  22: OpSmallListInt var7;
  29: OpBigListInt var8;
  37: OpLargeListMixed var9;
  46: OpLargeMapInt var10;
  56: OpLargeMixed var11;
  67: OpNestedMap var12;
}

@cpp.UseOpEncode
union OpComplexUnion {
  1: OpEmpty var1;
  2: OpSmallInt var2;
  4: OpBigInt var3;
  7: OpSmallString var4;
  11: OpBigString var5;
  16: OpMixed var6;
  22: OpSmallListInt var7;
  29: OpBigListInt var8;
  37: OpLargeListMixed var9;
  46: OpLargeMapInt var10;
  56: OpLargeMixed var11;
  67: OpNestedMap var12;
}
