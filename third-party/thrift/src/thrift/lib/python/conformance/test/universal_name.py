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

import unittest

from thrift.python.conformance.universal_name import (
    contains_universal_hash,
    find_by_universal_hash,
    get_universal_hash,
    get_universal_hash_prefix,
    get_universal_hash_size,
    matches_universal_hash,
    maybe_get_universal_hash_prefix,
    UniversalHashAlgorithm,
    validate_universal_hash,
    validate_universal_hash_bytes,
    validate_universal_name,
)


class UniversalNameTests(unittest.TestCase):
    def test_validate_universal_name(self) -> None:
        validate_universal_name("foo.com/my/type")
        with self.assertRaises(ValueError):
            validate_universal_name("foo.com/my/type#1")

    def test_validate_universal_hash(self) -> None:
        validate_universal_hash(UniversalHashAlgorithm.Sha2_256, b"a" * 32, 8)
        with self.assertRaises(ValueError):
            validate_universal_hash(UniversalHashAlgorithm.Sha2_256, b"b" * 1, 8)
        with self.assertRaises(ValueError):
            validate_universal_hash(UniversalHashAlgorithm.Sha2_256, b"c" * 33, 8)

    def test_validate_universal_hash_bytes(self) -> None:
        validate_universal_hash_bytes(32, 8)
        with self.assertRaises(ValueError):
            validate_universal_hash_bytes(1, 8)

    def test_get_universal_hash_size(self) -> None:
        self.assertEqual(get_universal_hash_size(UniversalHashAlgorithm.Sha2_256), 32)

    def test_get_universal_hash(self) -> None:
        self.assertEqual(
            get_universal_hash(UniversalHashAlgorithm.Sha2_256, "foo.com/my/type"),
            b"\tat$\x9c\xef\xad\xb5\xea\rE;\xcb3\xadTv\x01\xfb\xfe\xc4\xb2\xd7\x95\x92N\xebg\xd4[\xe6F",
        )

    def test_get_universal_hash_prefix(self) -> None:
        self.assertEqual(get_universal_hash_prefix(b"a" * 32, 0), b"")
        self.assertEqual(get_universal_hash_prefix(b"b" * 32, 8), b"b" * 8)
        self.assertEqual(get_universal_hash_prefix(b"c" * 32, 32), b"c" * 32)
        self.assertEqual(get_universal_hash_prefix(b"d" * 32, 33), b"d" * 32)

    def test_maybe_get_universal_hash_prefix(self) -> None:
        self.assertEqual(
            len(
                maybe_get_universal_hash_prefix(
                    UniversalHashAlgorithm.Sha2_256, "a" * 24, 0
                )
            ),
            0,
        )
        self.assertEqual(
            len(
                maybe_get_universal_hash_prefix(
                    UniversalHashAlgorithm.Sha2_256, "a" * 24, 8
                )
            ),
            8,
        )
        self.assertEqual(
            len(
                maybe_get_universal_hash_prefix(
                    UniversalHashAlgorithm.Sha2_256, "a" * 24, 23
                )
            ),
            23,
        )
        self.assertEqual(
            len(
                maybe_get_universal_hash_prefix(
                    UniversalHashAlgorithm.Sha2_256, "a" * 24, 24
                )
            ),
            0,
        )
        self.assertEqual(
            len(
                maybe_get_universal_hash_prefix(
                    UniversalHashAlgorithm.Sha2_256, "a" * 48, 32
                )
            ),
            32,
        )
        self.assertEqual(
            len(
                maybe_get_universal_hash_prefix(
                    UniversalHashAlgorithm.Sha2_256, "a" * 48, 33
                )
            ),
            32,
        )

    def test_match_universal_hash(self) -> None:
        self.assertFalse(
            matches_universal_hash(b"0123456789ABCDEF0123456789ABCDEF", b"")
        )
        self.assertFalse(
            matches_universal_hash(b"0123456789ABCDEF0123456789ABCDEF", b"1")
        )
        self.assertTrue(
            matches_universal_hash(b"0123456789ABCDEF0123456789ABCDEF", b"0")
        )
        self.assertTrue(
            matches_universal_hash(
                b"0123456789ABCDEF0123456789ABCDEF", b"0123456789ABCDEF"
            )
        )
        self.assertTrue(
            matches_universal_hash(
                b"0123456789ABCDEF0123456789ABCDEF", b"0123456789ABCDEF0123456789ABCDEF"
            )
        )
        self.assertFalse(
            matches_universal_hash(
                b"0123456789ABCDEF0123456789ABCDEF",
                b"0123456789ABCDEF0123456789ABCDEF0",
            )
        )

    def test_contains_universal_hash_and_find_by_universal_hash(self) -> None:
        registry = {
            b"1233": 0,
            b"1234": 1,
            b"12345": 2,
            b"1235": 3,
        }
        self.assertTrue(contains_universal_hash(registry, b"1"))
        with self.assertRaises(ValueError):
            find_by_universal_hash(registry, b"1")

        self.assertTrue(contains_universal_hash(registry, b"12"))
        with self.assertRaises(ValueError):
            find_by_universal_hash(registry, b"12")

        self.assertTrue(contains_universal_hash(registry, b"123"))
        with self.assertRaises(ValueError):
            find_by_universal_hash(registry, b"123")

        self.assertTrue(contains_universal_hash(registry, b"1234"))
        with self.assertRaises(ValueError):
            find_by_universal_hash(registry, b"1234")

        self.assertFalse(contains_universal_hash(registry, b""))
        with self.assertRaises(KeyError):
            find_by_universal_hash(registry, b"")

        self.assertFalse(contains_universal_hash(registry, b"0"))
        with self.assertRaises(KeyError):
            find_by_universal_hash(registry, b"0")

        self.assertTrue(contains_universal_hash(registry, b"1233"))
        self.assertEqual(find_by_universal_hash(registry, b"1233"), 0)

        self.assertFalse(contains_universal_hash(registry, b"12333"))
        with self.assertRaises(KeyError):
            find_by_universal_hash(registry, b"12333")

        self.assertTrue(contains_universal_hash(registry, b"12345"))
        self.assertEqual(find_by_universal_hash(registry, b"12345"), 2)

        self.assertFalse(contains_universal_hash(registry, b"12346"))
        with self.assertRaises(KeyError):
            find_by_universal_hash(registry, b"12346")

        self.assertTrue(contains_universal_hash(registry, b"1235"))
        self.assertEqual(find_by_universal_hash(registry, b"1235"), 3)

        self.assertFalse(contains_universal_hash(registry, b"2"))
        with self.assertRaises(KeyError):
            find_by_universal_hash(registry, b"2")
