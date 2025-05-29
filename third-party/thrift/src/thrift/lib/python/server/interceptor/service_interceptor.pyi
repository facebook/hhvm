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

from typing import Final, Generic, TypeVar

ConnectionState = TypeVar("ConnectionState")
RequestState = TypeVar("RequestState")

class AbstractServiceInterceptor(Generic[ConnectionState, RequestState]):
    def onConnection(self, connection_info: ConnectionInfo) -> ConnectionState: ...
    def onRequest(
        self, connection_state: ConnectionState, request_info: RequestInfo
    ) -> RequestState: ...
    def onResponse(
        self,
        request_state: RequestState,
        connection_state: ConnectionState,
        response_info: ResponseInfo,
    ) -> None: ...
    def onConnectionClosed(
        self, connection_state: ConnectionState, connection_info: ConnectionInfo
    ) -> None: ...

class PyObservableServiceInterceptor:
    def __init__(
        self, impl: AbstractServiceInterceptor[ConnectionState, RequestState]
    ) -> None: ...

class ConnectionInfo:
    pass

class RequestInfo:
    service_name: Final[str]
    defining_service_name: Final[str]
    method_name: Final[str]

class ResponseInfo:
    service_name: Final[str]
    defining_service_name: Final[str]
    method_name: Final[str]
    exception: Final[str | None]
