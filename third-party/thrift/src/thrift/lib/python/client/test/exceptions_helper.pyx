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

from folly cimport cFollyExceptionWrapper
from cpython.ref cimport PyObject
from libcpp.memory cimport unique_ptr

from thrift.python.exceptions cimport cTTransportException, try_make_unique_exception, addHandler, removeAllHandlers

cdef object hijack_transport_exception_handler(const cFollyExceptionWrapper& ex, PyObject* user_data):
      cdef unique_ptr[cTTransportException] te = try_make_unique_exception[cTTransportException](ex)
      if te:
        raise HijackTestException()


class HijackTestException(Exception):
      pass


class HijackTestHelper:
    def __enter__(self):
        addHandler(hijack_transport_exception_handler)
        #handlers.push_back(hijack_transport_exception_handler)
        return self

    def __exit__(self, exc_type, exc_value, exc_tb):
        removeAllHandlers()
