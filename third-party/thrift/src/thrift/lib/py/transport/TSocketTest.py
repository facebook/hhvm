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

from __future__ import absolute_import, division, print_function, unicode_literals

import os.path
import socket
import tempfile
import threading
import time
import unittest

import thrift.transport.TSocket as TSocket
import thrift.transport.TTransport as TTransport


class TSocketTest(unittest.TestCase):
    def test_usage_as_context_manager(self):
        """
        Asserts that both TSocket and TServerSocket can be used with `with` and
        that their resources are disposed of at the close of the `with`.
        """
        text = b"hi"  # sample text to send over the wire
        with TSocket.TServerSocket(port=0, family=socket.AF_INET6) as server:
            addr = server.getSocketNames()[0]
            with TSocket.TSocket(host=addr[0], port=addr[1]) as conn:
                conn.write(text)
            self.assertFalse(conn.isOpen())
            with server.accept() as client:
                read = client.read(len(text))
            self.assertFalse(conn.isOpen())
        self.assertFalse(server.isListening())
        self.assertEqual(read, text)

    def test_server_context_errors(self):
        # Make sure the TServerSocket context manager doesn't
        # swallow exceptions
        def do_test():
            with TSocket.TServerSocket(port=0, family=socket.AF_INET6):
                raise Exception("test_error")

        self.assertRaisesRegex(Exception, "test_error", do_test)

    def test_open_failure(self):
        # Bind a server socket to an address, but don't actually listen on it.
        server_socket = socket.socket(socket.AF_INET6)
        try:
            server_socket.bind(("::", 0))
            server_port = server_socket.getsockname()[1]

            # Explicitly use "localhost" as the hostname, so that the
            # connect code will try both IPv6 and IPv4.  We want to
            # exercise the failure behavior when trying multiple addresses.
            sock = TSocket.TSocket(host="localhost", port=server_port)
            sock.setTimeout(50)  # ms
            try:
                sock.open()
                self.fail("unexpectedly succeeded to connect to closed socket")
            except TTransport.TTransportException:
                # sock.open() should not leave the file descriptor open
                # when it fails
                self.assertEqual(None, sock.handle)
                self.assertEqual({}, sock.handles)

                # Calling close() again on the socket should be a no-op,
                # and shouldn't throw an error
                sock.close()
        finally:
            server_socket.close()

    def test_poller_process(self):
        # Make sure that pollers do not fail when they're given None as timeout
        text = "hi"  # sample text to send over the wire
        with TSocket.TServerSocket(port=0, family=socket.AF_INET6) as server:
            addr = server.getSocketNames()[0]

            def write_data():
                # delay writing to verify that poller.process is waiting
                time.sleep(1)
                with TSocket.TSocket(host=addr[0], port=addr[1]) as conn:
                    conn.write(text)

            poller = TSocket.ConnectionSelect()
            thread = threading.Thread(target=write_data)
            thread.start()
            for filenos in server.handles.keys():
                poller.read(filenos)

            r, _, x = poller.process(timeout=None)

            thread.join()
            # Verify that r is non-empty
            self.assertTrue(r)

    def test_deprecated_str_form_of_port(self):
        # Make sure that the deprecated form of the `port` parameter is
        # accepted in TServerSocket and TSocket.
        port = "0"
        text = b"hi"  # sample text to send over the wire
        # NB: unfortunately unittest.TestCase.assertWarns isn't available until
        # py3.
        with TSocket.TServerSocket(port=port, family=socket.AF_INET6) as server:
            addr = server.getSocketNames()[0]
            with TSocket.TSocket(host=addr[0], port=str(addr[1])) as conn:
                conn.write(text)
            with server.accept() as client:
                read = client.read(len(text))
            self.assertEqual(read, text)

    def test_bad_port(self):
        port = "bogus"
        with self.assertRaises(ValueError):
            with TSocket.TServerSocket(port=port):
                pass

        with self.assertRaises(ValueError):
            with TSocket.TSocket(port=port):
                pass

    def test_unix_socket(self):
        text = b"hi"  # sample text to send over the wire
        with tempfile.NamedTemporaryFile(delete=True) as fh:
            unix_socket = fh.name
            with TSocket.TServerSocket(unix_socket=unix_socket) as server:
                with TSocket.TSocket(unix_socket=unix_socket) as conn:
                    conn.write(text)
                with server.accept() as client:
                    read = client.read(len(text))
                self.assertEqual(read, text)
            # The socket will not be cleaned up when the server has been shutdown.
            self.assertTrue(os.path.exists(unix_socket))
