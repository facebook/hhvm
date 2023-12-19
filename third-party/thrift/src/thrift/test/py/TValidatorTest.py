#!/usr/bin/env python
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

import unittest

from myBinaryStruct.ttypes import *
from myBoolStruct.ttypes import *
from myByteStruct.ttypes import *
from myComplexStruct.ttypes import *
from myDoubleStruct.ttypes import *
from myI16Struct.ttypes import *
from myI32Struct.ttypes import *
from myMixedStruct.ttypes import *
from mySetStruct.ttypes import *
from myMapStruct.ttypes import *
from myNestedMapStruct.ttypes import *
from mySimpleStruct.ttypes import *

# import logging
# log = logging.getLogger()
# log.setLevel(logging.DEBUG)
# ch = logging.StreamHandler()
# ch.setLevel(logging.DEBUG)
# log.addHandler(ch)

from thrift.util.TValidator import TValidator
from ThriftTest.ttypes import WithAnnotations


class ValidationTest(unittest.TestCase):
    def setUp(self):
        self.v = TValidator()

    def valid(self, msg):
        self.assertTrue(self.v.validate(msg))

    def wrong(self, msg):
        self.assertFalse(self.v.validate(msg))

    def testBinary(self):
        self.valid(myBinaryStruct(a="xyzzy"))
        self.wrong(myBinaryStruct(a=3))

    def testBool(self):
        self.valid(myBoolStruct(a=True))
        self.valid(myBoolStruct(a=False))
        self.wrong(myBoolStruct(a=1))
        self.wrong(myBoolStruct(a="a"))

    def testByte(self):
        self.valid(myByteStruct(a=0))
        self.valid(myByteStruct(a=127))
        self.valid(myByteStruct(a=-128))
        self.wrong(myByteStruct(a=1.1))
        self.wrong(myByteStruct(a=128))
        self.wrong(myByteStruct(a=-129))

    def testI16(self):
        self.valid(myI16Struct(a=4567))
        self.wrong(myI16Struct(a=0xFEDCBA987))

    def testI32(self):
        self.valid(myI32Struct(a=12131415))
        self.wrong(myI32Struct(a=0xFFFFFFFFEDCBA))

    def testDouble(self):
        self.valid(myDoubleStruct(a=-2.192))
        self.valid(myDoubleStruct(a=float("inf")))
        self.valid(myDoubleStruct(a=float("-inf")))
        self.wrong(myDoubleStruct(a=2))

    def testMixed(self):
        self.valid(
            myMixedStruct(
                a=[],
                b=[mySuperSimpleStruct(a=5)],
                c={"flame": -8, "fire": -191},
                d={},
                e=set([1, 2, 3, 4]),
            )
        )

    def testStruct(self):
        self.valid(mySetStruct(a=set([4, 8, 15, 16])))
        self.valid(mySetStruct(a=set([])))
        self.wrong(mySetStruct(a=set([1, 0xFFFFFFFFFF, 2])))

    def testMap(self):
        self.valid(
            myMapStruct(
                stringMap={"a": "A", "b": "B"},
                boolMap={True: "True", False: "False"},
                byteMap={1: "one", 2: "two"},
                doubleMap={float("0.1"): "0.one", float("0.2"): "0.two"},
                enumMap={1: "male", 2: "female"},
            )
        )
        self.valid(
            mySimpleStruct(a=False, b=87, c=7880, d=-7880, e=-1, f=-0.1, g="T-bone")
        )
        self.wrong(mySimpleStruct(a=1))
        self.valid(
            myComplexStruct(
                mySimpleStruct(
                    a=True,
                    b=92,
                    c=902,
                    d=65536,
                    e=123456789,
                    f=3.1415,
                    g="Whan that Aprille",
                ),
                b=[314, 15, 9, 26535],
                c={
                    "qwerty": mySimpleStruct(c=1),
                    "slippy": mySimpleStruct(a=False, b=-4, c=5),
                },
                e=EnumTest.EnumTwo,
                x=ExceptionTest("test"),
            )
        )

    def testCustomValidator(self):
        def a_must_be_true(v):
            return v.a

        self.v.addClassValidator("mySimpleStruct", a_must_be_true)
        self.valid(
            myComplexStruct(
                mySimpleStruct(a=True),
            )
        )
        self.wrong(
            myComplexStruct(
                mySimpleStruct(a=False),
            )
        )

    def testNestedMap(self):
        self.valid(
            myNestedMapStruct(
                maps={
                    "1": {"1": mySimpleStruct(c=1)},
                    "2": {"2": mySimpleStruct(a=False, c=2)},
                }
            )
        )
        self.wrong(
            myNestedMapStruct(
                maps={
                    "1": {"1": mySimpleStruct(c=1)},
                    "2": {"2": mySimpleStruct(a=0, c=2)},
                }
            )
        )

    def testEnumSpec(self):
        self.assertTrue(hasattr(myComplexStruct.thrift_spec[4][3], "_NAMES_TO_VALUES"))

    def testAnnotations(self):
        self.assertTrue(hasattr(WithAnnotations, "thrift_field_annotations"))
        self.assertTrue(hasattr(WithAnnotations, "thrift_struct_annotations"))
        self.assertEqual(
            WithAnnotations.thrift_field_annotations,
            {
                1: {
                    "test.annotation": "none",
                    "test.hidden": "1",
                },
                3: {
                    "test.hidden": "3",
                },
            },
        )
        self.assertEqual(
            WithAnnotations.thrift_struct_annotations,
            {
                "test.struct_annotation": "ok",
                "test.partial": "1",
                "test.complex": """
   public:

    bool empty() const {
      return !(__isset.m1 ||
               __isset.m2 ||
               __isset.s1);
    }
  """,
            },
        )


if __name__ == "__main__":
    unittest.main()
