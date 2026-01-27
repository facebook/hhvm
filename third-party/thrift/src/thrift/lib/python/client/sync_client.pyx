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

# cython: c_string_type=unicode, c_string_encoding=utf8


from cython.operator cimport dereference as deref
from folly cimport cFollyTry
from folly.iobuf cimport IOBuf, cIOBuf, from_unique_ptr
from libc.stdint cimport uint16_t
from libcpp.memory cimport make_unique
from libcpp.string cimport string
from libcpp.utility cimport move as cmove
from thrift.python.client.request_channel cimport RequestChannel
from thrift.python.exceptions cimport create_py_exception
from thrift.python.exceptions import ApplicationError, ApplicationErrorType
from thrift.python.serializer import serialize_iobuf, deserialize
from thrift.python.mutable_serializer import (
    serialize_iobuf as serialize_iobuf_mutable,
    deserialize as deserialize_mutable,
)
from thrift.python.common cimport cRpcOptions, RpcOptions

cdef string blank_interaction = b""

cdef class SyncClient:
    def __init__(SyncClient self, RequestChannel channel):
        self._omni_client = make_unique[cOmniClient](cmove(channel._cpp_obj))
        self._exit_callbacks = []

    def __enter__(SyncClient self):
        if not self._omni_client:
            raise RuntimeError("Connection already closed")
        return self

    def __exit__(SyncClient self, exec_type, exc_value, traceback):
        try:
            for callback in self._exit_callbacks:
                try:
                    callback()
                except Exception:
                    pass
        finally:
            self._omni_client.reset()

    def clear_event_handlers(SyncClient self):
        if not self._omni_client:
            raise RuntimeError("Connection already closed")
        deref(self._omni_client).clearEventHandlers()

    def _send_request(
        SyncClient self,
        string service_name,
        string function_name,
        args,
        response_cls,
        *,
        string uri_or_name = b"",
        RpcOptions rpc_options = None,
        is_mutable_types = False,
    ):
        if not self._omni_client:
            raise RuntimeError("Connection already closed")

        cdef cFollyTry[uint16_t] protocol_try = deref(self._omni_client).getChannelProtocolId()
        if protocol_try.hasException():
            raise create_py_exception(protocol_try.exception(), rpc_options)
        cdef uint16_t protocol = protocol_try.value()
        cdef IOBuf args_iobuf = (
            serialize_iobuf(args, protocol=protocol)
            if not is_mutable_types
            else serialize_iobuf_mutable(args, protocol=protocol)
        )

        cdef unique_ptr[cIOBuf] args_ciobuf = cmove(args_iobuf.c_clone())
        cdef cRpcOptions c_rpc_options
        if rpc_options is not None:
            c_rpc_options = rpc_options._cpp_obj

        if response_cls is None:
            with nogil:
                deref(self._omni_client).oneway_send(
                    service_name,
                    function_name,
                    cmove(args_ciobuf),
                    cmove(cData(function_name, FunctionQualifier.OneWay, uri_or_name, InteractionMethodPosition.None, blank_interaction)),
                    self._persistent_headers,
                    cmove(c_rpc_options),
                )
        else:
            with nogil:
                resp = deref(self._omni_client).sync_send(
                    service_name,
                    function_name,
                    cmove(args_ciobuf),
                    cmove(cData(function_name, FunctionQualifier.Unspecified, uri_or_name, InteractionMethodPosition.None, blank_interaction)),
                    self._persistent_headers,
                    cmove(c_rpc_options),
                )
            if resp.buf.hasValue():
                response_iobuf = from_unique_ptr(cmove(resp.buf.value()))
                return (
                    deserialize(response_cls, response_iobuf, protocol=protocol)
                    if not is_mutable_types
                    else deserialize_mutable(response_cls, response_iobuf, protocol=protocol)
                )

            elif resp.buf.hasError():
                raise create_py_exception(resp.buf.error(), rpc_options)
            else:
                raise ApplicationError(
                    ApplicationErrorType.MISSING_RESULT,
                    "Received no result nor error",
                )

    def set_persistent_header(SyncClient self, string key, string value):
        self._persistent_headers[key] = value

    cdef add_event_handler(SyncClient self, const shared_ptr[cTProcessorEventHandler]& handler):
        if not self._omni_client:
            raise RuntimeError("Connection already closed")
        deref(self._omni_client).addEventHandler(handler)

    def _at_exit(SyncClient self, callback):
        self._exit_callbacks.append(callback)
