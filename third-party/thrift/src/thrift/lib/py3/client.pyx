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

import sys
cimport cython
from thrift.py3.exceptions cimport create_py_exception
from thrift.py3.common import Protocol
cimport thrift.py3.ssl as thrift_ssl
from libcpp.string cimport string
from libc.stdint cimport uint64_t
from cython.operator cimport dereference as deref
from folly.futures cimport bridgeFutureWith
from folly cimport cFollyTry, cFollyPromise, cFollyUnit
from folly.executor cimport get_executor
import folly.executor
from cpython.ref cimport PyObject
from libcpp cimport nullptr
import asyncio
import ipaddress
import os
from socket import SocketKind

cdef object proxy_factory = None


cpdef object get_proxy_factory():
    return proxy_factory


def install_proxy_factory(factory):
    global proxy_factory
    proxy_factory = factory


@cython.auto_pickle(False)
cdef class Client:
    """
    Base class for all thrift clients
    """
    def __cinit__(Client self):
        loop = asyncio.get_event_loop()
        self._executor = get_executor()
        # Keep a reference to the executor for the life of the client
        self._executor_wrapper = folly.executor.loop_to_q[loop]
        self._deferred_headers = {}
        self._connect_future = loop.create_future()

    cdef const type_info* _typeid(self):
        return NULL

    cdef bind_client(Client self, cRequestChannel_ptr&& channel):
        destroyInEventBaseThread(move(channel))

    def set_persistent_header(Client self, str key, str value):
        if not self._client:
            self._deferred_headers[key] = value
            return

        cdef string ckey = <bytes> key.encode('utf-8')
        cdef string cvalue = <bytes> value.encode('utf-8')
        deref(self._client).setPersistentHeader(ckey, cvalue)

    cdef add_event_handler(Client self, const shared_ptr[cTProcessorEventHandler]& handler):
        if not self._client:
            self._deferred_event_handlers.push_back(handler)
            return
        deref(self._client).addEventHandler(handler)

    def __dealloc__(Client self):
        if self._connect_future and self._connect_future.done() and not self._connect_future.exception():
            print(f'thrift-py3 client: {self!r} was not cleaned up, use the async context manager', file=sys.stderr)
        self._client.reset()

    def __enter__(Client self):
        raise asyncio.InvalidStateError('Use an async context for thrift clients and interactions')

    def __exit__(Client self):
        raise NotImplementedError()

    async def __aenter__(Client self):
        await asyncio.shield(self._connect_future)
        if self._context_entered:
            raise asyncio.InvalidStateError('Client context has been used already')
        self._context_entered = True
        for key, value in self._deferred_headers.items():
            self.set_persistent_header(key, value)
        self._deferred_headers = None
        for handler in self._deferred_event_handlers:
            self.add_event_handler(handler)
        self._deferred_event_handlers.clear()

        return self

    async def __aexit__(Client self, *exc):
        self._check_connect_future()
        self._client.reset()
        aexit_callback = self._aexit_callback
        if aexit_callback:
            aexit_callback()
        # To break any future usage of this client
        badfuture = asyncio.get_event_loop().create_future()
        badfuture.set_exception(asyncio.InvalidStateError('Client Out of Context'))
        badfuture.exception()
        self._connect_future = badfuture

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__():
        raise NotImplementedError()


async def _no_op():
    pass


@cython.auto_pickle(False)
cdef class _AsyncResolveCtxManager:
    """This class just handles resolving of hostnames passed to get_client
       by creating a wrapping async context manager"""
    cdef object clientKlass
    cdef object kws
    cdef object ctx

    def __init__(self, clientKlass, *, **kws):
        self.clientKlass = clientKlass
        self.kws = kws

    async def __aenter__(self):
        loop = asyncio.get_event_loop()
        result = await loop.getaddrinfo(
            self.kws['host'],
            self.kws['port'],
            type=SocketKind.SOCK_STREAM
        )
        self.kws['host'] = result[0][4][0]
        self.ctx = get_client(self.clientKlass, **self.kws)
        return await self.ctx.__aenter__()

    def __aexit__(self, *exc_info):
        if self.ctx:
            awaitable = self.ctx.__aexit__(*exc_info)
            self.ctx = None
            return awaitable
        return _no_op()


