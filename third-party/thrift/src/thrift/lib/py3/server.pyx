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

from cpython.version cimport PY_VERSION_HEX
from libcpp.memory cimport unique_ptr, shared_ptr, make_shared
from libc.string cimport const_uchar
from cython.operator cimport dereference as deref, preincrement as inc
from libc.stdint cimport uint64_t
from folly.iobuf cimport from_unique_ptr as create_IOBuf
from cpython.ref cimport PyObject
from folly.executor cimport get_executor
from folly.range cimport StringPiece
from libcpp.utility cimport move as cmove
from thrift.py3.std_libcpp cimport sv_to_str
from typing import Mapping

import asyncio
import collections
import functools
import inspect
import ipaddress
from pathlib import Path
import os

from enum import Enum
from thrift.py3.common import Priority, Headers

SocketAddress = collections.namedtuple('SocketAddress', 'ip port path')

from contextvars import ContextVar
# don't include in the module dict, so only cython can set it
THRIFT_REQUEST_CONTEXT = ContextVar('ThriftRequestContext')
get_context = THRIFT_REQUEST_CONTEXT.get


cdef inline _get_SocketAddress(const cfollySocketAddress* sadr):
    if sadr.isFamilyInet():
        ip = ipaddress.ip_address(sadr.getAddressStr().decode('utf-8'))
        return SocketAddress(ip=ip, port=sadr.getPort(), path=None)
    return SocketAddress(ip=None, port=None, path=Path(
            os.fsdecode(sadr.getPath())
        )
    )


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


class SSLPolicy(Enum):
    DISABLED = <int> (SSLPolicy__DISABLED)
    PERMITTED = <int> (SSLPolicy__PERMITTED)
    REQUIRED = <int> (SSLPolicy__REQUIRED)


cdef class AsyncProcessorFactory:
    async def __aenter__(self):
        # Establish async context managers as a way for end users to async initalize
        # internal structures used by Service Handlers.
        return self

    async def __aexit__(self, *exc_info):
        # Same as above, but allow end users to define things to be cleaned up
        pass

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__():
        raise NotImplementedError()

    async def onStartServing(self):
        pass

    async def onStopRequested(self):
        pass


cdef class ServiceInterface(AsyncProcessorFactory):
    pass


cdef class StatusServerInterface:
    pass


def getServiceName(ServiceInterface svc not None):
    processor = deref(svc._cpp_obj).getProcessor()
    gen_proc = dynamic_cast_gen(processor.get())
    if not gen_proc:
        raise TypeError('processor was not a GeneratedAsyncProcessorBase')
    cdef const char* name = gen_proc.getServiceName()
    return (<bytes>name).decode('utf-8')


cdef void handleAddressCallback(PyObject* future, cfollySocketAddress address) noexcept:
    (<object>future).set_result(_get_SocketAddress(&address))


