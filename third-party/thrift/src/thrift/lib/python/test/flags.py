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

import unittest

from thrift.python.flags import (
    define_bool_flag,
    define_int_flag,
    get_bool_flag,
    get_int_flag,
    mock_bool_flag,
    mock_int_flag,
)


class FlagsTest(unittest.TestCase):
    def test_int_flag(self) -> None:
        define_int_flag("foo", 42)
        with mock_int_flag("foo", 404):
            self.assertEqual(404, get_int_flag("foo"))
        self.assertEqual(42, get_int_flag("foo"))

    def test_bool_flag(self) -> None:
        define_bool_flag("bar", True)
        with mock_bool_flag("bar", False):
            self.assertFalse(get_bool_flag("bar"))
        self.assertTrue(get_bool_flag("bar"))

    def test_double_define_ignored(self) -> None:
        define_int_flag("foo", 42)
        define_int_flag("foo", 404)
        self.assertEqual(42, get_int_flag("foo"))

    def test_non_existent_flag(self) -> None:
        with self.assertRaises(KeyError):
            get_int_flag("ghost")
