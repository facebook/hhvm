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

namespace cpp MODULE2
namespace cpp2 module2
namespace java module2
namespace py module2
namespace java.swift test.fixtures.module2

include "module0.thrift"
include "module1.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct Struct {
  1: module0.Struct first;
  2: module1.Struct second;
}

struct BigStruct {
  1: Struct s;
  2: i32 id;
}

const Struct c2 = {"first": module0.c0, "second": module1.c1};

const Struct c3 = c2;
const Struct c4 = c3;
