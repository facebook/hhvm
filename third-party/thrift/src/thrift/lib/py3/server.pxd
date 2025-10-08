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
from libcpp cimport bool as cbool
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.string cimport string

from folly.iobuf cimport cIOBuf
from folly cimport cFollyExecutor
from thrift.py3.std_libcpp cimport milliseconds, seconds
from thrift.python.server_impl.event_handler cimport (
    PythonServerEventHandler,
    cBaseThriftServerMetadata,
    cIsOverloadedFunc,
    cSSLPolicy,
    cfollySocketAddress,
    cTransportRoutingHandler,
)
from thrift.python.types cimport ServiceInterface as PythonServiceInterface
from thrift.python.server_impl.async_processor cimport (
    cAsyncProcessorFactory,
    AsyncProcessorFactory as Py3AsyncProcessorFactory,
)
from thrift.python.server_impl.interceptor.server_module cimport cServerModule
from thrift.python.types cimport cServiceHealth, cServiceHealth_OK, cServiceHealth_ERROR


cdef extern from "thrift/lib/cpp2/server/StatusServerInterface.h" \
        namespace "apache::thrift":
    cdef cppclass cStatusServerInterface "apache::thrift::StatusServerInterface"(cAsyncProcessorFactory):
        pass


cdef extern from "thrift/lib/cpp2/server/ThriftServer.h" \
        namespace "apache::thrift":

    cdef cppclass cThriftServer "apache::thrift::ThriftServer":
        ThriftServer() nogil except +
        void setPort(uint16_t port) nogil
        uint16_t getPort() nogil
        void setAddress(cfollySocketAddress& addr) nogil
        void setAddress(string ip, uint16_t port) nogil
        void setInterface(shared_ptr[cAsyncProcessorFactory]) nogil
        void setStatusInterface(shared_ptr[cStatusServerInterface]) nogil
        void serve() nogil except +
        void stop() nogil except +
        void stopListening() nogil except +
        cSSLPolicy getSSLPolicy() nogil
        void setSSLPolicy(cSSLPolicy policy) nogil
        void setServerEventHandler(shared_ptr[PythonServerEventHandler] handler) nogil
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
        void setSocketQueueTimeout(milliseconds timeout)
        milliseconds getSocketQueueTimeoutMs()
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
        void addRoutingHandler(unique_ptr[cTransportRoutingHandler])
        void disableInfoLogging()
        cbool resourcePoolEnabled() nogil noexcept
        void requireResourcePools() nogil noexcept
        void setTaskExpireTime(milliseconds timeout)
        void setUseClientTimeout(cbool useClientTimeout)
        void addModule(unique_ptr[cServerModule] module)
        void setStreamExpireTime(milliseconds timeout)

cdef extern from "thrift/lib/cpp2/server/ThriftServerInternals.h" \
        namespace "apache::thrift::detail":
    cdef cppclass cThriftServerInternals "apache::thrift::detail::ThriftServerInternals":
        cThriftServerInternals(cThriftServer&) nogil except +
        void disableServiceHealthPoller() nogil
        void setServiceHealth(cServiceHealth health) nogil

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



cdef class ServiceInterface(Py3AsyncProcessorFactory):
    pass


cdef class ThriftServer:
    cdef shared_ptr[cThriftServer] server
    cdef Py3AsyncProcessorFactory factory
    cdef object loop
    cdef object address_future
    cdef void set_is_overloaded(self, cIsOverloadedFunc is_overloaded)
    cdef void add_routing_handler(self, unique_ptr[cTransportRoutingHandler] handler)
    # handler only set when initialized with thrift-python ServiceInterface
    cdef PythonServiceInterface handler
    # Health polling members
    cdef object _health_polling_task
    cdef double _health_polling_interval
    cdef cServiceHealth c_health


cdef class StatusServerInterface:
    cdef shared_ptr[cStatusServerInterface] _cpp_obj
