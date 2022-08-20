# Copyright (c) Facebook, Inc. and its affiliates.
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

import unittest


class TestPythonConstants(unittest.TestCase):
    def testStrings(self):
        from .constants import constants

        self.assertEquals(constants.apostrophe, "'")
        self.assertEquals(constants.tripleApostrophe, "'''")
        self.assertEquals(constants.quotationMark, '"')
        self.assertEquals(constants.quote, 'this is a "quote"')
        self.assertEquals(constants.backslash, "\\")
        self.assertEquals(constants.escaped_a, "a")

    def testDict(self):
        from .constants import constants

        self.assertEquals(constants.escapeChars["apostrophe"], "'")
        self.assertEquals(constants.escapeChars["quotationMark"], '"')
        self.assertEquals(constants.escapeChars["backslash"], "\\")
        self.assertEquals(constants.escapeChars["escaped_a"], "a")
        self.assertEquals(constants.char2ascii["'"], 39)
        self.assertEquals(constants.char2ascii['"'], 34)
        self.assertEquals(constants.char2ascii["\\"], 92)
        self.assertEquals(constants.char2ascii["a"], 97)

    def testStruct(self):
        from .constants import constants

        self.assertEquals(constants.str2struct["foo"].bar, {"baz": "qux"})


if __name__ == "__main__":
    unittest.main()
