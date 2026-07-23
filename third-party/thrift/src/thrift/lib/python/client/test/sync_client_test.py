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

# pyre-strict


from __future__ import annotations

import socket
import threading
import time
from unittest import IsolatedAsyncioTestCase, mock

from thrift.lib.python.client.test.client_event_handler.helper import (
    TestHelper as ClientEventHandlerTestHelper,
)
from thrift.lib.python.client.test.event_handler_helper import (
    addEventHandler,
    client_handler_that_throws,
)
from thrift.lib.python.client.test.test_server import server_in_another_process
from thrift.python.client import ClientType, get_sync_client
from thrift.python.common import RpcOptions
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    TransportError,
    TransportErrorType,
)
from thrift.python.leaf.thrift_clients import LeafService
from thrift.python.serializer import Protocol
from thrift.python.test.thrift_clients import EchoService, TestService
from thrift.python.test.thrift_types import ArithmeticException, EmptyException

from .exceptions_helper import HijackTestException, HijackTestHelper


TEST_HEADER_KEY = "headerKey"
TEST_HEADER_VALUE = "headerValue"
HTTP_TEST_HOST_A = "host-a.example.com"
HTTP_TEST_HOST_B = "host-b.example.com"
HTTP_ROUTE_HOST_A = "host-a"
HTTP_ROUTE_HOST_B = "host-b"
HTTP_ROUTE_MISSING_HOST = "missing-host"
HTTP_ROUTE_UNKNOWN_HOST = "unknown-host"


def _extract_http_host_header(raw_request: bytes) -> str | None:
    for line in raw_request.decode("latin1").split("\r\n"):
        if line.lower().startswith("host:"):
            return line.split(":", 1)[1].strip()
    return None


def _route_for_http_host(host_header: str | None) -> str:
    if host_header is None:
        return HTTP_ROUTE_MISSING_HOST

    hostname = host_header.split(":", 1)[0]
    if hostname == HTTP_TEST_HOST_A:
        return HTTP_ROUTE_HOST_A
    if hostname == HTTP_TEST_HOST_B:
        return HTTP_ROUTE_HOST_B
    return HTTP_ROUTE_UNKNOWN_HOST


def _route_raw_http_request(raw_request: bytes) -> tuple[str, str | None]:
    host_header = _extract_http_host_header(raw_request)
    return _route_for_http_host(host_header), host_header


def _serve_one_http_request(
    listener: socket.socket,
    selected_routes: list[tuple[str, str | None]],
    errors: list[Exception],
) -> None:
    try:
        connection, _ = listener.accept()
        with connection:
            raw_request = b""
            while b"\r\n\r\n" not in raw_request:
                chunk = connection.recv(4096)
                if not chunk:
                    break
                raw_request += chunk
            selected_routes.append(_route_raw_http_request(raw_request))
            connection.sendall(
                b"HTTP/1.1 404 Not Found\r\n"
                b"Content-Length: 0\r\n"
                b"Connection: close\r\n\r\n"
            )
    except Exception as ex:
        errors.append(ex)


