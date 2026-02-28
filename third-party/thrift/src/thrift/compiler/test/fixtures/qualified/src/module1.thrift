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

include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace cpp MODULE1
namespace cpp2 module1
namespace java module1
namespace py module1
namespace java.swift test.fixtures.module1

struct Struct {
  1: i32 first;
  2: string second;
}

enum Enum {
  ONE = 1,
  TWO = 2,
  THREE = 3,
}

const Struct c1 = {"first": 201, "second": "module1_str"};

const list<Enum> e1s = [ONE, THREE];