def get_client(
    clientKlass,
    *,
    host='::1',
    int port=-1,
    path=None,
    double timeout=1,
    headers=None,
    ClientType client_type = ClientType.THRIFT_HEADER_CLIENT_TYPE,
    protocol = Protocol.COMPACT,
    thrift_ssl.SSLContext ssl_context=None,
    double ssl_timeout=1
):
    if not isinstance(protocol, Protocol):
        raise TypeError(f'protocol={protocol} is not a valid {Protocol}')
    loop = asyncio.get_event_loop()
    # This is to prevent calling get_client at import time at module scope
    assert loop.is_running(), "Eventloop is not running"
    assert issubclass(clientKlass, Client), "Must be a py3 thrift client"

    cdef uint32_t _timeout_ms = int(timeout * 1000)
    cdef uint32_t _ssl_timeout_ms = int(ssl_timeout * 1000)
    cdef string cstr

    endpoint = b''
    if client_type is ClientType.THRIFT_HTTP_CLIENT_TYPE:
        if path is None:
            raise TypeError("use path='/endpoint' when using ClientType.THRIFT_HTTP_CLIENT_TYPE")
        endpoint = os.fsencode(path)  # means we can accept bytes/str/Path objects
        path = None

    if port == -1 and path is None:
        raise ValueError('path or port must be set')

    # See if what we were given is an ip or hostname, if not an ip return a resolver
    if isinstance(host, str) and host != "::1" and not path:
        try:
            ipaddress.ip_address(host)
        except ValueError:
            return _AsyncResolveCtxManager(
                clientKlass,
                host=host,
                port=port,
                path=endpoint,
                timeout=timeout,
                headers=headers,
                client_type=client_type,
                protocol=protocol,
                ssl_context=ssl_context,
                ssl_timeout=ssl_timeout
            )

    host = str(host)  # Accept ipaddress objects
    client = clientKlass()

    if path:
        fspath = os.fsencode(path)
        bridgeFutureWith[cRequestChannel_ptr](
            (<Client>client)._executor,
            createThriftChannelUnix(move[string](fspath), _timeout_ms, client_type, protocol),
            requestchannel_callback,
            <PyObject *> client
        )
    elif ssl_context:
        cstr = <bytes> host.encode('utf-8')
        bridgeFutureWith[cRequestChannel_ptr](
            (<Client>client)._executor,
            thrift_ssl.createThriftChannelTCP(
                ssl_context._cpp_obj,
                move[string](cstr),
                port,
                _timeout_ms,
                _ssl_timeout_ms,
                client_type,
                protocol,
                move[string](endpoint)
            ),
            requestchannel_callback,
            <PyObject *> client
        )
    else:
        cstr = <bytes> host.encode('utf-8')
        bridgeFutureWith[cRequestChannel_ptr](
            (<Client>client)._executor,
            createThriftChannelTCP(
                move[string](cstr),
                port,
                _timeout_ms,
                client_type,
                protocol,
                move[string](endpoint)
            ),
            requestchannel_callback,
            <PyObject *> client
        )
    if headers:
        for key, value in headers.items():
            client.set_persistent_header(key, value)

    factory = get_proxy_factory()
    proxy = factory(clientKlass) if factory else None
    return proxy(client) if proxy else client

cdef void interactions_callback(
    cFollyTry[unique_ptr[cClientWrapper]]&& result,
    PyObject* userData,
) noexcept:
    cdef Client client = <object> userData
    future = client._connect_future
    if result.hasException():
        future.set_exception(create_py_exception(result.exception(), None))
    else:
        client._client = move(result.value())
        future.set_result(None)

cdef void requestchannel_callback(
    cFollyTry[cRequestChannel_ptr]&& result,
    PyObject* userData,
) noexcept:
    cdef Client client = <object> userData
    future = client._connect_future
    if result.hasException():
        future.set_exception(create_py_exception(result.exception(), None))
    else:
        client.bind_client(move(result.value()))
        future.set_result(None)
