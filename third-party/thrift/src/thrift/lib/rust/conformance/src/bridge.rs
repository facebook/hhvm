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

#[cxx::bridge(namespace = fbthrift_conformance::rust)]
pub mod ffi {
    #[namespace = "apache::thrift::type"]
    #[derive(Hash)]
    #[repr(i32)]
    enum UniversalHashAlgorithm {
        Sha2_256 = 2,
    }

    unsafe extern "C++" {
        include!("thrift/lib/rust/conformance/include/UniversalName.h");

        #[namespace = "apache::thrift::type"]
        type UniversalHashAlgorithm;

        fn getUniversalHash(
            alg: UniversalHashAlgorithm,
            uri: &CxxString,
        ) -> Result<UniquePtr<CxxString>>;
        fn getUniversalHashPrefix(universalHash: &CxxString, hashBytes: i8)
        -> UniquePtr<CxxString>;
        fn matchesUniversalHash(universalHash: &CxxString, prefix: &CxxString) -> bool;
    }
}
