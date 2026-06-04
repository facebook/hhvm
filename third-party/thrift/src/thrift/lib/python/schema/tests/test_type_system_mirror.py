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

"""Drift guards for the hand-rolled discriminant enums in ``type_system.py``.

``Primitive`` and ``PresenceQualifier`` are deliberately independent pure-Python
``enum``s -- the ``:type_system`` library has no dependency on generated
thrift-python code -- but their integer values *mirror* the canonical Thrift
definitions (``type_id.thrift`` / ``type_system.thrift``). These tests make an
upstream change (a renumbered or newly-added primitive / presence qualifier) fail
instead of silently diverging the cross-language ``TypeSystemDigest``.

This is the only test module in the package that depends on the generated
thrift-python types; the core ``type_system`` library and its other tests stay
dependency-free.
"""

import unittest

from apache.thrift.type_system.type_id.thrift_types import TypeId
from apache.thrift.type_system.type_system.thrift_types import (
    PresenceQualifier as CanonicalPresenceQualifier,
)
from thrift.lib.python.schema.type_system import PresenceQualifier, Primitive

# `TypeId` arms 1-10 are the primitives. Arm 11 is the user-defined URI arm and
# 12-14 are containers -- both modeled by the `TypeRef` hierarchy, not `Primitive`.
_PRIMITIVE_ARM_IDS: frozenset[int] = frozenset(range(1, 11))

# The full `TypeId` union shape, pinned so an upstream *addition* (a new
# primitive, a new container kind, ...) trips a test rather than passing silently.
_EXPECTED_TYPE_ID_ARMS: dict[int, str] = {
    1: "boolType",
    2: "byteType",
    3: "i16Type",
    4: "i32Type",
    5: "i64Type",
    6: "floatType",
    7: "doubleType",
    8: "stringType",
    9: "binaryType",
    10: "anyType",
    11: "userDefinedType",
    12: "listType",
    13: "setType",
    14: "mapType",
}


def _arm_to_primitive_name(arm_name: str) -> str:
    """Map a `TypeId` arm name to its `Primitive` member name.

    ``boolType`` -> ``BOOL``, ``i32Type`` -> ``I32``, ``anyType`` -> ``ANY``.
    """
    assert arm_name.endswith("Type"), arm_name
    return arm_name[: -len("Type")].upper()


class PrimitiveMirrorTest(unittest.TestCase):
    def test_members_match_type_id_primitive_arms(self) -> None:
        canonical = {
            _arm_to_primitive_name(arm.name): arm.value
            for arm in TypeId.Type
            if arm.value in _PRIMITIVE_ARM_IDS
        }
        ours = {member.name: member.value for member in Primitive}
        self.assertEqual(ours, canonical)

    def test_type_id_union_shape_is_pinned(self) -> None:
        # `EMPTY` (value 0) is the thrift-python union "unset" sentinel, not a
        # real arm; everything else must match the canonical union exactly.
        actual = {arm.value: arm.name for arm in TypeId.Type if arm.value != 0}
        self.assertEqual(actual, _EXPECTED_TYPE_ID_ARMS)


class PresenceQualifierMirrorTest(unittest.TestCase):
    def test_members_match_canonical_enum(self) -> None:
        # The canonical enum carries an extra DEFAULT_INITIALIZED=0 "invalid
        # state" sentinel that the runtime model deliberately omits; every other
        # member must match name -> value exactly.
        canonical = {
            member.name: member.value
            for member in CanonicalPresenceQualifier
            if member.value != 0
        }
        ours = {member.name: member.value for member in PresenceQualifier}
        self.assertEqual(ours, canonical)

    def test_only_intentional_divergence_is_the_zero_sentinel(self) -> None:
        canonical_names = {member.name for member in CanonicalPresenceQualifier}
        our_names = {member.name for member in PresenceQualifier}
        self.assertEqual(canonical_names - our_names, {"DEFAULT_INITIALIZED"})
