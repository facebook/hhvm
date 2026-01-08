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

namespace cpp2 apache.thrift.conformance
namespace go thrift.conformance.patch_data
namespace php apache_thrift
namespace py thrift.conformance.patch_data
namespace py.asyncio thrift_asyncio.conformance.patch_data
namespace py3 thrift.conformance
namespace java.swift org.apache.thrift.conformance

include "thrift/conformance/if/any.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct PatchOpResponse {
  1: any.Any result;
}

struct PatchOpRequest {
  1: any.Any value;
  2: any.Any patch;
}

struct PatchOpTestCase {
  1: PatchOpRequest request;
  3: any.Any result;
}
