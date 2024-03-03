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

import http.client as http_client
import os
import socket
import sys
import warnings
from io import BytesIO as StringIO
from urllib import parse

from thrift.transport.TTransport import TTransportBase, TTransportException


class THttpClient(TTransportBase):
    """Http implementation of TTransport base."""

    def __init__(self, uri_or_host, port=None, path=None, ssl_context=None):
        """THttpClient supports two different types constructor parameters.

        THttpClient(host, port, path) - deprecated
        THttpClient(uri)

        Only the second supports https."""

        if port is not None:
            warnings.warn(
                "Please use the THttpClient('http://host:port/path') syntax",
                DeprecationWarning,
                stacklevel=2,
            )
            self.host = uri_or_host
            self.http_host = self.host
            self.port = port
            assert path
            self.path = path
            self.scheme = "http"
        else:
            parsed = parse.urlparse(uri_or_host)
            self.scheme = parsed.scheme
            assert self.scheme in ("http", "https")
            if self.scheme == "http":
                self.port = parsed.port or http_client.HTTP_PORT
            elif self.scheme == "https":
                self.port = parsed.port or http_client.HTTPS_PORT
            self.host = parsed.hostname
            self.http_host = parsed.netloc
            self.path = parsed.path
            if parsed.query:
                self.path += "?%s" % parsed.query
        self.__wbuf = StringIO()
        self.__http = None
        self.__timeout = None
        self.__custom_headers = None
        self.ssl_context = ssl_context

    def open(self):
        if self.scheme == "http":
            self.__http = http_client.HTTPConnection(
                self.host, self.port, timeout=self.__timeout
            )
        else:
            self.__http = http_client.HTTPSConnection(
                self.host, self.port, context=self.ssl_context, timeout=self.__timeout
            )

    def close(self):
        self.__http.close()
        self.__http = None

    def isOpen(self):
        return self.__http is not None

    def setTimeout(self, ms):
        if ms is None:
            self.__timeout = None
        else:
            self.__timeout = ms / 1000.0

    def setCustomHeaders(self, headers):
        self.__custom_headers = headers

    def setCustomHeader(self, name, value):
        if self.__custom_headers is None:
            self.__custom_headers = {}
        self.__custom_headers[name] = value

    def read(self, sz):
        return self.response.read(sz)

    def write(self, buf):
        self.__wbuf.write(buf)

    def flush(self):
        if self.isOpen():
            self.close()
        self.open()

        # Pull data out of buffer
        data = self.__wbuf.getvalue()
        self.__wbuf = StringIO()

        # HTTP request
        self.__http.putrequest("POST", self.path, skip_host=True)

        if not self.__custom_headers or "Host" not in self.__custom_headers:
            self.__http.putheader("Host", self.http_host)

        self.__http.putheader("Content-Type", "application/x-thrift")
        self.__http.putheader("Content-Length", str(len(data)))

        if not self.__custom_headers or "User-Agent" not in self.__custom_headers:
            user_agent = "Python/THttpClient"
            script = os.path.basename(sys.argv[0])
            if script:
                user_agent = "%s (%s)" % (user_agent, parse.quote(script))
            self.__http.putheader("User-Agent", user_agent)

        if self.__custom_headers:
            if sys.version_info[0] >= 3:
                custom_headers_iter = self.__custom_headers.items()
            else:
                custom_headers_iter = self.__custom_headers.items()
            for key, val in custom_headers_iter:
                self.__http.putheader(key, val)

        try:
            self.__http.endheaders()

            # Write payload
            self.__http.send(data)
        except socket.gaierror as e:
            raise TTransportException(TTransportException.NOT_OPEN, str(e))
        except Exception as e:
            raise TTransportException(TTransportException.UNKNOWN, str(e))

        # Get reply to flush the request
        self.response = self.__http.getresponse()
        self.code = self.response.status
        self.headers = self.response.getheaders()
