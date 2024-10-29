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

import os
from enum import Enum
from typing import Any, Union

Path = Union[str, bytes, os.PathLike[str], os.PathLike[bytes]]

class SSLVersion(Enum):
    TLSv1_2: SSLVersion = ...
    @property
    def value(self) -> int: ...

class SSLVerifyOption(Enum):
    VERIFY: SSLVerifyOption = ...
    VERIFY_REQ_CLIENT_CERT: SSLVerifyOption = ...
    NO_VERIFY: SSLVerifyOption = ...

class SSLContext:
    def __init__(self, version: SSLVersion = ...) -> None: ...
    def set_verify_option(self, option: SSLVerifyOption) -> None: ...
    @property
    def needs_peer_verify(self) -> bool: ...
    def load_cert_chain(self, *, certfile: Path, keyfile: Path) -> None: ...
    def load_verify_locations(self, *, cafile: Path) -> None: ...
    def authenticate(self, *, peer_cert: bool, peer_name: bool) -> None: ...
