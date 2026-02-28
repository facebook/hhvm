#!/usr/bin/env python3
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

import unittest
from collections import OrderedDict

from thrift.test.sample_structs.ttypes import DummyStruct, Struct
from thrift.util import struct_to_dict


class TestStructToDict(unittest.TestCase):
    def test_struct_to_dict(self) -> None:
        dummy_struct = DummyStruct(a=1)
        test_struct = Struct(
            a=1,
            b=[1, 2],
            c={1, 3},
            d={1: 2, 3: 4},
            e={1: [1, 2]},
            f={1: {1, 2}},
            g=[dummy_struct],
            h=[[1, 2]],
            i=[{1, 2}],
        )
        self.assertEqual(
            struct_to_dict(test_struct),
            OrderedDict(
                [
                    ("a", 1),
                    ("b", [1, 2]),
                    ("c", {1, 3}),
                    ("d", {1: 2, 3: 4}),
                    ("e", {1: [1, 2]}),
                    ("f", {1: {1, 2}}),
                    ("g", [OrderedDict([("a", 1)])]),
                    ("h", [[1, 2]]),
                    ("i", [{1, 2}]),
                ]
            ),
        )
