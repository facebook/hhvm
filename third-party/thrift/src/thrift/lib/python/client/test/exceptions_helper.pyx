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

import asyncio

from folly cimport cFollyExceptionWrapper
from cpython.ref cimport PyObject
from libcpp.memory cimport unique_ptr
from thrift.python.common cimport RpcOptions

from thrift.python.exceptions cimport cTTransportException, addHandler, removeAllHandlers

cdef object hijack_transport_exception_handler(const cFollyExceptionWrapper& ex, PyObject* user_data):
    cdef const cTTransportException* transport_exception = ex.get_exception[cTTransportException]()
    if transport_exception:
        options = <RpcOptions>user_data
        return HijackTestException(options.timeout if options else 0.0)
    return None


class HijackTestException(Exception):
    def __init__(self, timeout: float):
        self._timeout = timeout

    @property
    def timeout(self) -> float:
        return self._timeout


class HijackTestHelper:
    def __enter__(self):
        addHandler(hijack_transport_exception_handler)
        return self

    def __exit__(self, exc_type, exc_value, exc_tb):
        removeAllHandlers()
