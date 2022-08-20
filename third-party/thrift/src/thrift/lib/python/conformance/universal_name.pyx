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

# cython: c_string_type=unicode, c_string_encoding=utf8

from folly.range cimport to_bytes


def validate_universal_name(uri: str) -> None:
    validateUniversalName(uri)


def validate_universal_hash(
    alg: UniversalHashAlgorithm, universal_hash: bytes, min_hash_bytes: int
) -> None:
    validateUniversalHash(alg, universal_hash, min_hash_bytes)


def validate_universal_hash_bytes(hash_bytes: int, min_hash_bytes: int):
    validateUniversalHashBytes(hash_bytes, min_hash_bytes)


def get_universal_hash_size(alg: UniversalHashAlgorithm) -> int:
    return getUniversalHashSize(alg)


def get_universal_hash(alg: UniversalHashAlgorithm, uri: str) -> bytes:
    return getUniversalHash(alg, uri).toStdString()


def get_universal_hash_prefix(universal_hash: bytes, hash_bytes: int) -> bytes:
    return to_bytes(getUniversalHashPrefix(universal_hash, hash_bytes))


def maybe_get_universal_hash_prefix(
    alg: UniversalHashAlgorithm, uri: str, hash_bytes: int
) -> bytes:
    return maybeGetUniversalHashPrefix(alg, uri, hash_bytes).toStdString()


def matches_universal_hash(universal_hash: bytes, prefix: bytes) -> bool:
    return matchesUniversalHash(universal_hash, prefix)


def contains_universal_hash(
    universal_hash_registry: typing.Dict[bytes, typing.Any],
    universal_hash_prefix: bytes,
) -> bool:
    return any(
        matches_universal_hash(k, universal_hash_prefix)
        for k in universal_hash_registry
    )


def find_by_universal_hash(
    universal_hash_registry: typing.Dict[bytes, typing.Any],
    universal_hash_prefix: bytes,
) -> bool:
    matched = (
        v
        for k, v in universal_hash_registry.items()
        if matches_universal_hash(k, universal_hash_prefix)
    )
    try:
        ret = next(matched)
        try:
            next(matched)
            raise ValueError(f"multiple keys found with prefix {universal_hash_prefix}")
        except StopIteration:
            pass
        return ret
    except StopIteration:
        raise KeyError(f"no key found with prefix {universal_hash_prefix}")
