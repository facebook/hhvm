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

from libcpp cimport bool as cbool
from libcpp.memory cimport shared_ptr, unique_ptr
from thrift.python.std_libcpp cimport string_view

cdef extern from "thrift/lib/cpp2/async/AsyncProcessor.h" namespace "apache::thrift":
    cdef cppclass cAsyncProcessor "apache::thrift::AsyncProcessor":
        pass

    cdef cppclass cGeneratedAsyncProcessorBase \
            "apache::thrift::GeneratedAsyncProcessorBase"(cAsyncProcessor):
        string_view getServiceName()

    cdef cppclass cAsyncProcessorFactory "apache::thrift::AsyncProcessorFactory":
        unique_ptr[cAsyncProcessor] getProcessor()

    cdef cppclass cServerInterface \
            "apache::thrift::ServerInterface"(cAsyncProcessorFactory):
        pass

    cdef cGeneratedAsyncProcessorBase* dynamic_cast_gen \
            "dynamic_cast<apache::thrift::GeneratedAsyncProcessorBase*>"(...)

cdef extern from "thrift/lib/cpp2/util/EmptyAsyncProcessor.h":
    # This is a little wonky, but makes using it much easier from cython.
    # without having to use a static_pointer_cast to make cython happy.
    ctypedef cAsyncProcessorFactory EmptyAsyncProcessorFactory "apache::thrift::EmptyAsyncProcessorFactory"

cdef class AsyncProcessorFactory:
    cdef shared_ptr[cAsyncProcessorFactory] _cpp_obj
    cdef cbool requireResourcePools(AsyncProcessorFactory self)
