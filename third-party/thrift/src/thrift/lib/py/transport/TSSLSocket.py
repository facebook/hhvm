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

# pyre-unsafe

from __future__ import absolute_import, division, print_function, unicode_literals

# workaround for a python bug.  see http://bugs.python.org/issue8484
import hashlib
import logging
import os
import socket
import ssl
import sys
import traceback

from thrift.transport.TSocket import TServerSocket, TSocket
from thrift.transport.TTransport import TTransportException

# Python 2/3 compatibility
if sys.version_info[0] >= 3:
    unicode = str


def _detect_legacy_ssl() -> bool:
    """
    Checks whether or not we have the newer Python >= 2.7.9,3.2+ attributes
    necessary to properly configure TLS settings
    """
    required_attributes = [
        "SSLContext",
        "OP_NO_SSLv2",
        "OP_NO_SSLv3",
        "OP_NO_TLSv1",
    ]
    return not all(hasattr(ssl, attr) for attr in required_attributes)


_is_legacy_ssl: bool = _detect_legacy_ssl()


def _best_possible_default_version():
    global _is_legacy_ssl
    if _is_legacy_ssl:
        # Python < 2.7.9 does not expose OP_NO_SSLv2 and OP_NO_SSLv3. Depending
        # on what version of OpenSSL Python is linked against, SSLv23 *may*
        # be able to connect to TLS 1.2, but since we can't disable SSLv2
        # and SSLv3, we might as well default to TLS 1.0.
        return ssl.PROTOCOL_TLSv1

    # Newer versions of Python (>= 3.6.0) introduced PROTOCOL_TLS, which is
    # recommended against PROTOCOL_SSLv23, even though at this time they are
    # aliases for one another.
    return next(
        getattr(ssl, p) for p in ["PROTOCOL_TLS", "PROTOCOL_SSLv23"] if hasattr(ssl, p)
    )


if _is_legacy_ssl:
    _has_warned = False

    def _warn_if_legacy():
        global _has_warned
        if not _has_warned:
            logging.warning(
                "You are using an old version of Python (< 2.7.9) that is "
                "limited to an old version of TLS (1.0) with known security "
                "vulnerabilities. "
            )
            _has_warned = True

    def _warn_if_insecure_version_specified(version):
        pass

    def _get_ssl_socket(
        socket,
        ssl_version,
        cert_reqs=ssl.CERT_NONE,
        ca_certs=None,
        keyfile=None,
        certfile=None,
        **kwargs,
    ):
        return ssl.SSLSocket(
            socket,
            ssl_version=ssl_version,
            cert_reqs=cert_reqs,
            ca_certs=ca_certs,
            keyfile=keyfile,
            certfile=certfile,
        )

else:

    def _warn_if_legacy():
        pass

    def _warn_if_insecure_version_specified(version):
        if version is None:
            return

        blacklist = [ssl.PROTOCOL_TLSv1]

        if hasattr(ssl, "PROTOCOL_SSLv2"):
            blacklist.append(ssl.PROTOCOL_SSLv2)
        if hasattr(ssl, "PROTOCOL_SSLv3"):
            blacklist.append(ssl.PROTOCOL_SSLv3)
        if hasattr(ssl, "PROTOCOL_TLSv1_1"):
            blacklist.append(ssl.PROTOCOL_TLSv1_1)

        if version in blacklist:
            logging.warning(
                "You are constructing TSSLSocket and intentionally specifying "
                "a weak, vulnerable ssl_version on a platform that has secure "
                "versions available! Leave ssl_version unspecified and we will "
                "automatically choose a suitable, secure version for you."
            )

    def _get_ssl_socket(
        socket,
        ssl_version,
        cert_reqs=ssl.CERT_NONE,
        ca_certs=None,
        keyfile=None,
        certfile=None,
        disable_weaker_versions=True,
    ):
        ctx = ssl.SSLContext(ssl_version)

        if ssl.HAS_ALPN:
            # Provide an ALPN value for the legacy Header transport so that the
            # Thrift server doesn't have to peek at any bytes.
            ctx.set_alpn_protocols(["thrift"])

        # Some protocol versions, like PROTOCOL_TLS_CLIENT, automatically enable
        # SSLContext.check_hostname. Disable it here, we don't want to perform
        # hostname verification at this layer.
        ctx.check_hostname = False
        ctx.verify_mode = cert_reqs
        if certfile is not None:
            ctx.load_cert_chain(
                certfile=certfile,
                keyfile=keyfile,
            )

        if ca_certs is not None:
            ctx.load_verify_locations(
                cafile=ca_certs,
            )

        ctx.options |= ssl.OP_NO_SSLv2
        ctx.options |= ssl.OP_NO_SSLv3

        if disable_weaker_versions:
            ctx.options |= ssl.OP_NO_TLSv1

            # Python 2.7.9+ has this symbol, Python 3 only gets this at 3.4
            if hasattr(ssl, "OP_NO_TLSv1_1"):
                ctx.options |= ssl.OP_NO_TLSv1_1

        return ctx.wrap_socket(socket)


