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

from thrift.python.test.typeinfo_test import TypeInfoTests as CTests

# This python file serves as a boilerplate code for executing tests written
# in a Cython module. Simply import the Cython module containing the tests,
# and call the appropriate test functions within the TestCase class.


class TypeInfoTests(unittest.TestCase):
    def test_IntegerTypeInfo(self) -> None:
        CTests(self).test_IntegerTypeInfo()

    def test_StringTypeInfo(self) -> None:
        CTests(self).test_StringTypeInfo()

    def test_ListTypeInfo(self) -> None:
        CTests(self).test_ListTypeInfo()

    def test_ListTypeInfo_nested(self) -> None:
        CTests(self).test_ListTypeInfo_nested()

    def test_SetTypeInfo(self) -> None:
        CTests(self).test_SetTypeInfo()

    def test_SetTypeInfo_nested(self) -> None:
        CTests(self).test_SetTypeInfo_nested()
