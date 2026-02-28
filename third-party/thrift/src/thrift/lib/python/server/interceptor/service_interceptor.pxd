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

from libcpp.memory cimport shared_ptr
from libcpp.optional cimport optional
from libcpp.string cimport string
from thrift.python.std_libcpp cimport string_view

cdef extern from "thrift/lib/cpp2/server/ServiceInterceptorBase.h" namespace "apache::thrift":
    cdef cppclass ServiceInterceptorBase:
        pass

    cdef cppclass cRequestInfo "apache::thrift::ServiceInterceptorBase::RequestInfo":
        pass

    cdef cppclass cResponseInfo "apache::thrift::ServiceInterceptorBase::ResponseInfo":
        pass

cdef extern from "thrift/lib/cpp2/server/Cpp2ConnContext.h" namespace "apache::thrift":
    cdef cppclass cCpp2ConnContext "apache::thrift::Cpp2ConnContext"
    cdef cppclass cCpp2RequestContext "apache::thrift::Cpp2RequestContext"

cdef extern from "thrift/lib/python/server/interceptor/PythonServiceInterceptor.h":
    cdef cppclass cPythonServiceInterceptor "apache::thrift::python::PythonServiceInterceptor"(ServiceInterceptorBase):
        pass

    cdef cppclass cObservableServiceInterceptor "apache::thrift::python::ObservableServiceInterceptor"(cPythonServiceInterceptor):
        pass


cdef class PythonServiceInterceptor:
    cdef shared_ptr[cPythonServiceInterceptor] _cpp_interceptor

# This is lazy wrapper around ConnectionInfo class; it defines properties to allow python to access fields
cdef class ConnectionInfo:
    cdef cCpp2ConnContext* _cpp_ctx
    # just exposing this to cython until we know if anything interesting for Python handlers
    cdef cCpp2ConnContext* cpp_conn_context(self)

cdef api object make_connection_info(cCpp2ConnContext* cpp_ctx) noexcept

# This is lazy wrapper around RequestInfo class; it defines properties to allow python to access fields
cdef class RequestInfo:
    cdef cCpp2RequestContext* _cpp_ctx
    cdef string_view _service_name
    cdef string_view _defining_service_name
    cdef string_view _method_name 
    # cdef tuple arguments # can be deserialized from IOBuf
    # just exposing this to cython until we know if anything interesting for Python handlers
    cdef cCpp2RequestContext* cpp_request_context(self)

cdef api object make_request_info(
    cCpp2RequestContext* cpp_ctx,
    string_view serviceName,
    string_view definingServiceName,
    string_view methodName,
) noexcept

cdef class ResponseInfo:
    cdef const cCpp2RequestContext* _cpp_ctx
    cdef string_view _service_name
    cdef string_view _defining_service_name
    cdef string_view _method_name 
    cdef optional[string] _error_message
    # just exposing this to cython until we know if anything interesting for Python handlers
    cdef const cCpp2RequestContext* cpp_request_context(self)

cdef api object make_response_info(
    const cCpp2RequestContext* _cpp_ctx,
    string_view serviceName,
    string_view definingServiceName,
    string_view methodName,
    optional[string] errorMessage,
) noexcept


cdef class PyObservableServiceInterceptor(PythonServiceInterceptor):
    cdef _onConnectionCallback
    cdef _onRequestCallback 
    cdef _onResponseCallback 
    cdef _onConnectionClosedCallback


cdef api object call_on_request_callback(interceptor, connection_state, request_info)
#   -  PyObservableServiceInterceptor interceptor
#   -  object connection_state returned by on_connect_callback (may be None)
#   -  RequestInfo request_info 
#   -  object request_state passed to on_response_callback
#   -  exceptions: Python may throw; must check return for nullptr and handle

cdef api object call_on_response_callback(interceptor, request_state, connection_state, response_info)
#   -  PyObservableServiceInterceptor interceptor
#   -  object connection_state returned by on_connect_callback (may be None)
#   -  RequestInfo request_info 
#   -  return: should be None
#   -  exceptions: Python may throw; must check return for nullptr and handle

cdef api object call_on_connect_callback(interceptor, connection_info)
#   -  PyObservableServiceInterceptor interceptor
#   -  ConnectionInfo connection_info is python wrapper around Cpp2ConnContext
#   -  return: object connection_state of implementer's choice; passed to other callbacks
#   -  exceptions: Python may throw; must check return for nullptr and handle

cdef api object call_on_connect_closed_callback(interceptor, connection_state, connection_info) 
#   -  PyObservableServiceInterceptor interceptor
#   -  object connection_state returned by on_connect_callback (may be None)
#   -  ConnectionInfo connection_info is python wrapper around Cpp2ConnContext
#   -  return: should be None
#   -  cpp interface is noexcept; Python exceptions will terminate
