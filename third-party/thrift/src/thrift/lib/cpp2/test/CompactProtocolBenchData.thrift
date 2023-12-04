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

struct Empty {}

struct Shallow {
  1: i64 var1;
  2: i64 var2;
  3: i64 var3;
  4: i64 var4;
  5: i64 var5;
  6: i64 var6;
  7: i64 var7;
  8: i64 var8;
  9: i64 var9;
  10: i64 var10;
  11: i64 var11;
  12: i64 var12;
  13: i64 var13;
  14: i64 var14;
  15: i64 var15;
  16: i64 var16;
  17: i64 var17;
  18: i64 var18;
  19: i64 var19;
  20: i64 var20;
}

struct Deep2 {
  1: list<string> datas;
}

struct Deep1 {
  1: list<Deep2> deeps;
}

struct Deep {
  1: list<Deep1> deeps;
}
