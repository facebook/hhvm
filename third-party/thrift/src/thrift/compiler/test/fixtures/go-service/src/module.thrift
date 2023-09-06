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

struct GetEntityRequest {
  1: string id;
}

struct GetEntityResponse {
  1: string entity;
}

struct NonComparableStruct {
  1: string foo;
  2: list<string> bar;
  3: map<NonComparableStruct, i64> baz;
}

service GetEntity {
  GetEntityResponse getEntity(1: GetEntityRequest r);

  bool getBool();
  byte getByte();
  i16 getI16();
  i32 getI32();
  i64 getI64();
  double getDouble();
  string getString();
  binary getBinary();
  map<string, string> getMap();
  set<string> getSet();
  list<string> getList();

  // Legacy method with negative parameter tags
  i32 getLegacyStuff(1: i64 numPos, -1: i64 numNeg1, -2: i64 numNeg2);

  i32 getCtxCollision(1: i64 ctx);
  i32 getCtx1Collision(1: i64 ctx, 2: i64 ctx1);
}