class TSSLSocket(TSocket):
    """Socket implementation that communicates over an SSL/TLS encrypted
    channel."""

    def __init__(
        self,
        host="localhost",
        port=9090,
        unix_socket=None,
        ssl_version=None,
        cert_reqs=ssl.CERT_NONE,
        ca_certs=None,
        verify_name=False,
        keyfile=None,
        certfile=None,
        allow_weak_ssl_versions=False,
    ):
        """Initialize a TSSLSocket.

        @param ssl_version(int)  protocol version. see ssl module. If none is
                                 specified, we will default to the most
                                 reasonably secure and compatible configuration
                                 if possible.

                                 For Python versions >= 2.7.9, we will default
                                 to at least TLS 1.1.

                                 For Python versions < 2.7.9, we can only
                                 default to TLS 1.0, which is the best that
                                 Python guarantees to offers at this version.
                                 If you specify ssl.PROTOCOL_SSLv23, and
                                 the OpenSSL linked with Python is new enough,
                                 it is possible for a TLS 1.2 connection be
                                 established; however, there is no way in
                                 < Python 2.7.9 to explicitly disable SSLv2
                                 and SSLv3. For that reason, we default to
                                 TLS 1.0.

        @param cert_reqs(int)    whether to verify peer certificate. see ssl
                                 module.
        @param ca_certs(str)     filename containing trusted root certs.
        @param verify_name       if False, no peer name validation is performed
                                 if True, verify subject name of peer vs 'host'
                                 if a str, verify subject name of peer vs given
                                 str
        @param keyfile           filename containing the client's private key
        @param certfile          filename containing the client's cert and
                                 optionally the private key

        @param allow_weak_ssl_versions(bool) By default, we try to disable older
                                             protocol versions. Only set this
                                             if you know what you are doing.
        """
        TSocket.__init__(self, host, port, unix_socket)
        self.cert_reqs = cert_reqs
        self.ca_certs = ca_certs
        self.ssl_version = ssl_version
        self.verify_name = verify_name
        self.client_keyfile = keyfile
        self.client_certfile = certfile
        self.allow_weak_ssl_versions = allow_weak_ssl_versions
        _warn_if_legacy()
        _warn_if_insecure_version_specified(ssl_version)

    def open(self):
        TSocket.open(self)
        try:
            ssl_version = (
                self.ssl_version
                if self.ssl_version is not None
                else _best_possible_default_version()
            )

            sslh = _get_ssl_socket(
                self.handle,
                ssl_version=ssl_version,
                cert_reqs=self.cert_reqs,
                ca_certs=self.ca_certs,
                keyfile=self.client_keyfile,
                certfile=self.client_certfile,
                disable_weaker_versions=not self.allow_weak_ssl_versions,
            )

            if self.verify_name:
                # validate the peer certificate commonName against the
                # hostname (or given name) that we were expecting.
                cert = sslh.getpeercert()
                str_type = (str, unicode) if sys.version_info[0] < 3 else str
                if isinstance(self.verify_name, str_type):
                    valid_names = self._getCertNames(cert)
                    name = self.verify_name
                else:
                    valid_names = self._getCertNames(cert, "DNS")
                    name = self.host
                match = False
                for valid_name in valid_names:
                    if self._matchName(name, valid_name):
                        match = True
                        break
                if not match:
                    sslh.close()
                    raise TTransportException(
                        TTransportException.NOT_OPEN,
                        "failed to verify certificate name",
                    )
            self.setHandle(sslh)
        except ssl.SSLError as e:
            raise TTransportException(
                TTransportException.NOT_OPEN, "SSL error during handshake: " + str(e)
            )
        except socket.error as e:
            raise TTransportException(
                TTransportException.NOT_OPEN,
                "socket error during SSL handshake: " + str(e),
            )

    @staticmethod
    def _getCertNames(cert, includeAlt=None):
        """Returns a set containing the common name(s) for the given cert. If
        includeAlt is not None, then X509v3 alternative names of type includeAlt
        (e.g. 'DNS', 'IPADD') will be included as potential matches."""
        # The "subject" field is a tuple containing the sequence of relative
        # distinguished names (RDNs) given in the certificate's data structure
        # for the principal, and each RDN is a sequence of name-value pairs.
        names = set()
        for rdn in cert.get("subject", ()):
            for k, v in rdn:
                if k == "commonName":
                    names.add(v)
        if includeAlt:
            for k, v in cert.get("subjectAltName", ()):
                if k == includeAlt:
                    names.add(v)
        return names

    @staticmethod
    def _matchName(name, pattern):
        """match a DNS name against a pattern. match is not case sensitive.
        a '*' in the pattern will match any single component of name."""
        name_parts = name.split(".")
        pattern_parts = pattern.split(".")
        if len(name_parts) != len(pattern_parts):
            return False
        for n, p in zip(name_parts, pattern_parts):
            if p != "*" and (n.lower() != p.lower()):
                return False
        return True


