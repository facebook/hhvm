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

import asyncio
import inspect

cimport cython
cimport folly.iobuf
import folly.executor

from cython.operator cimport dereference as deref
from folly.executor cimport get_executor
from folly.futures cimport bridgeSemiFutureWith, bridgeFutureWith
from folly.iobuf cimport IOBuf
from libcpp.memory cimport make_unique, static_pointer_cast
from libcpp.utility cimport move as cmove
from thrift.python.client.omni_client cimport cOmniClientResponseWithHeaders, RpcKind, cOmniInteractionClient, createOmniInteractionClient
from thrift.python.exceptions cimport create_py_exception
from thrift.python.exceptions import ApplicationError, ApplicationErrorType
from thrift.python.serializer import serialize_iobuf, deserialize
from thrift.python.stream cimport ClientBufferedStream

@cython.auto_pickle(False)
cdef class AsyncClient:
    def __cinit__(AsyncClient self):
        loop = asyncio.get_event_loop()
        self._executor = get_executor()
        # Keep a reference to the executor for the life of the client
        self._executor_wrapper = folly.executor.loop_to_q[loop]
        self._connect_future = loop.create_future()

    def __init__(AsyncClient self):
        self._aexit_callbacks = []

    cdef bind_client(self, cRequestChannel_ptr channel):
        self._omni_client = make_unique[cOmniClient](cmove(channel))

    def __enter__(AsyncClient self):
        raise asyncio.InvalidStateError('Use an async context for thrift clients')

    def __exit__(AsyncClient self):
        raise NotImplementedError()

    async def __aenter__(AsyncClient self):
        await asyncio.shield(self._connect_future)
        return self

    async def __aexit__(AsyncClient self, exc_type, exc_value, traceback):
        try:
            for callback in self._aexit_callbacks:
                try:
                    ret = callback()
                    if inspect.isawaitable(ret):
                        await ret
                except Exception:
                    pass
        finally:
            self._omni_client.reset()
            # To break any future usage of this client
            badfuture = asyncio.get_event_loop().create_future()
            badfuture.set_exception(asyncio.InvalidStateError('Client Out of Context'))
            badfuture.exception()
            self._connect_future = badfuture

    def _create_interaction(
        AsyncClient self,
        string methodName,
        interactionClass,
    ):
        # TODO subclass check
        interactionClient = interactionClass()
        bridgeFutureWith[unique_ptr[cOmniInteractionClient]](
            (<AsyncClient> interactionClient)._executor,
            createOmniInteractionClient(deref(self._omni_client).getChannelShared(), methodName),
            _interaction_client_callback,
            <PyObject *> interactionClient,
        )
        return interactionClient

    def _send_request(
        AsyncClient self,
        string service_name,
        string function_name,
        args,
        response_cls,
    ):
        protocol = deref(self._omni_client).getChannelProtocolId()
        cdef IOBuf args_iobuf = serialize_iobuf(args, protocol=protocol)

        loop = asyncio.get_event_loop()
        future = loop.create_future()

        if response_cls is None:
            deref(self._omni_client).oneway_send(
                service_name,
                function_name,
                args_iobuf.c_clone(),
                self._persistent_headers,
            )
            future.set_result(None)
            return future
        else:
            userdata = (future, response_cls, protocol)
            rpc_kind = RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE if isinstance(
                response_cls, tuple) else RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE
            bridgeSemiFutureWith[cOmniClientResponseWithHeaders](
                self._executor,
                deref(self._omni_client).semifuture_send(
                    service_name,
                    function_name,
                    args_iobuf.c_clone(),
                    self._persistent_headers,
                    rpc_kind,
                ),
                _async_client_send_request_callback,
                <PyObject *> userdata,
            )
            return asyncio.shield(future)

    def set_persistent_header(AsyncClient self, string key, string value):
        self._persistent_headers[key] = value

    def _at_aexit(AsyncClient self, callback):
        self._aexit_callbacks.append(callback)


cdef void _async_client_send_request_callback(
    cFollyTry[cOmniClientResponseWithHeaders]&& result,
    PyObject* userdata,
):
    pyfuture, response_cls, protocol = <object> userdata
    cdef cOmniClientResponseWithHeaders resp = cmove(result.value())

    if resp.buf.hasError():
        # TODO: pass a proper RpcOptions value
        pyfuture.set_exception(create_py_exception(resp.buf.error(), None))
        return
    if not resp.buf.hasValue():
        pyfuture.set_exception(ApplicationError(
            ApplicationErrorType.MISSING_RESULT,
            "Received no result nor error",
        ))
        return
    response_iobuf = folly.iobuf.from_unique_ptr(cmove(resp.buf.value()))
    py_stream = None
    if isinstance(response_cls, tuple):
        response_cls, stream_cls = response_cls
        py_stream = ClientBufferedStream._fbthrift_create(
            cmove(resp.stream),
            stream_cls,
            protocol,
        )
    py_resp = deserialize(response_cls, response_iobuf, protocol=protocol)
    pyfuture.set_result(py_resp if py_stream is None else (py_resp, py_stream))

cdef void _interaction_client_callback(cFollyTry[unique_ptr[cOmniInteractionClient]]&& result,
    PyObject* userData,):
    cdef AsyncClient client = <object> userData
    future = client._connect_future
    if result.hasException():
        try:
            result.exception().throw_exception()
        except Exception as pyex:
            future.set_exception(pyex)
    else:
        client._omni_client = unique_ptr[cOmniClient](<cOmniClient*>result.value().release())
        future.set_result(None)
