/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

enum HasOptionalsTestEnum {
  FOO = 1,
  BAR = 2,
  BAZ = 3,
}

struct HasOptionalsExtra {
  1: i64 extraInt64Def;
  2: required i64 extraInt64Req;
  3: optional i64 extraInt64Opt;
  4: string extraStringDef;
  5: required string extraStringReq;
  6: optional string extraStringOpt;
  7: set<i64> extraSetDef;
  8: required set<i64> extraSetReq;
  9: optional set<i64> extraSetOpt;
  10: list<i64> extraListDef;
  11: required list<i64> extraListReq;
  12: optional list<i64> extraListOpt;
  13: map<i64, i64> extraMapDef;
  14: required map<i64, i64> extraMapReq;
  15: optional map<i64, i64> extraMapOpt;
  16: HasOptionalsTestEnum extraEnumDef;
  17: required HasOptionalsTestEnum extraEnumReq;
  18: optional HasOptionalsTestEnum extraEnumOpt;
}

struct HasOptionals {
  1: i64 int64Def;
  2: required i64 int64Req;
  3: optional i64 int64Opt;
  4: string stringDef;
  5: required string stringReq;
  6: optional string stringOpt;
  7: set<i64> setDef;
  8: required set<i64> setReq;
  9: optional set<i64> setOpt;
  10: list<i64> listDef;
  11: required list<i64> listReq;
  12: optional list<i64> listOpt;
  13: map<i64, i64> mapDef;
  14: required map<i64, i64> mapReq;
  15: optional map<i64, i64> mapOpt;
  16: HasOptionalsTestEnum enumDef;
  17: required HasOptionalsTestEnum enumReq;
  18: optional HasOptionalsTestEnum enumOpt;
  19: HasOptionalsExtra structDef;
  20: required HasOptionalsExtra structReq;
  21: optional HasOptionalsExtra structOpt;
}
