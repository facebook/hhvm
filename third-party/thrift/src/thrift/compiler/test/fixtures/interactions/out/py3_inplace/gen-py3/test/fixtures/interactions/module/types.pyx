#
# Autogenerated by Thrift for thrift/compiler/test/fixtures/interactions/src/module.thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#
cimport cython as __cython
from cpython.object cimport PyTypeObject
from libcpp.memory cimport shared_ptr, make_shared, unique_ptr
from libcpp.optional cimport optional as __optional
from libcpp.string cimport string
from libcpp cimport bool as cbool
from libcpp.iterator cimport inserter as cinserter
from libcpp.utility cimport move as cmove
from cpython cimport bool as pbool
from cython.operator cimport dereference as deref, preincrement as inc, address as ptr_address
import thrift.py3.types
from thrift.py3.types import _IsSet as _fbthrift_IsSet
from thrift.py3.types cimport make_unique
cimport thrift.py3.types
cimport thrift.py3.exceptions
cimport thrift.python.exceptions
import thrift.python.converter
from thrift.python.types import EnumMeta as __EnumMeta
from thrift.python.std_libcpp cimport sv_to_str as __sv_to_str, string_view as __cstring_view
from thrift.python.types cimport BadEnum as __BadEnum
from thrift.py3.types cimport (
    richcmp as __richcmp,
    init_unicode_from_cpp as __init_unicode_from_cpp,
    set_iter as __set_iter,
    map_iter as __map_iter,
    reference_shared_ptr as __reference_shared_ptr,
    get_field_name_by_index as __get_field_name_by_index,
    reset_field as __reset_field,
    translate_cpp_enum_to_python,
    const_pointer_cast,
    make_const_shared,
    constant_shared_ptr,
    deref_const as __deref_const,
    mixin_deprecation_log_error,
)
from thrift.py3.types cimport _ensure_py3_or_raise, _ensure_py3_container_or_raise
cimport thrift.py3.serializer as serializer
from thrift.python.protocol cimport Protocol as __Protocol
import folly.iobuf as _fbthrift_iobuf
from folly.optional cimport cOptional
from folly.memory cimport to_shared_ptr as __to_shared_ptr
from folly.range cimport Range as __cRange

import sys
from collections.abc import Sequence, Set, Mapping, Iterable
import weakref as __weakref
import builtins as _builtins
import importlib
import asyncio
from folly.coro cimport bridgeCoroTaskWith
cimport test.fixtures.another_interactions.shared.types as _test_fixtures_another_interactions_shared_types
import test.fixtures.another_interactions.shared.types as _test_fixtures_another_interactions_shared_types
cimport test.fixtures.another_interactions.shared.thrift_converter as _test_fixtures_another_interactions_shared_thrift_converter


cimport test.fixtures.interactions.module.thrift_converter as _test_fixtures_interactions_module_thrift_converter

import test.fixtures.interactions.module.types_inplace_FBTHRIFT_ONLY_DO_NOT_USE as _fbthrift_types_inplace
CustomException = _fbthrift_types_inplace.CustomException
ShouldBeBoxed = _fbthrift_types_inplace.ShouldBeBoxed


cdef class ClientBufferedStream__bool(ClientBufferedStream):

    @staticmethod
    cdef _fbthrift_create(cClientBufferedStream[cbool]& c_obj, __RpcOptions rpc_options):
        __fbthrift_inst = ClientBufferedStream__bool(rpc_options)
        __fbthrift_inst._gen = make_unique[cClientBufferedStreamWrapper[cbool]](c_obj)
        return __fbthrift_inst

    @staticmethod
    cdef void callback(
        cFollyTry[__cOptional[cbool]]&& result,
        PyObject* userdata,
    ) noexcept:
        cdef __cOptional[cbool] opt_val
        cdef cbool _value
        stream, pyfuture, rpc_options = <object> userdata
        if result.hasException():
            pyfuture.set_exception(
                thrift.python.exceptions.create_py_exception(result.exception(), <__RpcOptions>rpc_options)
            )
        else:
            opt_val = result.value()
            if opt_val.has_value():
                try:
                    _value = opt_val.value()
                    pyfuture.set_result(<bint>_value)
                except Exception as ex:
                    pyfuture.set_exception(ex.with_traceback(None))
            else:
                pyfuture.set_exception(StopAsyncIteration())

    def __anext__(self):
        __loop = asyncio.get_event_loop()
        __future = __loop.create_future()
        # to avoid "Future exception was never retrieved" error at SIGINT
        __future.add_done_callback(lambda x: x.exception())
        __userdata = (self, __future, self._rpc_options)
        bridgeCoroTaskWith[__cOptional[cbool]](
            self._executor,
            deref(self._gen).getNext(),
            ClientBufferedStream__bool.callback,
            <PyObject *>__userdata,
        )
        return asyncio.shield(__future)

cdef class ServerStream__bool(ServerStream):
    pass

cdef class ClientBufferedStream__i32(ClientBufferedStream):

    @staticmethod
    cdef _fbthrift_create(cClientBufferedStream[cint32_t]& c_obj, __RpcOptions rpc_options):
        __fbthrift_inst = ClientBufferedStream__i32(rpc_options)
        __fbthrift_inst._gen = make_unique[cClientBufferedStreamWrapper[cint32_t]](c_obj)
        return __fbthrift_inst

    @staticmethod
    cdef void callback(
        cFollyTry[__cOptional[cint32_t]]&& result,
        PyObject* userdata,
    ) noexcept:
        cdef __cOptional[cint32_t] opt_val
        cdef cint32_t _value
        stream, pyfuture, rpc_options = <object> userdata
        if result.hasException():
            pyfuture.set_exception(
                thrift.python.exceptions.create_py_exception(result.exception(), <__RpcOptions>rpc_options)
            )
        else:
            opt_val = result.value()
            if opt_val.has_value():
                try:
                    _value = opt_val.value()
                    pyfuture.set_result(_value)
                except Exception as ex:
                    pyfuture.set_exception(ex.with_traceback(None))
            else:
                pyfuture.set_exception(StopAsyncIteration())

    def __anext__(self):
        __loop = asyncio.get_event_loop()
        __future = __loop.create_future()
        # to avoid "Future exception was never retrieved" error at SIGINT
        __future.add_done_callback(lambda x: x.exception())
        __userdata = (self, __future, self._rpc_options)
        bridgeCoroTaskWith[__cOptional[cint32_t]](
            self._executor,
            deref(self._gen).getNext(),
            ClientBufferedStream__i32.callback,
            <PyObject *>__userdata,
        )
        return asyncio.shield(__future)

cdef class ServerStream__i32(ServerStream):
    pass

cdef class ResponseAndClientBufferedStream__i32_i32(ResponseAndClientBufferedStream):

    @staticmethod
    cdef _fbthrift_create(cResponseAndClientBufferedStream[cint32_t, cint32_t]& c_obj, __RpcOptions rpc_options):
        __fbthrift_inst = ResponseAndClientBufferedStream__i32_i32()
        __fbthrift_inst._stream = ClientBufferedStream__i32._fbthrift_create(
            c_obj.stream, rpc_options,
        )
        cdef cint32_t _value = c_obj.response
        __fbthrift_inst._response = _value
        return __fbthrift_inst

    def __iter__(self):
        yield self._response
        yield self._stream

cdef class ResponseAndServerStream__i32_i32(ResponseAndServerStream):
    pass

