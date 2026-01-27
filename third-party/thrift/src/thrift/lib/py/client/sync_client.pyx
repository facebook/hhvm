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

from copy import copy
from cython.operator cimport dereference as deref
from folly cimport cFollyTry
from folly.iobuf cimport IOBuf, from_unique_ptr
from libc.stdint cimport uint16_t
from libcpp.memory cimport make_unique
from libcpp.string cimport string
from libcpp.utility cimport move as cmove
from thrift.protocol import TBinaryProtocol, TCompactProtocol
from thrift.py.client.common import Protocol
from thrift.py.client.exceptions cimport create_py_exception
from thrift.python.client.request_channel cimport RequestChannel
from thrift.util.Serializer import serialize, deserialize
from thrift.Thrift import TApplicationException
from thrift.python.common cimport cRpcOptions, RpcOptions


cdef class SyncClient:
    def __init__(SyncClient self, RequestChannel channel):
        self._omni_client = make_unique[cOmniClient](cmove(channel._cpp_obj))

    def __enter__(SyncClient self):
        if not self._omni_client:
            raise RuntimeError("Connection already closed")
        return self

    def __exit__(SyncClient self, exec_type, exc_value, traceback):
        self._omni_client.reset()

    def _send_request(
        SyncClient self,
        string service_name,
        string function_name,
        args,
        response_cls,
        *,
        string uri_or_name = b"",
        RpcOptions rpc_options = None,
    ):
        if not self._omni_client:
            raise RuntimeError("Connection already closed")

        cdef cFollyTry[uint16_t] protocol_try = deref(self._omni_client).getChannelProtocolId()
        if protocol_try.hasException():
            raise create_py_exception(protocol_try.exception())
        cdef uint16_t protocol = protocol_try.value()
        if protocol == Protocol.BINARY:
            protocol_factory = TBinaryProtocol.TBinaryProtocolFactory()
        elif protocol == Protocol.COMPACT:
            protocol_factory = TCompactProtocol.TCompactProtocolFactory()
        else:
            raise TApplicationException(TApplicationException.INVALID_PROTOCOL)

        cdef IOBuf args_iobuf = IOBuf(serialize(protocol_factory, args))

        cdef unordered_map[string, string] headers = self._persistent_headers
        for k, v in self._onetime_headers:
            headers[k] = v
        self._onetime_headers.clear()

        # Providing default RpcOptions because user RpcOptions is not supported
        cdef cRpcOptions c_rpc_options

        if response_cls is None:
            deref(self._omni_client).oneway_send(
                service_name,
                function_name,
                args_iobuf.c_clone(),
                cmove(cData(function_name, FunctionQualifier.OneWay, uri_or_name, InteractionMethodPosition.None, "".encode('ascii'))),
                headers,
                cmove(c_rpc_options),
            )
        else:
            resp = deref(self._omni_client).sync_send(
                service_name,
                function_name,
                args_iobuf.c_clone(),
                cmove(cData(function_name, FunctionQualifier.Unspecified, uri_or_name, InteractionMethodPosition.None, "".encode('ascii'))),
                headers,
                cmove(c_rpc_options),
            )
            if resp.buf.hasValue():
                response_iobuf = from_unique_ptr(cmove(resp.buf.value()))
                response = response_cls()
                deserialize(protocol_factory, b"".join(response_iobuf), response)
                for p in resp.headers:
                    self._last_resp_headers[p.first] = p.second
                return response
            elif resp.buf.hasError():
                raise create_py_exception(resp.buf.error())
            else:
                raise TApplicationException(TApplicationException.MISSING_RESULT)

    def set_persistent_header(SyncClient self, string key, string value):
        self._persistent_headers[key] = value

    def get_persistent_headers(SyncClient self):
        return self._persistent_headers

    def clear_persistent_headers(SyncClient self):
        self._persistent_headers.clear()

    def set_onetime_header(SyncClient self, string key, string value):
        self._onetime_headers[key] = value

    def get_last_response_headers(SyncClient self):
        return self._last_resp_headers
