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

from cpython cimport PyObject
from libcpp.optional cimport optional
from libcpp.memory cimport make_shared
from libcpp.string cimport string
from thrift.python.std_libcpp cimport string_view, sv_to_str
from libcpp.utility cimport move as cmove
from typing import Generic, TypeVar

ConnectionState = TypeVar("ConnectionState")
RequestState = TypeVar("RequestState")

# this is a stub class used to define interface that user must implement
class AbstractServiceInterceptor(Generic[ConnectionState, RequestState]):
    def __init__(self):
        raise RuntimeError("AbstractServiceInterceptor not instantiable from Python")



cdef class PyObservableServiceInterceptor(PythonServiceInterceptor):
    def _noop_callback(self, *_args):
        pass

    def __cinit__(self, python_interceptor):
        cdef string cls_name = python_interceptor.__class__.__name__.encode()
        self._cpp_interceptor = make_shared[cObservableServiceInterceptor](
            <PyObject*>self,
            cmove(cls_name),
        )

    def __init__(self, python_interceptor):
        if isinstance(python_interceptor, type):
            kls_name = python_interceptor.__name__
            raise TypeError(f"Got class `{kls_name}` instead of instance of `{kls_name}`")
        # bind callbacks to methods of python_interceptor object
        # and replace missing methods with no-op
        self._onRequestCallback = getattr(
            python_interceptor,
            "onRequest", 
            self._noop_callback
        )
        self._onResponseCallback = getattr(
            python_interceptor,
            "onResponse",
            self._noop_callback
        )
        self._onConnectionCallback = getattr(
            python_interceptor,
            "onConnection",
            self._noop_callback
        )
        self._onConnectionClosedCallback = getattr(
            python_interceptor,
            "onConnectionClosed",
            self._noop_callback
        )


    
cdef api object call_on_request_callback(
    interceptor,
    connection_state,
    request_info
):
    # this should be async if we want to productionize
    return (<PyObservableServiceInterceptor?>interceptor)._onRequestCallback(
        connection_state,
        request_info,
    )

cdef api object call_on_response_callback(
    interceptor,
    request_state,
    connection_state,
    response_info,
):
    # this should be async if we want to productionize
    return (<PyObservableServiceInterceptor?>interceptor)._onResponseCallback(
        request_state,
        connection_state,
        response_info,
    )

cdef api object call_on_connect_callback(interceptor, connect_info):
    return (<PyObservableServiceInterceptor?>interceptor)._onConnectionCallback(
        connect_info
    )


cdef api object call_on_connect_closed_callback(
    interceptor,
    connection_state,
    connect_closed_info
):
    return (<PyObservableServiceInterceptor?>interceptor)._onConnectionClosedCallback(
        connection_state,
        connect_closed_info
    )


cdef class ConnectionInfo:
    def __init__(self):
        raise RuntimeError("ConnectionInfo not instantiable from Python")

    # not yet binding this to Python until we know what's interesting
    # could be consumed by customer's own cython bindings
    cdef cCpp2ConnContext* cpp_conn_context(ConnectionInfo self):
        return self._cpp_ctx

cdef api object make_connection_info(cCpp2ConnContext* cpp_ctx) noexcept:
    cdef ConnectionInfo inst = ConnectionInfo.__new__(ConnectionInfo)
    inst._cpp_ctx = cpp_ctx
    return inst

# This is lazy wrapper around RequestInfo class; it defines properties to allow python to access fields
cdef class RequestInfo:
    def __init__(self):
        raise RuntimeError("RequestInfo not instantiable from Python")

    @property
    def service_name(self):
        if self._service_name.empty():
            return None
        cdef string_view s = self._service_name
        return sv_to_str(s)

    @property
    def defining_service_name(self):
        if self._defining_service_name.empty():
            return None
        cdef string_view s = self._defining_service_name
        return sv_to_str(s)

    @property
    def method_name(self):
        if self._method_name.empty():
            return None
        cdef string_view s = self._method_name
        return sv_to_str(s)

    cdef cCpp2RequestContext* cpp_request_context(self):
        return self._cpp_ctx


cdef api object make_request_info(
    cCpp2RequestContext* cpp_ctx,
    string_view serviceName,
    string_view definingServiceName,
    string_view methodName,
) noexcept:
    cdef RequestInfo inst = RequestInfo.__new__(RequestInfo)
    inst._cpp_ctx = cpp_ctx
    inst._service_name = serviceName
    inst._defining_service_name = definingServiceName
    inst._method_name = methodName
    return inst

# This is lazy wrapper around RequestInfo class; it defines properties to allow python to access fields
cdef class ResponseInfo:
    def __init__(self):
        raise RuntimeError("ReponseInfo not instantiable from Python")

    @property
    def service_name(self):
        if self._service_name.empty():
            return None
        cdef string_view s = self._service_name
        return sv_to_str(s)

    @property
    def defining_service_name(self):
        if self._defining_service_name.empty():
            return None
        cdef string_view s = self._defining_service_name
        return sv_to_str(s)

    @property
    def method_name(self):
        if self._method_name.empty():
            return None
        cdef string_view s = self._method_name
        return sv_to_str(s)

    @property
    def exception(self):
        if not self._error_message.has_value():
            return None
        return (<bytes><string>self._error_message.value()).decode()

    cdef const cCpp2RequestContext* cpp_request_context(self):
        return self._cpp_ctx


cdef api object make_response_info(
    const cCpp2RequestContext* cpp_ctx,
    string_view serviceName,
    string_view definingServiceName,
    string_view methodName,
    optional[string] errorMessage,
) noexcept:
    cdef ResponseInfo inst = ResponseInfo.__new__(ResponseInfo)
    inst._cpp_ctx = cpp_ctx
    inst._service_name = serviceName
    inst._defining_service_name = definingServiceName
    inst._method_name = methodName
    inst._error_message = cmove(errorMessage)
    return inst
