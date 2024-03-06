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
import ipaddress
import os
import socket

import cython
import folly.executor

from folly.futures cimport bridgeFutureWith
from libc.stdint cimport uint32_t
from libcpp.string cimport string
from libcpp.utility cimport move as cmove
from thrift.python.client cimport ssl as thrift_ssl
from thrift.python.client.async_client cimport AsyncClient
from thrift.python.client.client_wrapper import Client
from thrift.python.client.request_channel cimport (
    createThriftChannelTCP,
    createThriftChannelUnix,
    ClientType as cClientType,
)
from thrift.python.client.request_channel import ClientType
from thrift.python.serializer cimport Protocol as cProtocol


cdef object proxy_factory = None


cpdef object get_proxy_factory():
    return proxy_factory


def install_proxy_factory(factory):
    global proxy_factory
    proxy_factory = factory


def get_client(
    clientKlass,
    *,
    host=None,
    port=None,
    path=None,
    double timeout=1,
    cClientType client_type = ClientType.THRIFT_HEADER_CLIENT_TYPE,
    cProtocol protocol = cProtocol.COMPACT,
    thrift_ssl.SSLContext ssl_context=None,
    double ssl_timeout=1,
):
    if not issubclass(clientKlass, Client):
        raise TypeError(f"{clientKlass} is not a thrift python client class")

    endpoint = None
    if client_type in (ClientType.THRIFT_HTTP_CLIENT_TYPE, ClientType.THRIFT_HTTP2_CLIENT_TYPE):
        if host is None or port is None:
            raise ValueError(f"Must set host and port when using HTTP clients")
        if path is None:
            raise ValueError(f"use path='/endpoint' when using HTTP clients")
        endpoint = os.fsencode(path)
        path = None

    cdef uint32_t _timeout_ms = int(timeout * 1000)
    cdef uint32_t _ssl_timeout_ms = int(ssl_timeout * 1000)
    host = str(host)  # Accept ipaddress objects
    client = clientKlass.Async()

    if host is not None and port is not None:
        if path is not None:
            raise ValueError("Can not set path and host/port at same time")

        if isinstance(host, str):
            try:
                ipaddress.ip_address(host)
            except ValueError:
                return _AsyncResolveCtxManager(
                    clientKlass,
                    host=host,
                    port=port,
                    path=endpoint,
                    timeout=timeout,
                    client_type=client_type,
                    protocol=protocol,
                    ssl_context=ssl_context,
                    ssl_timeout=ssl_timeout,
                )
        else:
            host = str(host)

        if endpoint is None:
            endpoint = b""
        if ssl_context:
            bridgeFutureWith[cRequestChannel_ptr](
                (<AsyncClient>client)._executor,
                thrift_ssl.createThriftChannelTCP(
                    ssl_context._cpp_obj,
                    host,
                    port,
                    _timeout_ms,
                    _ssl_timeout_ms,
                    client_type,
                    protocol,
                    endpoint,
                ),
                requestchannel_callback,
                <PyObject *>client,
            )
        else:
            bridgeFutureWith[cRequestChannel_ptr](
                (<AsyncClient>client)._executor,
                createThriftChannelTCP(
                    host, port, _timeout_ms, client_type, protocol, endpoint
                ),
                requestchannel_callback,
                <PyObject *>client,
            )
    elif path is not None:
        fspath = os.fsencode(path)
        bridgeFutureWith[cRequestChannel_ptr](
            (<AsyncClient>client)._executor,
            createThriftChannelUnix(
                cmove[string](fspath), _timeout_ms, client_type, protocol
            ),
            requestchannel_callback,
            <PyObject *>client,
        )
    else:
        raise ValueError("Must set path or host/port")
    factory = get_proxy_factory()
    proxy = factory(clientKlass.Async) if factory else None
    return proxy(client) if proxy else client


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
            type=socket.SOCK_STREAM
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


cdef void requestchannel_callback(
    cFollyTry[cRequestChannel_ptr]&& result,
    PyObject* userData,
) noexcept:
    cdef AsyncClient client = <object> userData
    future = client._connect_future
    if result.hasException():
        try:
            result.exception().throw_exception()
        except Exception as pyex:
            future.set_exception(pyex)
    else:
        client.bind_client(cmove(result.value()))
        future.set_result(None)
