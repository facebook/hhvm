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

from cython.operator cimport dereference as deref
from libcpp.memory cimport static_pointer_cast
from libcpp.memory cimport shared_ptr
from libcpp.utility cimport move as cmove
from libcpp cimport bool as cbool
from folly.executor cimport get_executor
import unittest
import inspect
import functools

cdef cCreateMethodMetadataResult create(string function_name, RpcKind rpc_kind):
    cdef cmap[string, pair[RpcKind, PyObjPtr]] funcs
    cdef cvector[PyObjPtr] lifecycle
    cdef object server = None
    cdef string serviceName

    funcs[<string>function_name] = pair[RpcKind, PyObjPtr](<RpcKind>rpc_kind, <PyObject*>None)

    cdef shared_ptr[cPythonAsyncProcessorFactory] obj = cCreatePythonAsyncProcessorFactory(
        <PyObject*>server,
        cmove(funcs),
        cmove(lifecycle),
        get_executor(),
        serviceName
    )
    return deref(obj).createMethodMetadata()


cdef class PythonAsyncProcessorFactoryCTest:
    def __cinit__(self, object unit_test):
        self.ut = unit_test

    def test_create_method_metadata(self, string function_name, RpcKind rpc_kind, string expected) -> None:
        create_method_metadata_result = create(function_name, rpc_kind)
        actual = cAsyncProcessorFactory.describe(create_method_metadata_result)
        self.ut.maxDiff = None
        self.ut.assertEqual(expected, actual)
