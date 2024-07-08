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

#[cxx::bridge(namespace = "thrift::rust::thrift_any")]
pub mod ffi {
    unsafe extern "C++" {
        include!("thrift/lib/rust/any/tests/src/cpp.h");

        // compact serialized Basic to compact serialized Any
        fn basic_to_any(basic: &CxxString) -> UniquePtr<CxxString>;
        // compact serialized Any to compact serialized Basic
        fn any_to_basic(any: &CxxString) -> UniquePtr<CxxString>;

        // compact serialized SimpleUnion to compact serialized Any
        fn simple_union_to_any(simple_union: &CxxString) -> UniquePtr<CxxString>;
        // compact serialized Any to compact serialized SimpleUnion
        fn any_to_simple_union(any: &CxxString) -> UniquePtr<CxxString>;

        fn compress_any(basic: &CxxString) -> UniquePtr<CxxString>;
        fn decompress_any(basic: &CxxString) -> UniquePtr<CxxString>;
    }
}
