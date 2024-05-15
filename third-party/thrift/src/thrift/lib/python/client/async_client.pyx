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
from thrift.python.client.omni_client cimport cOmniClientResponseWithHeaders, RpcKind, cOmniInteractionClient, createOmniInteractionClient, cData, FunctionQualifier, InteractionMethodPosition
from thrift.python.client.request_channel cimport RequestChannel
from thrift.python.exceptions cimport create_py_exception
from thrift.python.exceptions import ApplicationError, ApplicationErrorType
from thrift.python.serializer import serialize_iobuf, deserialize
from thrift.python.stream cimport ClientBufferedStream
from thrift.py3.common cimport cRpcOptions, RpcOptions

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

    def _set_channel(AsyncClient self, RequestChannel channel):
        self.bind_client(cmove(channel._cpp_obj))
        self._connect_future.set_result(None)

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
        if not issubclass(interactionClass, AsyncClient):
            raise TypeError(f"{interactionClass} is not a thrift python async client class")

        interactionClient = interactionClass()
        bridgeFutureWith[unique_ptr[cOmniInteractionClient]](
            (<AsyncClient> interactionClient)._executor,
            createOmniInteractionClient(deref(self._omni_client).getChannelShared(), methodName),
            _interaction_client_callback,
            <PyObject *> interactionClient,
        )
        return interactionClient

    def clear_event_handlers(AsyncClient self):
        if not self._omni_client:
            raise RuntimeError("Connection already closed")
        deref(self._omni_client).clearEventHandlers()

    async def _send_request(
        AsyncClient self,
        string service_name,
        string function_name,
        args,
        response_cls,
        *,
        FunctionQualifier qualifier = FunctionQualifier.Unspecified,
        InteractionMethodPosition interaction_position = InteractionMethodPosition.None,
        string interaction_name = b"",
        AsyncClient created_interaction = None,
        string uri_or_name = b"",
        RpcOptions rpc_options = None,
    ):
        # Required because the python async model means that we can't have an async function that returns a future
        if interaction_position == InteractionMethodPosition.Factory and created_interaction is not None:
            await asyncio.shield(created_interaction._connect_future)
        return await self._send_request_inner(service_name, function_name, args, response_cls, qualifier, interaction_position, interaction_name, created_interaction, uri_or_name, rpc_options)

    def _send_request_inner(
        AsyncClient self,
        string service_name,
        string function_name,
        args,
        response_cls,
        FunctionQualifier qualifier = FunctionQualifier.Unspecified,
        InteractionMethodPosition interaction_position = InteractionMethodPosition.None,
        string interaction_name = b"",
        AsyncClient created_interaction = None,
        string uri_or_name = b"",
        RpcOptions rpc_options = None,
    ):
        protocol = deref(self._omni_client).getChannelProtocolId()
        cdef IOBuf args_iobuf = serialize_iobuf(args, protocol=protocol)

        loop = asyncio.get_event_loop()
        future = loop.create_future()

        # TODO(ffrancet) clean up this logic when client side RPCOptions get implemented,
        # just call created_interaction.setInteraction on the RPCOptions object
        if interaction_position == InteractionMethodPosition.Factory and created_interaction is not None:
            #await asyncio.shield(created_interaction._connect_future)
            # Without access to the RPCOptions object (yet) the memory management model makes the
            # flow here really awkward since cilent A is making the call, but client B has the interaction
            # data, meaning both need to be accessed at the same time, so just directly passing the pointer
            deref(self._omni_client).set_interaction_factory(created_interaction._omni_client.get())

        cdef cRpcOptions c_rpc_options
        if rpc_options is not None:
            c_rpc_options = rpc_options._cpp_obj

        if response_cls is None:
            deref(self._omni_client).oneway_send(
                service_name,
                function_name,
                args_iobuf.c_clone(),
                cmove(cData(function_name, FunctionQualifier.OneWay, uri_or_name, interaction_position, interaction_name)),
                self._persistent_headers,
                cmove(c_rpc_options),
            )
            future.set_result(None)
            return future
        else:
            userdata = (future, response_cls, protocol, rpc_options)
            rpc_kind = RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE if isinstance(
                response_cls, tuple) else RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE
            bridgeSemiFutureWith[cOmniClientResponseWithHeaders](
                self._executor,
                deref(self._omni_client).semifuture_send(
                    service_name,
                    function_name,
                    args_iobuf.c_clone(),
                    cmove(cData(function_name, qualifier, uri_or_name, interaction_position, interaction_name)),
                    self._persistent_headers,
                    cmove(c_rpc_options),
                    self._executor,
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
) noexcept:
    pyfuture, response_cls, protocol, rpc_options = <object> userdata
    cdef cOmniClientResponseWithHeaders resp = cmove(result.value())

    if resp.buf.hasError():
        pyfuture.set_exception(create_py_exception(resp.buf.error(), rpc_options))
        return

    if rpc_options is not None and not resp.headers.empty():
        (<RpcOptions>rpc_options)._cpp_obj.setReadHeaders(cmove(resp.headers))

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
    PyObject* userData,) noexcept:
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
