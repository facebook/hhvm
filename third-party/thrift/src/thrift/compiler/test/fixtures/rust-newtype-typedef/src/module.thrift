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

include "thrift/annotation/rust.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@rust.NewType
@rust.Type{name = "sorted_vector_map::SortedVectorMap"}
typedef map<i32, i32> MapType

@rust.NewType
@rust.Type{name = "smallvec::SmallVec<[u8; 16]>"}
typedef binary BinType

@rust.Type{name = "OrderedFloat<f64>"}
typedef double Double

@rust.NewType
@rust.Type{name = "Bytes"}
typedef binary BytesType

@rust.Ord
struct MyStruct {
  1: MapType the_map;
  2: BinType the_bin;
  3: binary_8247 inline_bin;
  4: BytesType the_bytes;
  5: binary_9564 inline_bytes;
  6: Double floaty;
  7: double_8056 doublefloaty;
}

// The following were automatically generated and may benefit from renaming.
@rust.Type{name = "smallvec::SmallVec<[u8; 32]>"}
typedef binary binary_8247
@rust.Type{name = "Bytes"}
typedef binary binary_9564
@rust.Type{name = "OrderedFloat<f64>"}
typedef double double_8056
