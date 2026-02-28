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

namespace java.swift com.facebook.mojo.example

include "thrift/lib/java/fbthrift-maven-plugin/fbthrift-maven-plugin-example/Example2.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct Example1 {
  1: optional i32 bar;
  2: optional string baz;
}

struct TestsTransitiveThriftFileInclusion {
  1: optional Example2.Example2 fieldWithTypeFromTransitiveThriftFileInclusion;
}
