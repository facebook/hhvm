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

from libcpp.memory cimport make_unique, make_shared, unique_ptr, shared_ptr
from libcpp.utility cimport move
from libc.stdint cimport uint16_t

cdef extern from "thrift/lib/py3/test/interactions/interaction_test.h" namespace "::interactions::test::thrift":
    cdef cppclass cCalculatorHandler "::interactions::test::thrift::SemiCalculatorHandler":
        cCalculatorHandler()

cdef extern from "<thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>" namespace "::apache::thrift":
    cdef cppclass cScopedServerInterfaceThread "::apache::thrift::ScopedServerInterfaceThread":
        cScopedServerInterfaceThread(shared_ptr[cCalculatorHandler])
        uint16_t getPort()

cdef class ServerBox:
    cdef unique_ptr[cScopedServerInterfaceThread] _cpp_obj
    @staticmethod
    cdef box(unique_ptr[cScopedServerInterfaceThread]&& server)
    cpdef reset(self)
    cpdef uint16_t getPort(self)
