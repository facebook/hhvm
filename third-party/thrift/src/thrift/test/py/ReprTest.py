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

from __future__ import absolute_import, division, print_function, unicode_literals

import textwrap
import unittest

from thrift.test.UnionTest.ttypes import (
    OneOfEach,
    RandomStuff,
    StructWithDoubleUnderscoreField,
    StructWithUnionAndOther,
    TestUnion,
)


class TestRepr(unittest.TestCase):
    def assertReprEquals(self, obj):
        self.assertEqual(obj, eval(repr(obj)))

    def test_repr(self):
        """Ensure that __repr__() return a valid expression that can be
        used to construct the original object
        """

        self.assertReprEquals(RandomStuff(bigint=123456))

        self.assertReprEquals(StructWithUnionAndOther(string_field="test"))

        self.assertReprEquals(TestUnion(string_field="blah"))

    def test_content(self):
        """Ensure that the content of repr() is what we wanted. We should
        print the members in the same order as it is appeared in Thrift file,
        skipping unset members.
        """
        obj = RandomStuff(bigint=123)
        output = """\
            RandomStuff(
                bigint=123)"""
        self.assertEqual(repr(obj), textwrap.dedent(output))

    def test_defaults(self):
        """Ensure repr() includes fields which have default values."""
        obj = OneOfEach()
        output = """\
            OneOfEach(
                a_bite=100,
                integer16=23000,
                integer64=10000000000,
                byte_list=[1, 2, 3],
                i16_list=[1, 2, 3],
                i64_list=[1, 2, 3])"""
        self.assertEqual(repr(obj), textwrap.dedent(output))

    def test_double_underscore_field_repr(self):
        obj = StructWithDoubleUnderscoreField(__field=123, field=456)
        output = """\
            StructWithDoubleUnderscoreField(
                __field=123,
                field=456)"""
        self.assertEqual(repr(obj), textwrap.dedent(output))
