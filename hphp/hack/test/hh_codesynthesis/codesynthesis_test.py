#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

import unittest

from hphp.hack.src.hh_codesynthesis import hh_codesynthesis


class ExtractLogicRulesTest(unittest.TestCase):
    def test_wrong_format(self) -> None:
        exp = [
            "extends_to(a, b).",
            "extends_to(i, b).",
            "extends_to(t, a).",
            "symbols(a;b;i;t).",
        ]
        deps = """\
Extends A -> Type B
Extends I -> Type B
Extends T -> Type A, Broke
Type A -> Type B
Type I -> Type B
Type T -> Type A, Type B"""
        self.assertListEqual(
            exp, hh_codesynthesis.extract_logic_rules(deps.split("\n"))
        )

    def test_multiple_lines(self) -> None:
        exp = [
            "extends_to(i1, c1).",
            "extends_to(i1, c2).",
            "extends_to(i1, c3).",
            "extends_to(i1, i2).",
            "extends_to(i3, c4).",
            "extends_to(i4, c5).",
            "symbols(c1;c2;c3;c4;c5;i1;i2;i3;i4).",
        ]
        deps = """\
Extends I1 -> Type C1, Type C2, Type C3, Type I2
Extends I3 -> Type C4,
                Type C6,
                Type I5,
                Type I6,
                Type I7,
                Type I8
Extends I4 -> Type C5,
               Type C6,
               Type I9,
               Type I10,
               Type I11,
               Type I12,
               Type I13,
               Type I14"""
        self.assertListEqual(
            exp, hh_codesynthesis.extract_logic_rules(deps.split("\n"))
        )

    def test_extends_dependency(self) -> None:
        exp = [
            "extends_to(a, b).",
            "extends_to(i, b).",
            "extends_to(t, a).",
            "symbols(a;b;i;t).",
        ]
        deps = """\
Extends A -> Type B
Extends I -> Type B
Extends T -> Type A
Type A -> Type B
Type I -> Type B
Type T -> Type A, Type B"""
        self.assertListEqual(
            exp, hh_codesynthesis.extract_logic_rules(deps.split("\n"))
        )
