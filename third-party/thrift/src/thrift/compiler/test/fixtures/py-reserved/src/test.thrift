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

struct def {
  1: i64 from;
  2: string in;
  3: i32 as;
  4: bool if;
  5: list<i32> else;
  6: i32 try;
  7: i32 while;
  8: bool yield;
  9: bool break;
  10: bool await;
  11: bool return;
}

service lambda {
  bool global(1: i64 raise);
  def import();
}
