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

import os

from enum import Enum

from cython.operator cimport dereference as deref
from libcpp.memory cimport make_shared
from libcpp.utility cimport move as cmove


init()  # folly::ssl::init()


class SSLVersion(Enum):
    TLSv1_2 = <int> (cTLSv1_2)


class SSLVerifyOption(Enum):
    VERIFY = <int> (cVERIFY)
    VERIFY_REQ_CLIENT_CERT = <int> (cVERIFY_REQ_CLIENT_CERT)
    NO_VERIFY = <int> (cNO_VERIFY)


cdef class SSLContext:
    def __cinit__(self, version=SSLVersion.TLSv1_2):
        cdef cSSLVersion cversion
        if version is SSLVersion.TLSv1_2:
            cversion = cTLSv1_2
        else:
            raise TypeError(f"{version} is not an {SSLVersion}.")
        self._cpp_obj = cmove(make_shared[cSSLContext](cversion))
        self.set_verify_option(SSLVerifyOption.VERIFY_REQ_CLIENT_CERT)

    def set_verify_option(self, option):
        cdef cSSLVerifyPeerEnum coption
        if option is SSLVerifyOption.VERIFY_REQ_CLIENT_CERT:
            coption = cVERIFY_REQ_CLIENT_CERT
        elif option is SSLVerifyOption.VERIFY:
            coption = cVERIFY
        elif option is SSLVerifyOption.NO_VERIFY:
            coption = cNO_VERIFY
        else:
            raise TypeError(f"{option} is not an {SSLVerifyOption}.")
        deref(self._cpp_obj).setVerificationOption(coption)

    @property
    def needs_peer_verify(self):
        return deref(self._cpp_obj).needsPeerVerification()

    def load_cert_chain(self, *, certfile not None, keyfile not None):
        cdef bytes cert = os.fsencode(certfile)
        cdef bytes key = os.fsencode(keyfile)

        deref(self._cpp_obj).loadCertKeyPairFromFiles(cert, key)

    def load_verify_locations(self, *, cafile not None):
        cdef bytes ca = os.fsencode(cafile)
        deref(self._cpp_obj).loadTrustedCertificates(ca)

    def authenticate(self, *, bint peer_cert, bint peer_name):
        deref(self._cpp_obj).authenticate(peer_cert, peer_name)