class SyncClientTests(IsolatedAsyncioTestCase):
    def test_basic(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertEqual(3, client.add(1, 2))

    def test_client_type_and_protocol(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(
                TestService,
                path=path,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
                protocol=Protocol.BINARY,
            ) as client:
                sum = client.add(1, 2)
                self.assertEqual(3, sum)

    def test_http_host_header_selects_virtual_host_route(self) -> None:
        self.assertEqual(
            (HTTP_ROUTE_MISSING_HOST, None),
            _route_raw_http_request(b"GET / HTTP/1.1\r\n\r\n"),
        )
        self.assertEqual(
            (HTTP_ROUTE_HOST_A, HTTP_TEST_HOST_A),
            _route_raw_http_request(
                b"GET / HTTP/1.1\r\nHost: host-a.example.com\r\n\r\n"
            ),
        )
        self.assertEqual(
            (HTTP_ROUTE_HOST_B, HTTP_TEST_HOST_B),
            _route_raw_http_request(
                b"GET / HTTP/1.1\r\nHost: host-b.example.com\r\n\r\n"
            ),
        )
        self.assertEqual(
            (HTTP_ROUTE_UNKNOWN_HOST, "127.0.0.1"),
            _route_raw_http_request(b"GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"),
        )

    def test_http_client_preserves_hostname_for_virtual_host_routing(self) -> None:
        listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listener.settimeout(2)
        listener.bind(("127.0.0.1", 0))
        listener.listen(1)
        port = listener.getsockname()[1]
        selected_routes: list[tuple[str, str | None]] = []
        errors: list[Exception] = []
        thread = threading.Thread(
            target=_serve_one_http_request,
            args=(listener, selected_routes, errors),
        )
        thread.start()

        try:
            with mock.patch(
                "socket.getaddrinfo",
                return_value=[
                    (
                        socket.AF_INET,
                        socket.SOCK_STREAM,
                        socket.IPPROTO_TCP,
                        "",
                        ("127.0.0.1", port),
                    )
                ],
            ):
                with get_sync_client(
                    TestService,
                    host=HTTP_TEST_HOST_A,
                    port=port,
                    path="/",
                    client_type=ClientType.THRIFT_HTTP_CLIENT_TYPE,
                ) as client:
                    with self.assertRaises(TransportError):
                        client.add(1, 2)
        finally:
            thread.join(timeout=2)
            listener.close()

        self.assertFalse(thread.is_alive())
        if errors:
            raise errors[0]
        self.assertEqual([(HTTP_ROUTE_HOST_A, HTTP_TEST_HOST_A)], selected_routes)

    def test_void_return(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertIsNone(client.noop())

    def test_exception(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertAlmostEqual(2, client.divide(6, 3))
                with self.assertRaises(ArithmeticException):
                    client.divide(1, 0)

    def test_void_return_with_exception(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                with self.assertRaises(EmptyException):
                    client.oops()

    def test_oneway(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertIsNone(client.oneway())
                time.sleep(1)  # wait for server to clear the queue

    def test_oneway_with_rocket(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(
                TestService, path=path, client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE
            ) as client:
                self.assertIsNone(client.oneway())
                time.sleep(1)  # wait for server to clear the queue

    def test_keep_alive_timeout_ms_with_rocket(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(
                TestService,
                path=path,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
                keep_alive_timeout_ms=5000,
            ) as client:
                self.assertEqual(3, client.add(1, 2))

    def test_unexpected_exception(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                with self.assertRaisesRegex(ApplicationError, "Surprise!") as ex:
                    client.surprise()
                self.assertEqual(ApplicationErrorType.UNKNOWN, ex.exception.type)

    def test_derived_service(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(EchoService, path=path) as client:
                self.assertEqual("hello", client.echo("hello"))
                self.assertEqual(3, client.add(1, 2))

    def test_deriving_from_external_service(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(LeafService, path=path) as client:
                self.assertEqual([3, 2, 1], list(client.reverse([1, 2, 3])))
                self.assertEqual("hello", client.echo("hello"))
                self.assertEqual(3, client.add(1, 2))

    def test_transport_error(self) -> None:
        with get_sync_client(TestService, host="localhost", port=1) as client:
            with self.assertRaises(TransportError) as ex:
                client.add(1, 2)
            self.assertEqual(TransportErrorType.UNKNOWN, ex.exception.type)

    def test_persistent_header(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                client.set_persistent_header(TEST_HEADER_KEY, TEST_HEADER_VALUE)
                self.assertEqual(TEST_HEADER_VALUE, client.readHeader(TEST_HEADER_KEY))

    def test_reuse_client(self) -> None:
        with server_in_another_process() as path:
            client = get_sync_client(TestService, path=path)
            with client:
                self.assertEqual(3, client.add(1, 2))
            with self.assertRaises(RuntimeError):
                with client:
                    pass
            with self.assertRaises(RuntimeError):
                client.add(1, 2)

    def test_add_test_handler_with_rpc_options_should_hijack_transport_error_and_use_rpc_options(
        self,
    ) -> None:
        # pyrefly: ignore [bad-context-manager]
        with HijackTestHelper():
            with get_sync_client(TestService, host="localhost", port=1) as client:
                with self.assertRaises(HijackTestException) as context:
                    options = RpcOptions()
                    options.timeout = 12.5
                    client.add(1, 2, rpc_options=options)
                self.assertEqual(context.exception.timeout, 12.5)

    def test_add_test_handler_without_rpc_options_should_hijack_transport_error(
        self,
    ) -> None:
        # pyrefly: ignore [bad-context-manager]
        with HijackTestHelper():
            with get_sync_client(TestService, host="localhost", port=1) as client:
                with self.assertRaises(HijackTestException) as context:
                    client.add(1, 2)
                self.assertEqual(context.exception.timeout, 0.0)

    def test_exit_callback(self) -> None:
        class Callback:
            def __init__(self):
                self.triggered = False

            def trigger(self):
                self.triggered = True

        cb1 = Callback()
        cb2 = Callback()

        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                client._at_exit(cb1.trigger)
                client._at_exit(cb2.trigger)

        self.assertTrue(cb1.triggered)
        self.assertTrue(cb2.triggered)

    async def test_client_event_handler(self) -> None:
        test_helper: ClientEventHandlerTestHelper = ClientEventHandlerTestHelper()
        self.assertFalse(test_helper.is_handler_called())
        with test_helper.get_sync_client(TestService, port=1) as cli:
            try:
                cli.noop()
            except TransportError:
                pass
            self.assertTrue(test_helper.is_handler_called())

    def test_exception_in_client_event_handler(self) -> None:
        with self.assertRaises(RuntimeError):
            with client_handler_that_throws():
                with server_in_another_process() as path:
                    with get_sync_client(TestService, path=path) as client:
                        self.assertEqual(3, client.add(1, 2))

    def test_no_exception_when_clear_event_handler(self) -> None:
        addEventHandler()
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                client.clear_event_handlers()
                self.assertEqual(3, client.add(1, 2))

    def test_protocol_id_retrieval_with_binary_protocol(self) -> None:
        """Test that getChannelProtocolId works correctly with binary protocol.

        This verifies that the folly::Try<uint16_t> return type from
        getChannelProtocolId() is properly handled in the Cython layer.
        """
        with server_in_another_process() as path:
            with get_sync_client(
                TestService,
                path=path,
                protocol=Protocol.BINARY,
            ) as client:
                # If getChannelProtocolId() failed, this call would raise
                # an exception before the request is sent
                result = client.add(1, 2)
                self.assertEqual(3, result)

    def test_protocol_id_retrieval_with_compact_protocol(self) -> None:
        """Test that getChannelProtocolId works correctly with compact protocol.

        This verifies that the folly::Try<uint16_t> return type from
        getChannelProtocolId() is properly handled in the Cython layer.
        """
        with server_in_another_process() as path:
            with get_sync_client(
                TestService,
                path=path,
                protocol=Protocol.COMPACT,
            ) as client:
                # If getChannelProtocolId() failed, this call would raise
                # an exception before the request is sent
                result = client.add(1, 2)
                self.assertEqual(3, result)
