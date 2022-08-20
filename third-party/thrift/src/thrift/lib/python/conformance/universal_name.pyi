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

import typing
from enum import Enum

class UniversalHashAlgorithm(Enum):
    Sha2_256: UniversalHashAlgorithm = ...

# Validates that uri is a valid universal name uri of the form:
# {domain}/{path}. For example: facebook.com/thrift/Value.
#
# The scheme "fbthrift://"" is implied and not included in the uri.
#
# Throws ValueError on failure.
def validate_universal_name(uri: str) -> None: ...

# Validates that the given type hash meets the size requirements.
#
# Throws ValueError on failure.
def validate_universal_hash(
    alg: UniversalHashAlgorithm, universal_hash: bytes, min_hash_bytes: int
) -> None: ...

# Validates that the given type hash bytes size meets size requirements.
#
# Throws std::invalid_argument on failure.
def validate_universal_hash_bytes(hash_bytes: int, min_hash_bytes: int) -> None: ...

# The number of bytes returned by the given type hash algorithm.
def get_universal_hash_size(alg: UniversalHashAlgorithm) -> int: ...

# Returns the hash for the given universal name uri.
#
# The hash includes the implied scheme, "fbthrift://".
def get_universal_hash(alg: UniversalHashAlgorithm, uri: str) -> bytes: ...

# Shrinks the universal_hash to fit in the given number of bytes.
def get_universal_hash_prefix(universal_hash: bytes, hash_bytes: int) -> bytes: ...

# Returns the type hash prefix iff smaller than the uri.
def maybe_get_universal_hash_prefix(
    alg: UniversalHashAlgorithm, uri: str, hash_bytes: int
) -> bytes: ...

# Returns true iff prefix was derived from universal_hash.
def matches_universal_hash(universal_hash: bytes, prefix: bytes) -> bool: ...

T = typing.TypeVar("T")

# Returns true, iff the given sorted map contains an entry that matches the
# given type hash prefix.
def contains_universal_hash(
    universal_hash_registry: typing.Dict[bytes, T],
    universal_hash_prefix: bytes,
) -> bool: ...

# Finds a matching hash within the given sorted map.
#
# Raises a KeyError if not found.
# Raises a ValueError if the result is ambiguous.
def find_by_universal_hash(
    universal_hash_registry: typing.Dict[bytes, T], universal_hash_prefix: bytes
) -> T: ...
