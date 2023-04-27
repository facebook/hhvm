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

from thrift.py.client.common import ClientType
from thrift.py.client.sync_client import SyncClient

from thrift.python.client cimport ssl as thrift_ssl
from thrift.python.client.request_channel cimport ClientType as cClientType
from thrift.python.client.sync_channel_factory cimport create_channel
from thrift.python.serializer cimport Protocol as cProtocol
from thrift.transport.TTransport import TTransportException

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
    try:
        channel = create_channel(
            host, port, path, timeout, client_type, protocol, ssl_context, ssl_timeout
        )
        return clientKlass(cpp_transport=SyncClient(channel))
    except RuntimeError as re:
        raise TTransportException(type=TTransportException.NOT_OPEN, message=str(re))
