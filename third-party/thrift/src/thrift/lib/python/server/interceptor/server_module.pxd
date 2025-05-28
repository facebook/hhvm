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


from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr, unique_ptr


cdef extern from "thrift/lib/cpp2/server/ServiceInterceptorBase.h" namespace "apache::thrift":
    cdef cppclass ServiceInterceptorBase:
        pass

cdef extern from "thrift/lib/cpp2/server/ServerModule.h" namespace "apache::thrift":
    cdef cppclass cServerModule "apache::thrift::ServerModule":
        ServerModule() except +
        string getName() const
        vector[shared_ptr[ServiceInterceptorBase]] getServiceInterceptors()

cdef extern from "thrift/lib/python/server/interceptor/PythonServerModule.h":
    cdef cppclass cPythonServerModule "apache::thrift::python::PythonServerModule"(cServerModule):
        PythonServerModule(string name)
        void addServiceInterceptor(shared_ptr[ServiceInterceptorBase])


cdef class PythonServerModule:
    cdef unique_ptr[cPythonServerModule] _cpp_module
