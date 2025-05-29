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

from libcpp.memory cimport make_unique, shared_ptr, static_pointer_cast
from libcpp.string cimport string
from libcpp.utility cimport move as cmove

from thrift.python.server_impl.interceptor.service_interceptor cimport (
    cPythonServiceInterceptor,
    PythonServiceInterceptor,
)

cdef class PythonServerModule: 
    def __init__(self, str name):
        self._cpp_module = make_unique[cPythonServerModule](<string>name.encode("utf-8"))

    def add_service_interceptor(self, PythonServiceInterceptor interceptor):
        cdef shared_ptr[ServiceInterceptorBase] ptr = \
            static_pointer_cast[ServiceInterceptorBase, cPythonServiceInterceptor](interceptor._cpp_interceptor)
        self._cpp_module.get().addServiceInterceptor(cmove(ptr))