cdef class ThriftServer:
    def __cinit__(self):
        self.server = make_shared[cThriftServer]()

    def __init__(self, AsyncProcessorFactory handler, int port=0, ip=None, path=None):
        self.loop = asyncio.get_event_loop()
        self.factory = handler
        if handler is not None:
            self.server.get().setThreadManagerFromExecutor(get_executor(), b'python_executor')
            if handler._cpp_obj:
                self.server.get().setProcessorFactory(handler._cpp_obj)
            else:
                raise RuntimeError(
                    'The handler is not valid, it has no C++ handler. Maybe its not a '
                    'generated ServiceInterface?'
                )
        else:
            # This thrift server is only for monitoring/status/control
            self.server.get().setProcessorFactory(make_shared[EmptyAsyncProcessorFactory]())
        if path:
            fspath = os.fsencode(path)
            self.server.get().setAddress(
                makeFromPath(
                    StringPiece(fspath, len(fspath))
                )
            )
        elif ip:
            # We stringify to accept python ipaddress objects
            self.server.get().setAddress(str(ip).encode('utf-8'), port)
        else:
            self.server.get().setPort(port)
        self.address_future = self.loop.create_future()
        self.server.get().setServerEventHandler(
            make_shared[Py3ServerEventHandler](
                get_executor(),
                object_partial(handleAddressCallback, <PyObject*> self.address_future)
            )
        )
        self.server.get().metadata().wrapper = b"ThriftServer-py3"

    async def serve(self):
        # This check is only useful for C++-based Thrift servers.
        # TODO(praihan): remove this after migration of C++ services onto extra interfaces
        self.server.get().setAllowCheckUnimplementedExtraInterfaces(False)
        def _serve():
            with nogil:
                self.server.get().serve()
        try:
            await self.loop.run_in_executor(None, _serve)
            self.address_future.cancel()
        except asyncio.CancelledError:
            try:
                await self.get_address()
            finally:
                self.server.get().stop()
            raise
        except Exception as e:
            self.server.get().stop()
            # If somebody is waiting on get_address and the server died
            # then we should forward this exception over to that future.
            if not self.address_future.done():
                self.address_future.set_exception(e)
            raise

    def set_status_interface(self, StatusServerInterface iface not None):
        self.server.get().setStatusInterface(iface._cpp_obj)

    def get_address(self):
        return asyncio.shield(self.address_future)

    def get_active_requests(self):
        return self.server.get().getActiveRequests()

    def get_max_requests(self):
        return self.server.get().getMaxRequests()

    def set_max_requests(self, max_requests):
        self.server.get().setMaxRequests(max_requests)

    def get_max_connections(self):
        return self.server.get().getMaxConnections()

    def set_max_connections(self, max_connections):
        self.server.get().setMaxConnections(max_connections)

    def get_listen_backlog(self):
        return self.server.get().getListenBacklog()

    def set_listen_backlog(self, listen_backlog):
        self.server.get().setListenBacklog(listen_backlog)

    def set_io_worker_threads(self, num):
        self.server.get().setNumIOWorkerThreads(num)

    def get_io_worker_threads(self):
        return self.server.get().getNumIOWorkerThreads()

    def get_cpu_worker_threads(self):
        return self.server.get().getNumCPUWorkerThreads()

    def set_workers_join_timeout(self, timeout):
        self.server.get().setWorkersJoinTimeout(seconds(<int64_t>timeout))

    def get_ssl_policy(self):
        cdef cSSLPolicy cPolicy = self.server.get().getSSLPolicy()
        if cPolicy == SSLPolicy__DISABLED:
            return SSLPolicy.DISABLED
        elif cPolicy == SSLPolicy__PERMITTED:
            return SSLPolicy.PERMITTED
        elif cPolicy == SSLPolicy__REQUIRED:
            return SSLPolicy.REQUIRED
        else:
            raise RuntimeError("Unknown SSLPolicy defined.")

    def set_ssl_policy(self, policy):
        cdef cSSLPolicy cPolicy
        if policy == SSLPolicy.DISABLED:
            cPolicy = SSLPolicy__DISABLED
        elif policy == SSLPolicy.PERMITTED:
            cPolicy = SSLPolicy__PERMITTED
        elif policy == SSLPolicy.REQUIRED:
            cPolicy = SSLPolicy__REQUIRED
        else:
            raise RuntimeError("Unknown SSLPolicy defined.")
        self.server.get().setSSLPolicy(cPolicy)

    def set_allow_plaintext_on_loopback(self, enabled):
        self.server.get().setAllowPlaintextOnLoopback(enabled);

    def is_plaintext_allowed_on_loopback(self):
        return self.server.get().isPlaintextAllowedOnLoopback();

    def set_idle_timeout(self, seconds):
        self.server.get().setIdleTimeout(milliseconds(<int64_t>(seconds * 1000)))

    def get_idle_timeout(self):
        return self.server.get().getIdleTimeout().count() / 1000

    def set_queue_timeout(self, seconds):
        self.server.get().setQueueTimeout(milliseconds(<int64_t>(seconds * 1000)))

    def get_queue_timeout(self):
        return self.server.get().getQueueTimeout().count() / 1000

    cdef void set_is_overloaded(self, cIsOverloadedFunc is_overloaded):
        self.server.get().setIsOverloaded(cmove(is_overloaded))

    def set_language_framework_name(self, name):
        self.server.get().metadata().languageFramework = name.encode()

    def stop(self):
        self.server.get().stop()

    def stop_listening(self):
        self.server.get().stopListening()

    def use_existing_socket(self, socket):
        self.server.get().useExistingSocket(socket)

    def set_stop_workers_on_stop_listening(self, cbool stop_workers):
        self.server.get().setStopWorkersOnStopListening(stop_workers)

    def get_stop_workers_on_stop_listening(self):
        return self.server.get().getStopWorkersOnStopListening()

    def set_idle_server_timeout(self, seconds):
        self.server.get().setIdleServerTimeout(milliseconds(<int64_t>(seconds * 1000)))

    def get_quick_exit_on_shutdown_timeout(self):
        return self.server.get().getQuickExitOnShutdownTimeout()

    def set_quick_exit_on_shutdown_timeout(self, cbool quick_exit_on_shutdown_timeout):
        self.server.get().setQuickExitOnShutdownTimeout(quick_exit_on_shutdown_timeout)

    cdef void add_routing_handler(self, unique_ptr[cTransportRoutingHandler] handler):
        self.server.get().addRoutingHandler(cmove(handler))

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
