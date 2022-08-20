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

struct DefaultValues {
  1: i32 def_reg;
  2: i32 def_val = 12;
  3: required i32 req_reg;
  4: required i32 req_val = 34;
  5: optional i32 opt_reg;
  6: optional i32 opt_val = 56;
  7: optional list<i32> opt_list_reg;
  8: optional list<i32> opt_list_val = [56, 78, 90];
}
