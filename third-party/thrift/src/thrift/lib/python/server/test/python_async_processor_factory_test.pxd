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

from cpython.ref cimport PyObject
from libcpp cimport bool as cbool
from libcpp.map cimport map as cmap
from libcpp.memory cimport shared_ptr
from libcpp.pair cimport pair
from libcpp.vector cimport vector as cvector
from libcpp.string cimport string

from folly.executor cimport cAsyncioExecutor
from thrift.python.protocol cimport RpcKind
from thrift.python.server_impl.async_processor cimport AsyncProcessorFactory
from thrift.python.server_impl.python_async_processor cimport HandlerFunc

ctypedef PyObject* PyObjPtr

cdef extern from "thrift/lib/cpp2/async/AsyncProcessor.h" namespace "apache::thrift":
    cdef cppclass cCreateMethodMetadataResult "apache::thrift::AsyncProcessorFactory::CreateMethodMetadataResult":
        pass

    cdef cppclass cAsyncProcessorFactory "apache::thrift::AsyncProcessorFactory":
        @staticmethod
        string describe(cCreateMethodMetadataResult result) noexcept

cdef extern from "thrift/lib/python/server/PythonAsyncProcessorFactory.h":
    cdef cppclass cPythonAsyncProcessorFactory "::apache::thrift::python::PythonAsyncProcessorFactory"(cAsyncProcessorFactory):
        cCreateMethodMetadataResult createMethodMetadata() noexcept

    cdef shared_ptr[cPythonAsyncProcessorFactory] \
        cCreatePythonAsyncProcessorFactory "::apache::thrift::python::PythonAsyncProcessorFactory::create"(
            PyObject* server,
            cmap[string, HandlerFunc] funcs,
            cvector[PyObjPtr] lifecycle,
            cAsyncioExecutor* executor,
            string serviceName,
        ) except +

cdef class PythonAsyncProcessorFactoryCTest:
    cdef object ut
