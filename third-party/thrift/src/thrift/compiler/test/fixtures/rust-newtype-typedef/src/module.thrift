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

typedef map<i32, i32> MapType (
  rust.newtype,
  rust.type = "sorted_vector_map::SortedVectorMap",
)

typedef binary BinType (
  rust.newtype,
  rust.type = "smallvec::SmallVec<[u8; 16]>",
)

typedef binary BytesType (rust.newtype, rust.type = "Bytes")

struct MyStruct {
  1: MapType the_map;
  2: BinType the_bin;
  3: binary (rust.type = "smallvec::SmallVec<[u8; 32]>") inline_bin;
  4: BytesType the_bytes;
  5: binary (rust.type = "Bytes") inline_bytes;
}
