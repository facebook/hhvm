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

from cython.operator cimport dereference as deref
from libc.string cimport const_uchar

from folly.iobuf cimport from_unique_ptr as create_IOBuf
from thrift.python.server_impl.event_handler cimport getRequestId
from thrift.python.common cimport Priority_to_cpp, Headers

import collections
from contextvars import ContextVar
import ipaddress
import os
from pathlib import Path

from thrift.python.common import Priority

SocketAddress = collections.namedtuple('SocketAddress', 'ip port path')

# don't include in the module dict, so only cython can set it
THRIFT_REQUEST_CONTEXT = ContextVar('ThriftRequestContext')
get_context = THRIFT_REQUEST_CONTEXT.get

cdef class RequestContext:
    @staticmethod
    cdef RequestContext _fbthrift_create(Cpp2RequestContext* ctx):
        inst = <RequestContext>RequestContext.__new__(RequestContext)
        inst._ctx = ctx
        inst._c_ctx = ConnectionContext._fbthrift_create(ctx.getConnectionContext())
        inst._requestId = getRequestId()
        return inst

    @property
    def connection_context(self):
        return self._c_ctx

    @property
    def read_headers(self):
        if not self._readheaders:
            self._readheaders = ReadHeaders._fbthrift_create(self)
        return self._readheaders

    @property
    def write_headers(self):
        # So we don't create a cycle
        if not self._writeheaders:
            self._writeheaders = WriteHeaders._fbthrift_create(self)
        return self._writeheaders

    @property
    def priority(self):
        return Priority(<int>self._ctx.getCallPriority())

    def set_header(self, str key not None, str value not None):
        self._ctx.getHeader().setHeader(key.encode('utf-8'), value.encode('utf-8'))

    @property
    def method_name(ConnectionContext self):
        return self._ctx.getMethodName().decode('utf-8')

    @property
    def request_id(self):
        return self._requestId.decode('utf-8')

    @property
    def request_timeout(self):
        return float(self._ctx.getRequestTimeout().count() / 1000)


cdef class ClientMetadata:
    @staticmethod
    cdef ClientMetadata _fbthrift_create(optional[ClientMetadataRef] metadata_ref):
        inst = <ClientMetadata>ClientMetadata.__new__(ClientMetadata)
        inst._cagent = _get_agent_from_metadata(metadata_ref)
        inst._chostname = _get_hostname_from_metadata(metadata_ref)
        inst._cfields = _get_fields_from_metadata(metadata_ref)
        return inst

    @property
    def agent(ClientMetadata self) -> str:
        return self._cagent.decode('utf-8')

    @property
    def hostname(ClientMetadata self) -> str:
        return self._chostname.decode('utf-8')

    def getMetadataField(self, str key not None) -> str:
        if key is None:
          return ""
        cdef string ckey = key.encode('utf-8')
        it = self._cfields.find(ckey)
        if it == self._cfields.end():
          return ""
        return (<bytes>deref(it).second).decode('utf-8')


