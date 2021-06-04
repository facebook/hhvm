#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

import unittest

from hphp.hack.src.hh_codesynthesis.hackGenerator import (
    _HackInterfaceGenerator,
)


class _HackInterfaceGeneratorTest(unittest.TestCase):
    def setUp(self) -> None:
        self.obj = _HackInterfaceGenerator("I0")

    def test_single_interface(self) -> None:
        self.assertEqual("interface I0  {}", str(self.obj))

    def test_multiple_extends_interface(self) -> None:
        self.obj.add_extend("I1")
        self.assertEqual("interface I0 extends I1 {}", str(self.obj))
        self.obj.add_extend("I2")
        self.assertEqual("interface I0 extends I1,I2 {}", str(self.obj))
