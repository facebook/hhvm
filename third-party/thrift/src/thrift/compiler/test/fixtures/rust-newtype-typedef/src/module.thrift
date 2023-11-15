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

typedef map<i32, i32> MapType (
  rust.newtype,
  rust.type = "sorted_vector_map::SortedVectorMap",
)

typedef binary BinType (
  rust.newtype,
  rust.type = "smallvec::SmallVec<[u8; 16]>",
)

typedef double (rust.type = "OrderedFloat<f64>") Double

typedef binary BytesType (rust.newtype, rust.type = "Bytes")

struct MyStruct {
  1: MapType the_map;
  2: BinType the_bin;
  3: binary_8247 inline_bin;
  4: BytesType the_bytes;
  5: binary_9564 inline_bytes;
  6: Double floaty;
  7: double_8056 doublefloaty;
} (rust.ord)

// The following were automatically generated and may benefit from renaming.
typedef binary (rust.type = "smallvec::SmallVec<[u8; 32]>") binary_8247
typedef binary (rust.type = "Bytes") binary_9564
typedef double (rust.type = "OrderedFloat<f64>") double_8056
