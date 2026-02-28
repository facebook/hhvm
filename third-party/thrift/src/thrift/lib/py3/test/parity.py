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

import testing.constants_FBTHRIFT_ONLY_DO_NOT_USE as testing_constants
import testing.types as testing_types
import transitive_deps.a.constants_FBTHRIFT_ONLY_DO_NOT_USE as transitive_deps_constants
import transitive_deps.a.types as transitive_deps_types
from later.unittest import TestCase
from parameterized import parameterized_class

"""
This file tests parity between thrift-py3 cython constants, which are
converted from thrift-cpp2 constants and python constants, where all values are
generated directly in python.

This file will also serve to test parity between pure python thrift-py3 structs
and existing cython structs, insofar as those structs are used in constants.

This test is a prototype for generated tests to verify parity across thrift files.
"""


@parameterized_class(
    ("cython_const", "python_const"),
    [
        (testing_constants.LocationMap, testing_types.LocationMap),
        (testing_constants.int_list, testing_types.int_list),
        (testing_constants.RedColour, testing_types.RedColour),
        (testing_constants.BlueColour, testing_types.BlueColour),
        (testing_constants.FANCY_CONST, testing_types.FANCY_CONST),
        (transitive_deps_constants.kTransitiveD, transitive_deps_types.kTransitiveD),
    ],
)
class ConstantTests(TestCase):
    def test_cython_python_parity(self) -> None:
        # pyre-ignore[16]: disable static typing for parameterized test
        self.assertEqual(self.cython_const, self.python_const)
