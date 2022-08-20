# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from libc.stdint cimport int8_t
from libcpp cimport bool
from libcpp.string cimport string

from folly.fbstring cimport fbstring
from folly.range cimport StringPiece

ctypedef int8_t hash_size_t

cdef extern from "thrift/lib/cpp2/type/UniversalHashAlgorithm.h" namespace "apache::thrift::type":
    cpdef enum class UniversalHashAlgorithm:
        Sha2_256

cdef extern from "thrift/lib/cpp2/type/UniversalName.h" namespace "apache::thrift::type":
    cdef void validateUniversalName(string uri) except +
    cdef void validateUniversalHash(UniversalHashAlgorithm alg, string universalHash, hash_size_t minHashBytes) except +
    cdef void validateUniversalHashBytes(hash_size_t hashBytes, hash_size_t minHashBytes) except +
    cdef hash_size_t getUniversalHashSize(UniversalHashAlgorithm alg)
    cdef fbstring getUniversalHash(UniversalHashAlgorithm alg, string uri)
    cdef StringPiece getUniversalHashPrefix(string universalHash, hash_size_t hashBytes)
    cdef fbstring maybeGetUniversalHashPrefix(UniversalHashAlgorithm alg, string uri, hash_size_t hashBytes)
    cdef bool matchesUniversalHash(string universalHash, string prefix)
