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

from libc.stdint cimport uint16_t, int32_t, uint32_t, int64_t
from libcpp.string cimport string
from libcpp cimport bool as cbool
from libcpp.memory cimport shared_ptr, unique_ptr
from folly.iobuf cimport cIOBuf
from folly.range cimport StringPiece
from folly cimport cFollyExecutor
from cpython.ref cimport PyObject
from thrift.py3.common cimport cPriority, Priority_to_cpp, Headers, cThriftMetadata
from thrift.py3.std_libcpp cimport milliseconds, seconds, string_view
from libcpp.optional cimport optional
from libcpp.utility cimport pair


cdef extern from "thrift/lib/py3/server.h" namespace "::thrift::py3":
    cdef cppclass cfollySocketAddress "folly::SocketAddress":
        uint16_t getPort()
        bint isFamilyInet()
        bint empty()
        string getAddressStr()
        string getPath()

    cfollySocketAddress makeFromPath "folly::SocketAddress::makeFromPath"(StringPiece path)

    cdef cppclass AddressHandler:  # This isn't true but its easier for cython
        pass

    AddressHandler object_partial(void(*)(PyObject*, cfollySocketAddress), PyObject*)

    cdef cppclass Py3ServerEventHandler:
        Py3ServerEventHandler(cFollyExecutor*, AddressHandler) nogil

    cdef cppclass cIsOverloadedFunc "apache::thrift::IsOverloadedFunc":
        pass
    string getRequestId() except +

cdef extern from "thrift/lib/cpp2/async/AsyncProcessor.h" \
        namespace "apache::thrift":
    cdef cppclass cAsyncProcessor "apache::thrift::AsyncProcessor":
        pass

    cdef cppclass cGeneratedAsyncProcessorBase "apache::thrift::GeneratedAsyncProcessorBase"(cAsyncProcessor):
        const char* getServiceName()

    cdef cppclass cAsyncProcessorFactory \
            "apache::thrift::AsyncProcessorFactory":
        unique_ptr[cAsyncProcessor] getProcessor()

    cdef cppclass cServerInterface \
            "apache::thrift::ServerInterface"(cAsyncProcessorFactory):
        pass

    cdef cGeneratedAsyncProcessorBase* dynamic_cast_gen "dynamic_cast<apache::thrift::GeneratedAsyncProcessorBase*>"(...)

cdef extern from "thrift/lib/cpp2/server/TransportRoutingHandler.h" \
        namespace "apache::thrift":
    cdef cppclass cTransportRoutingHandler "apache::thrift::TransportRoutingHandler":
        pass

cdef extern from "thrift/lib/cpp2/server/StatusServerInterface.h" \
        namespace "apache::thrift":
    cdef cppclass cStatusServerInterface "apache::thrift::StatusServerInterface"(cAsyncProcessorFactory):
        pass

cdef extern from "thrift/lib/cpp2/util/EmptyAsyncProcessor.h":
    # This is a little wonky, but makes using it much easier from cython.
    # without having to use a static_pointer_cast to make cython happy.
    ctypedef cAsyncProcessorFactory EmptyAsyncProcessorFactory "apache::thrift::EmptyAsyncProcessorFactory"

cdef extern from "thrift/lib/cpp2/server/ThriftServer.h" \
        namespace "apache::thrift":

    cdef cppclass cSSLPolicy "apache::thrift::SSLPolicy":
        bint operator==(cSSLPolicy&)

    cSSLPolicy SSLPolicy__DISABLED "apache::thrift::SSLPolicy::DISABLED"
    cSSLPolicy SSLPolicy__PERMITTED "apache::thrift::SSLPolicy::PERMITTED"
    cSSLPolicy SSLPolicy__REQUIRED "apache::thrift::SSLPolicy::REQUIRED"

    cdef cppclass cBaseThriftServerMetadata "apache::thrift::BaseThriftServer::Metadata":
        string wrapper
        string languageFramework

    cdef cppclass cThriftServer "apache::thrift::ThriftServer":
        ThriftServer() nogil except +
        void setPort(uint16_t port) nogil
        uint16_t getPort() nogil
        void setAddress(cfollySocketAddress& addr) nogil
        void setAddress(string ip, uint16_t port) nogil
        void setInterface(shared_ptr[cServerInterface]) nogil
        void setStatusInterface(shared_ptr[cStatusServerInterface]) nogil
        void setProcessorFactory(shared_ptr[cAsyncProcessorFactory]) nogil
        void serve() nogil except +
        void stop() nogil except +
        void stopListening() nogil except +
        cSSLPolicy getSSLPolicy() nogil
        void setSSLPolicy(cSSLPolicy policy) nogil
        void setServerEventHandler(shared_ptr[Py3ServerEventHandler] handler) nogil
        int32_t getActiveRequests()
        uint32_t getMaxRequests()
        void setMaxRequests(uint32_t maxRequests)
        uint32_t getMaxConnections()
        void setMaxConnections(uint32_t maxConnections)
        int getListenBacklog()
        void setListenBacklog(int listenBacklog)
        void setNumIOWorkerThreads(uint32_t numIOWorkerThreads)
        uint32_t getNumIOWorkerThreads()
        void setNumCPUWorkerThreads(uint32_t numCPUWorkerThreads)
        uint32_t getNumCPUWorkerThreads()
        void setWorkersJoinTimeout(seconds timeout)
        void setAllowPlaintextOnLoopback(cbool allow)
        cbool isPlaintextAllowedOnLoopback()
        void setIdleTimeout(milliseconds idleTimeout)
        milliseconds getIdleTimeout()
        void setQueueTimeout(milliseconds timeout)
        milliseconds getQueueTimeout()
        void setIsOverloaded(cIsOverloadedFunc isOverloaded)
        void useExistingSocket(int socket) except +
        cBaseThriftServerMetadata& metadata()
        void setThreadManagerFromExecutor(cFollyExecutor*, string)
        void setStopWorkersOnStopListening(cbool stopWorkers)
        cbool getStopWorkersOnStopListening()
        void setAllowCheckUnimplementedExtraInterfaces(cbool allow)
        void setIdleServerTimeout(milliseconds idleServerTimeout)
        cbool getQuickExitOnShutdownTimeout()
        void setQuickExitOnShutdownTimeout(cbool quickExitOnShutdownTimeout)

