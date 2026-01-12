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

from thrift.python.server_impl.interceptor.service_interceptor import (
    AbstractServiceInterceptor,
    ConnectionInfo,
    RequestInfo,
    ResponseInfo,
)


class ConnectionState:
    def __init__(self, info: ConnectionInfo) -> None:
        self.info: ConnectionInfo = info


class RequestState:
    def __init__(self, info: RequestInfo) -> None:
        self.info: RequestInfo = info


class CountingInterceptor(AbstractServiceInterceptor[ConnectionState, RequestState]):
    class Counts:
        def __init__(self) -> None:
            self.onConnect: int = 0
            self.onConnectClosed: int = 0
            self.onRequest: int = 0
            self.onResponse: int = 0

    def __init__(self) -> None:
        self._counts = CountingInterceptor.Counts()
        self.services: set[str] = set()
        self.defining_services: set[str] = set()
        self.methods: set[str] = set()
        self.connection_states: set[ConnectionState] = set()
        self.request_states: set[RequestState] = set()

    @property
    def counts(self) -> Counts:
        return self._counts

    def onConnection(self, connection_info: ConnectionInfo) -> ConnectionState:
        self._counts.onConnect += 1
        state = ConnectionState(connection_info)
        self.connection_states.add(state)
        return state

    def onConnectionClosed(
        self, connection_state: ConnectionState, connection_info: ConnectionInfo
    ) -> None:
        self._counts.onConnectClosed += 1
        assert connection_state in self.connection_states, "connection_state unknown"
        assert connection_state.info != connection_info, (
            "connection_state unexpected match"
        )

    def onRequest(
        self, connection_state: ConnectionState, request_info: RequestInfo
    ) -> RequestState:
        self.counts.onRequest += 1
        self.services.add(request_info.service_name)
        self.defining_services.add(request_info.defining_service_name)
        self.methods.add(request_info.method_name)

        assert connection_state in self.connection_states

        state = RequestState(request_info)
        self.request_states.add(state)
        return state

    def onResponse(
        self,
        request_state: RequestState,
        connection_state: ConnectionState,
        response_info: ResponseInfo,
    ) -> None:
        self._counts.onResponse += 1

        assert connection_state in self.connection_states, "connection_state unknown"
        assert request_state in self.request_states, "request_state unknown"
        assert response_info is not None, "response_info is not None"
        assert response_info.exception is None, "exception should be None"
        assert response_info.service_name == request_state.info.service_name, (
            "service name mismatch"
        )
        assert (
            response_info.defining_service_name
            == request_state.info.defining_service_name
        ), "service name mismatch"
        assert response_info.method_name == request_state.info.method_name, (
            "method name mismatch"
        )


class OnConnectThrowsInterceptor(
    AbstractServiceInterceptor[ConnectionState, RequestState]
):
    def __init__(self) -> None:
        self.on_connection_throws: int = 0

    def onConnection(self, connection_info: ConnectionInfo) -> ConnectionState:
        self.on_connection_throws += 1
        raise RuntimeError("Expect the unexpected")


class OnRequestThrowsInterceptor(
    AbstractServiceInterceptor[ConnectionState, RequestState]
):
    def __init__(self) -> None:
        self.on_request_throws: int = 0
        self.on_response: int = 0
        self.response_errors: list[str] = []

    def onRequest(
        self, connection_state: ConnectionState, request_info: RequestInfo
    ) -> RequestState:
        self.on_request_throws += 1
        raise RuntimeError("Expect the unexpected")

    def onResponse(
        self,
        request_state: RequestState,
        connection_state: ConnectionState,
        response_info: ResponseInfo,
    ) -> None:
        self.on_response += 1
        if response_info.exception is not None:
            self.response_errors.append(response_info.exception)


class OnResponseThrowsInterceptor(
    AbstractServiceInterceptor[ConnectionState, RequestState]
):
    def __init__(self) -> None:
        self.on_response_throws: int = 0

    def onResponse(
        self,
        request_state: RequestState,
        connection_state: ConnectionState,
        response_info: ResponseInfo,
    ) -> None:
        self.on_response_throws += 1
        raise RuntimeError("Expect the unexpected")
