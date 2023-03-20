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

import typing as __typing

class CppContextData: ...

class CallbackWrapper:
    def call(self, obj: object) -> None: ...

class CppServerWrapper:
    def setAddress(self, ip: __typing.Union[bytes, str], port: int) -> None: ...
    def getAddress(
        self,
    ) -> __typing.Union[
        __typing.Tuple[str, int], __typing.Tuple[str, int, int, int], str
    ]: ...
    def setUnixSocketPath(self, ip: str) -> None: ...
    def setNumCPUWorkerThreads(self, num_threads: int) -> None: ...
    def setNumIOWorkerThreads(self, num_threads: int) -> None: ...
    def setPort(self, port: int) -> None: ...
    def stop(self) -> None: ...
    def cleanUp(self) -> None: ...
    def setEnable(self, enable: bool) -> None: ...
    def setIdleTimeout(self, timeout: int) -> None: ...
    def getPort(self) -> int: ...

class CallTimestamps: ...

class SSLPolicy(int):
    DISABLED: __typing.ClassVar[SSLPolicy]
    PERMITTED: __typing.ClassVar[SSLPolicy]
    REQUIRED: __typing.ClassVar[SSLPolicy]

class VerifyClientCertificate(int):
    IF_PRESENTED: __typing.ClassVar[VerifyClientCertificate]
    ALWAYS_VERIFY: __typing.ClassVar[VerifyClientCertificate]
    NONE_DO_NOT_REQUEST: __typing.ClassVar[VerifyClientCertificate]

class SSLVersion(int):
    TLSv1_2: __typing.ClassVar[SSLVersion]