cdef extern from "folly/ssl/OpenSSLCertUtils.h" \
        namespace "folly::ssl":
    # I need a opque id for x509 structs
    cdef cppclass X509:
        pass
    cdef cppclass X509UniquePtr:
        X509* get()

cdef extern from "folly/ssl/OpenSSLCertUtils.h" \
        namespace "folly::ssl::OpenSSLCertUtils":
    unique_ptr[cIOBuf] derEncode(X509& cert)

cdef extern from "folly/io/async/AsyncTransportCertificate.h" \
        namespace "folly":
    cdef cppclass AsyncTransportCertificate:
        string getIdentity()

cdef extern from "folly/io/async/ssl/OpenSSLTransportCertificate.h" \
        namespace "folly::OpenSSLTransportCertificate":
    X509UniquePtr tryExtractX509(const AsyncTransportCertificate* cert)

cdef extern from "folly/io/async/AsyncTransport.h" namespace "folly":
    cdef cppclass AsyncTransport:
        const AsyncTransportCertificate* getPeerCertificate()

cdef extern from "folly/container/F14Map.h" namespace "folly":
  cdef cppclass F14NodeMap[K, T]:
    cppclass const_iterator:
        pair[K, T]& operator*()
        const_iterator operator++()
        const_iterator operator--()
        cbool operator==(const_iterator)
        cbool operator!=(const_iterator)
    const_iterator begin()
    const_iterator end()
    const_iterator find(const K&)

cdef extern from "thrift/lib/cpp/transport/THeader.h" namespace "apache::thrift":
    cdef cppclass THeader:
        const F14NodeMap[string, string]& getWriteHeaders()
        const F14NodeMap[string, string]& getHeaders()
        void setHeader(string& key, string& value)

cdef extern from "thrift/lib/cpp2/server/Cpp2ConnContext.h" \
        namespace "apache::thrift":

    cdef cppclass Cpp2ConnContext:
        string getSecurityProtocol()
        string getPeerCommonName()
        AsyncTransport* getTransport()
        cfollySocketAddress* getPeerAddress()
        cfollySocketAddress* getLocalAddress()
        optional[ClientMetadataRef] getClientMetadataRef()

    cdef cppclass Cpp2RequestContext:
        Cpp2ConnContext* getConnectionContext()
        cPriority getCallPriority()
        THeader* getHeader()
        string getMethodName()
        milliseconds getRequestTimeout()

    cdef cppclass ClientMetadataRef:
      optional[string_view] getAgent()
      optional[string_view] getHostname()
      optional[string_view] getOtherMetadataField(string_view key)
      const F14NodeMap[string, string]& getFields()


cdef class AsyncProcessorFactory:
    cdef shared_ptr[cAsyncProcessorFactory] _cpp_obj


cdef class ServiceInterface(AsyncProcessorFactory):
    pass


cdef class ThriftServer:
    cdef shared_ptr[cThriftServer] server
    cdef AsyncProcessorFactory factory
    cdef object loop
    cdef object address_future
    cdef void set_is_overloaded(self, cIsOverloadedFunc is_overloaded)


cdef class ClientMetadata:
    cdef string _cagent
    cdef string _chostname
    cdef F14NodeMap[string, string] _cfields

    @staticmethod
    cdef ClientMetadata _fbthrift_create(optional[ClientMetadataRef] metadata_ref)


cdef class ConnectionContext:
    cdef Cpp2ConnContext* _ctx
    cdef object _peer_address
    cdef object _local_address
    cdef ClientMetadata _client_metadata

    @staticmethod
    cdef ConnectionContext _fbthrift_create(Cpp2ConnContext* ctx)


cdef class RequestContext:
    cdef ConnectionContext _c_ctx
    cdef Cpp2RequestContext* _ctx
    cdef object _readheaders
    cdef object _writeheaders
    cdef string _requestId

    @staticmethod
    cdef RequestContext _fbthrift_create(Cpp2RequestContext* ctx)


cdef class ReadHeaders(Headers):
    cdef RequestContext _parent
    @staticmethod
    cdef _fbthrift_create(RequestContext ctx)


cdef class WriteHeaders(Headers):
    cdef RequestContext _parent
    @staticmethod
    cdef _fbthrift_create(RequestContext ctx)


cdef class StatusServerInterface:
    cdef shared_ptr[cStatusServerInterface] _cpp_obj


cdef extern from "<utility>" namespace "std" nogil:
    cdef unique_ptr[cTransportRoutingHandler] move(unique_ptr[cTransportRoutingHandler])

cdef object THRIFT_REQUEST_CONTEXT
