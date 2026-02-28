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

from cpython.ref cimport PyObject
from libc.stdint cimport uint64_t
from libcpp cimport bool as cbool
from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.utility cimport pair

from folly.iobuf cimport cIOBuf
from thrift.py3.std_libcpp cimport milliseconds, string_view
from thrift.python.common cimport cPriority
from thrift.python.server_impl.event_handler cimport cfollySocketAddress
from thrift.python.common cimport Headers

# Cython stdlib in 3.1.x has an except+ on the value() which breaks C++ scoping rules.
cdef extern from "<optional>" namespace "std" nogil:
    cdef cppclass optional[T]:
        cbool has_value()
        T& value()

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

cdef extern from "folly/io/async/AsyncTransportCertificate.h" \
        namespace "folly":
    cdef cppclass AsyncTransportCertificate:
        string getIdentity()

cdef extern from "folly/io/async/AsyncTransport.h" namespace "folly":
    cdef cppclass AsyncTransport:
        const AsyncTransportCertificate* getPeerCertificate()

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


cdef extern from "folly/io/async/ssl/OpenSSLTransportCertificate.h" \
        namespace "folly::OpenSSLTransportCertificate":
    X509UniquePtr tryExtractX509(const AsyncTransportCertificate* cert)


cdef void handleAddressCallback(PyObject* future, cfollySocketAddress address) noexcept


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

cdef object THRIFT_REQUEST_CONTEXT

cdef api Cpp2RequestContext* extract_cpp_request_context(object ctx) except NULL

cdef api Cpp2ConnContext* extract_cpp_connection_context(object ctx) except NULL
