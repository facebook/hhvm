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
from libcpp.string cimport string
from thrift.py3.std_libcpp cimport string_view
from thrift.python.server_impl.request_context cimport (
    ConnectionContext,
    RequestContext,
    Cpp2RequestContext,
    Cpp2ConnContext
)

cdef extern from "folly/Expected.h" namespace "folly":
    cdef cppclass Expected[T, E]:
        T value() except+
        E error() except+
        bint hasValue() except+
        bint hasError() except+

ctypedef Cpp2RequestContext* Cpp2RequestContextPtr
ctypedef Cpp2ConnContext* Cpp2ConnContextPtr

cdef extern from "thrift/lib/python/server/RequestContextExtractor.h" namespace "apache::thrift::python":
    cdef cppclass RequestContextExtractor:
        @staticmethod
        Expected[Cpp2RequestContextPtr, string_view] extractCppRequestContext(PyObject* obj) except *

        @staticmethod
        Expected[Cpp2ConnContextPtr, string_view] extractCppConnectionContext(PyObject* obj) except *

cdef class PyRequestContextExtractor:

    @staticmethod
    def extract_cpp_request_context(request_context):
        """
        Extract the underlying C++ request context from a Python RequestContext object.

        Args:
            request_context: RequestContext object to extract from

        Raises:
            TypeError: If PyObject* is not a valid RequestContext object
        """
        RequestContextExtractor.extractCppRequestContext(<PyObject*>request_context)

    @staticmethod
    def extract_cpp_connection_context(connection_context):
        """
        Extract the underlying C++ connection context from a Python ConnectionContext object.

        Args:
            connection_context: ConnectionContext object to extract from

        Raises:
            TypeError: If PyObject* is not a valid ConnectionContext object
        """
        RequestContextExtractor.extractCppConnectionContext(<PyObject*>connection_context)
