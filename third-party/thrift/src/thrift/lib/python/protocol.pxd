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

# DERPECATED_VERBOSE_JSON IS DEPRECATED AND WILL BE REMOVED AFTER THRIFT-PYTHON UNIFICATION
cdef extern from "<thrift/lib/cpp/protocol/TProtocolTypes.h>" namespace "apache::thrift::protocol":
    cpdef enum Protocol "apache::thrift::protocol::PROTOCOL_TYPES":
        BINARY "apache::thrift::protocol::T_BINARY_PROTOCOL"
        DEPRECATED_VERBOSE_JSON "apache::thrift::protocol::T_JSON_PROTOCOL"
        COMPACT "apache::thrift::protocol::T_COMPACT_PROTOCOL"
        JSON "apache::thrift::protocol::T_SIMPLE_JSON_PROTOCOL"


cdef extern from "thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h" namespace "::apache::thrift":
    cpdef enum class RpcKind:
        SINGLE_REQUEST_SINGLE_RESPONSE = 0
        SINGLE_REQUEST_NO_RESPONSE = 1
        SINGLE_REQUEST_STREAMING_RESPONSE = 4
        SINK = 6
        BIDIRECTIONAL_STREAM = 7
