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

from __future__ import absolute_import, division, print_function, unicode_literals

import unittest

import thrift.test.py.constants.thrift_types as constants_python
import thrift.test.py.constants.types as constants_py3

from .constants import constants as constants_py

CONSTANTS = [("py", constants_py), ("py3", constants_py3), ("python", constants_python)]


class TestPythonConstants(unittest.TestCase):
    def testStrings(self):
        for which, constants in CONSTANTS:
            with self.subTest(which=which):
                self.assertEqual(constants.apostrophe, "'")
                self.assertEqual(constants.tripleApostrophe, "'''")
                self.assertEqual(constants.quotationMark, '"')
                self.assertEqual(constants.quote, 'this is a "quote"')
                self.assertEqual(constants.backslash, "\\")
                self.assertEqual(constants.escaped_a, "a")

    def testDict(self):
        for which, constants in CONSTANTS:
            with self.subTest(which=which):
                self.assertEqual(constants.escapeChars["apostrophe"], "'")
                self.assertEqual(constants.escapeChars["quotationMark"], '"')
                self.assertEqual(constants.escapeChars["backslash"], "\\")
                self.assertEqual(constants.escapeChars["escaped_a"], "a")
                self.assertEqual(constants.escapeChars["newline"], "\n")
                self.assertEqual(constants.escapeChars["questionMark"], "?")
                self.assertEqual(constants.char2ascii["'"], 39)
                self.assertEqual(constants.char2ascii['"'], 34)
                self.assertEqual(constants.char2ascii["\\"], 92)
                self.assertEqual(constants.char2ascii["a"], 97)
                self.assertEqual(constants.char2ascii["\n"], 10)
                self.assertEqual(constants.char2ascii["?"], 63)

    def testStruct(self):
        for which, constants in CONSTANTS:
            with self.subTest(which=which):
                self.assertEqual(constants.str2struct["foo"].bar, {"baz": "qux"})


if __name__ == "__main__":
    unittest.main()
