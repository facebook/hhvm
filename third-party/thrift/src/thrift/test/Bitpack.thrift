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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct A {
  1: optional bool a;
  2: optional bool b;
  3: optional bool c;
  4: optional bool d;
  5: optional bool e;
  6: optional bool f;
  7: optional bool g;
  8: optional bool h;
}
@cpp.PackIsset
struct A_bitpack {
  1: optional bool a;
  2: optional bool b;
  3: optional bool c;
  4: optional bool d;
  5: optional bool e;
  6: optional bool f;
  7: optional bool g;
  8: optional bool h;
}
@cpp.PackIsset{atomic = true}
struct A_atomic_bitpack {
  1: optional bool a;
  2: optional bool b;
  3: optional bool c;
  4: optional bool d;
  5: optional bool e;
  6: optional bool f;
  7: optional bool g;
  8: optional bool h;
}

struct Extra_unbitpack {
  1: i32 extraInt32Def;
  2: required i32 extraInt32Req;
  3: optional i32 extraInt32Opt;
}
@cpp.PackIsset
struct Extra_bitpack {
  1: i32 extraInt32Def;
  2: required i32 extraInt32Req;
  3: optional i32 extraInt32Opt;
}
@cpp.PackIsset{atomic = true}
struct AtomicExtra_bitpack {
  1: i32 extraInt32Def;
  2: required i32 extraInt32Req;
  3: optional i32 extraInt32Opt;
}

struct Unbitpack {
  1: required i32 int32Req;
  2: optional i32 int32Opt;
  3: required string stringReq;
  4: optional string stringOpt;
  5: required set<i32> setReq;
  6: optional set<i32> setOpt;
  7: required list<i32> listReq;
  8: optional list<i32> listOpt;
  9: optional Extra_unbitpack structOpt;
  10: optional Extra_bitpack structPackedOpt;
  11: optional AtomicExtra_bitpack structAtomicPackedOpt;
}
@cpp.PackIsset
struct Bitpack {
  1: required i32 int32Req;
  2: optional i32 int32Opt;
  3: required string stringReq;
  4: optional string stringOpt;
  5: required set<i32> setReq;
  6: optional set<i32> setOpt;
  7: required list<i32> listReq;
  8: optional list<i32> listOpt;
  9: optional Extra_unbitpack structOpt;
  10: optional Extra_bitpack structPackedOpt;
  11: optional AtomicExtra_bitpack structAtomicPackedOpt;
}

@cpp.PackIsset{atomic = true}
struct AtomicBitpack {
  1: required i32 int32Req;
  2: optional i32 int32Opt;
  3: required string stringReq;
  4: optional string stringOpt;
  5: required set<i32> setReq;
  6: optional set<i32> setOpt;
  7: required list<i32> listReq;
  8: optional list<i32> listOpt;
  9: optional Extra_unbitpack structOpt;
  10: optional Extra_bitpack structPackedOpt;
  11: optional AtomicExtra_bitpack structAtomicPackedOpt;
}
