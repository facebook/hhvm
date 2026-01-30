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

package "apache.org/thrift/test/field_ref_const_propagation"

namespace cpp2 apache.thrift.test

struct Unqualified {
  1: string msg;
}

struct Optional {
  1: optional string msg;
}

struct Required {
  1: required string msg;
}

struct Boxed {
  @thrift.Box
  1: optional string msg;
}

struct TerseWrite {
  @thrift.Experimental
  @thrift.TerseWrite
  1: string msg;
}

union Union {
  1: string msg;
}
