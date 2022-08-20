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

namespace cpp test.regression
namespace cpp2 test.regression
namespace d test.regression
namespace java test.regression
namespace java.swift test.regression
namespace hack test.regression
namespace php test.regression
namespace py3 test.regression

struct template_arguments_struct {
  1: i32 T;
  2: i32 U;
  3: i32 V;
  4: i32 Args;
  5: i32 UArgs;
  6: i32 VArgs;
}

struct template_arguments_union {
  1: i32 T;
  2: i32 U;
  3: i32 V;
  4: i32 Args;
  5: i32 UArgs;
  6: i32 VArgs;
}

enum template_arguments_enum {
  T = 0,
  U = 1,
  V = 2,
  Args = 3,
  UArgs = 4,
  VArgs = 5,
}
