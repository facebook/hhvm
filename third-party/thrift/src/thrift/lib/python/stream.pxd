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

from libcpp.memory cimport unique_ptr

from folly cimport cFollyExecutor
from folly.iobuf cimport cIOBuf
from thrift.py3.stream cimport cClientBufferedStreamWrapper, cClientBufferedStream
from thrift.python.serializer cimport Protocol


ctypedef cClientBufferedStream[cIOBuf] cIOBufClientBufferedStream

ctypedef cClientBufferedStreamWrapper[cIOBuf] cIOBufClientBufferedStreamWrapper


cdef class ClientBufferedStream:
    cdef object _class
    cdef cFollyExecutor* _executor
    cdef Protocol _protocol
    cdef unique_ptr[cIOBufClientBufferedStreamWrapper] _gen
    @staticmethod
    cdef _fbthrift_create(unique_ptr[cIOBufClientBufferedStream] stream, klass, Protocol prot)
