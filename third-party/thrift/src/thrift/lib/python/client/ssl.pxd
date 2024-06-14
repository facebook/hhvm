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

from libcpp.string cimport string
from libcpp.memory cimport shared_ptr
from libc.stdint cimport uint64_t, uint32_t, uint16_t
from folly cimport cFollyFuture
from thrift.python.protocol cimport Protocol as cProtocol


cdef extern from "folly/io/async/SSLContext.h":
    cdef cppclass cSSLVersion "folly::SSLContext::SSLVersion":
        bint operator==(cSSLVersion&)

    cSSLVersion cTLSv1_2 "folly::SSLContext::TLSv1_2"

    cdef cppclass cSSLVerifyPeerEnum "folly::SSLContext::SSLVerifyPeerEnum":
        bint operator==(cSSLVerifyPeerEnum&)

    cSSLVerifyPeerEnum cVERIFY_REQ_CLIENT_CERT "folly::SSLContext::VERIFY_REQ_CLIENT_CERT"
    cSSLVerifyPeerEnum cVERIFY "folly::SSLContext::VERIFY"
    cSSLVerifyPeerEnum cNO_VERIFY "folly::SSLContext::NO_VERIFY"

    cdef cppclass cSSLContext "folly::SSLContext" nogil:
        cSSLContext(cSSLVersion version)
        void setVerificationOption(const cSSLVerifyPeerEnum& verify)
        bint needsPeerVerification()
        void loadCertKeyPairFromFiles(const char* certPath, const char* keyPath) except+
        void loadTrustedCertificates(const char* path) except+
        void authenticate(bint checkPeerCert, bint checkPeerName)


cdef extern from "<utility>" namespace "std" nogil:
    cdef shared_ptr[cSSLContext] move(shared_ptr[cSSLContext])



cdef class SSLContext:
    cdef object __weakref__
    cdef shared_ptr[cSSLContext] _cpp_obj