cdef class ConnectionContext:
    @staticmethod
    cdef ConnectionContext _fbthrift_create(Cpp2ConnContext* ctx):
        cdef const cfollySocketAddress* peer_address
        cdef const cfollySocketAddress* local_address
        inst = <ConnectionContext>ConnectionContext.__new__(ConnectionContext)
        if ctx:
            inst._ctx = ctx
            peer_address = ctx.getPeerAddress()
            if not peer_address.empty():
                inst._peer_address = _get_SocketAddress(peer_address)
            local_address = ctx.getLocalAddress()
            if not local_address.empty():
                inst._local_address = _get_SocketAddress(local_address)
            inst._client_metadata = ClientMetadata._fbthrift_create(ctx.getClientMetadataRef())
        return inst

    @property
    def peer_address(ConnectionContext self):
        return self._peer_address

    @property
    def peer_common_name(ConnectionContext self):
        return self._ctx.getPeerCommonName().decode('utf-8')

    @property
    def security_protocol(ConnectionContext self):
        return self._ctx.getSecurityProtocol().decode('utf-8')

    @property
    def peer_certificate(ConnectionContext self):
        cdef const_uchar* data
        cdef X509UniquePtr cert
        cdef uint64_t length
        cdef const AsyncTransport* transport
        cdef const AsyncTransportCertificate* osslCert
        transport = self._ctx.getTransport()
        if not transport:
            return None
        osslCert = transport.getPeerCertificate()
        if not osslCert:
            return None
        cert = tryExtractX509(osslCert);
        if cert.get():
            iobuf = create_IOBuf(derEncode(deref(cert.get())))
            if iobuf.is_chained:
                return b''.join(iobuf)
            return bytes(iobuf)
        return None

    @property
    def peer_certificate_identity(ConnectionContext self):
        cdef const AsyncTransport* transport
        cdef const AsyncTransportCertificate* osslCert
        transport = self._ctx.getTransport()
        if not transport:
            return None
        osslCert = transport.getPeerCertificate()
        if not osslCert:
            return None
        return deref(osslCert).getIdentity().decode('utf-8')

    @property
    def local_address(ConnectionContext self):
        return self._local_address

    @property
    def client_metadata(ConnectionContext self):
      return self._client_metadata


cdef class ReadHeaders(Headers):
    @staticmethod
    cdef _fbthrift_create(RequestContext ctx):
        inst = <ReadHeaders>ReadHeaders.__new__(ReadHeaders)
        inst._parent = ctx
        return inst

    cdef const F14NodeMap[string, string]* _getMap(self):
        return &self._parent._ctx.getHeader().getHeaders()


cdef class WriteHeaders(Headers):
    @staticmethod
    cdef _fbthrift_create(RequestContext ctx):
        inst = <WriteHeaders>WriteHeaders.__new__(WriteHeaders)
        inst._parent = ctx
        return inst

    cdef const F14NodeMap[string, string]* _getMap(self):
        return &self._parent._ctx.getHeader().getWriteHeaders()


cdef void handleAddressCallback(PyObject* future, cfollySocketAddress address) noexcept:
    (<object>future).set_result(_get_SocketAddress(&address))

cdef inline string _get_agent_from_metadata(optional[ClientMetadataRef] metadata_ref):
    cdef string cagent
    if not metadata_ref.has_value():
      return cagent
    if not metadata_ref.value().getAgent().has_value():
        return cagent
    cagent = metadata_ref.value().getAgent().value().data()
    return cagent

cdef inline string _get_hostname_from_metadata(optional[ClientMetadataRef] metadata_ref):
    cdef string chostname
    if not metadata_ref.has_value():
      return chostname
    if not metadata_ref.value().getHostname().has_value():
      return chostname
    chostname = metadata_ref.value().getHostname().value().data()
    return chostname

cdef inline F14NodeMap[string, string] _get_fields_from_metadata(optional[ClientMetadataRef] metadata_ref):
    cdef F14NodeMap[string, string] empty_fields
    if not metadata_ref.has_value():
      return empty_fields
    return metadata_ref.value().getFields()

cdef inline _get_SocketAddress(const cfollySocketAddress* sadr):
    if sadr.isFamilyInet():
        ip = ipaddress.ip_address(sadr.getAddressStr().decode('utf-8'))
        return SocketAddress(ip=ip, port=sadr.getPort(), path=None)
    return SocketAddress(ip=None, port=None, path=Path(
            os.fsdecode(sadr.getPath())
        )
    )

cdef api Cpp2RequestContext* extract_cpp_request_context(object ctx) except NULL:
    return (<RequestContext?>ctx)._ctx

cdef api Cpp2ConnContext* extract_cpp_connection_context(object ctx) except NULL:
    return (<ConnectionContext?>ctx)._ctx
