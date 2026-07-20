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

from enum import Enum

class Protocol(Enum):
    # pyrefly: ignore [invalid-annotation]
    BINARY: Protocol = ...
    # Do not use DEPRECATED_VERBOSE_JSON
    # pyrefly: ignore [invalid-annotation]
    DEPRECATED_VERBOSE_JSON: Protocol = ...
    # pyrefly: ignore [invalid-annotation]
    COMPACT: Protocol = ...
    # pyrefly: ignore [invalid-annotation]
    JSON: Protocol = ...
    # pyrefly: ignore [invalid-annotation]
    JSON5: Protocol = ...

class RpcKind(Enum):
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_SINGLE_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_NO_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_STREAMING_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINK: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    BIDIRECTIONAL_STREAM: RpcKind = ...
