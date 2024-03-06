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

import ipaddress
import os
import socket

from libc.stdint cimport uint32_t
from libcpp.utility cimport move as cmove
from libcpp.string cimport string
from thrift.python.client.request_channel cimport (
    sync_createThriftChannelTCP,
    sync_createThriftChannelUnix,
)
from thrift.python.client.request_channel import ClientType


cdef RequestChannel create_channel(
    object host,
    object port,
    object path,
    double timeout,
    cClientType client_type,
    cProtocol protocol,
    thrift_ssl.SSLContext ssl_context,
    double ssl_timeout,
):
    endpoint = b''
    if client_type in (ClientType.THRIFT_HTTP_CLIENT_TYPE, ClientType.THRIFT_HTTP2_CLIENT_TYPE):
        if host is None or port is None:
            raise ValueError("Must set host and port when using HTTP clients")
        if path is None:
            raise ValueError("use path='/endpoint' when using HTTP clients")
        endpoint = os.fsencode(path)
        path = None

    cdef uint32_t _timeout_ms = int(timeout * 1000)
    cdef uint32_t _ssl_timeout_ms = int(ssl_timeout * 1000)

    if host is not None and port is not None:
        if path is not None:
            raise ValueError("Can not set path and host/port at same time")

        if isinstance(host, str):
            try:
                ipaddress.ip_address(host)
            except ValueError:
                host = socket.getaddrinfo(host, port, type=socket.SOCK_STREAM)[0][4][0]
        else:
            host = str(host)

        if ssl_context:
            return RequestChannel.create(thrift_ssl.sync_createThriftChannelTCP(
                ssl_context._cpp_obj,
                host,
                port,
                _timeout_ms,
                _ssl_timeout_ms,
                client_type,
                protocol,
                endpoint,
            ))
        else:
            return RequestChannel.create(sync_createThriftChannelTCP(
                host, port, _timeout_ms, client_type, protocol, endpoint
            ))
    elif path is not None:
        fspath = os.fsencode(path)
        return RequestChannel.create(sync_createThriftChannelUnix(
            cmove[string](fspath), _timeout_ms, client_type, protocol
        ))
    else:
        raise ValueError("Must set path or host/port")