class TSSLServerSocket(TServerSocket):
    """
    SSL implementation of TServerSocket

    Note that this does not support TNonblockingServer

    This uses the ssl module's wrap_socket() method to provide SSL
    negotiated encryption.
    """

    def __init__(
        self,
        port=9090,
        ssl_version=ssl.PROTOCOL_TLSv1,
        cert_reqs=ssl.CERT_NONE,
        ca_certs=None,
        certfile="cert.pem",
        unix_socket=None,
    ):
        """Initialize a TSSLServerSocket

        @param certfile: The filename of the server certificate file, defaults
                         to cert.pem
        @type certfile: str
        @param port: The port to listen on for inbound connections.
        @type port: int
        """
        self.setCertfile(certfile)
        self.setCertReqs(cert_reqs, ca_certs)
        self.ssl_version = ssl_version
        TServerSocket.__init__(self, port, unix_socket)

    def setCertfile(self, certfile):
        """Set or change the server certificate file used to wrap new
        connections.

        @param certfile: The filename of the server certificate, i.e.
                         '/etc/certs/server.pem'
        @type certfile: str

        Raises an IOError exception if the certfile is not present or
        unreadable.
        """
        if not os.access(certfile, os.R_OK):
            raise IOError("No such certfile found: %s" % (certfile))
        self.certfile = certfile

    def setCertReqs(self, cert_reqs, ca_certs):
        """Set or change the parameters used to validate the client's
        certificate.  The parameters behave the same as the arguments to
        python's ssl.wrap_socket() method with the same name.
        """
        self.cert_reqs = cert_reqs
        self.ca_certs = ca_certs

    def accept(self):
        plain_client, addr = self._sock_accept()
        try:
            context = ssl.SSLContext(self.ssl_version)
            context.verify_mode = self.cert_reqs

            if self.certfile:
                context.load_cert_chain(certfile=self.certfile)
            if self.ca_certs:
                context.load_verify_locations(cafile=self.ca_certs)

            client = context.wrap_socket(
                plain_client,
                server_side=True,
            )
        except ssl.SSLError:
            # failed handshake/ssl wrap, close socket to client
            plain_client.close()
            # raise ssl_exc
            # We can't raise the exception, because it kills most TServer
            # derived serve() methods.
            # Instead, return None, and let the TServer instance deal with it
            # in other exception handling.  (but TSimpleServer dies anyway)
            print(traceback.print_exc())
            return None

        return self._makeTSocketFromAccepted((client, addr))
