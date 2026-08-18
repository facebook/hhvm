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

import typing as _typing

class CppContextData: ...

class CallbackWrapper:
    def call(self, obj: object) -> None: ...

class CppServerWrapper:
    def setAddress(self, ip: _typing.Union[bytes, str], port: int) -> None: ...
    def getAddress(
        self,
    ) -> _typing.Union[
        _typing.Tuple[str, int], _typing.Tuple[str, int, int, int], str
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
    DISABLED: _typing.ClassVar[SSLPolicy]
    PERMITTED: _typing.ClassVar[SSLPolicy]
    REQUIRED: _typing.ClassVar[SSLPolicy]

class VerifyClientCertificate(int):
    IF_PRESENTED: _typing.ClassVar[VerifyClientCertificate]
    ALWAYS_VERIFY: _typing.ClassVar[VerifyClientCertificate]
    NONE_DO_NOT_REQUEST: _typing.ClassVar[VerifyClientCertificate]

class SSLVersion(int):
    TLSv1_2: _typing.ClassVar[SSLVersion]
