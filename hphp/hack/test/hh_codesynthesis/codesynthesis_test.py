#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

import os
import tempfile
import unittest

from hphp.hack.src.hh_codesynthesis import hh_codesynthesis, hackGenerator


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

    def test_multiple_lines_all(self) -> None:
        # T92303034 Temporary handle for the multiple lines using replace(",\n", ","),
        exp = [
            "extends_to(i1, c1).",
            "extends_to(i1, c2).",
            "extends_to(i1, c3).",
            "extends_to(i1, i2).",
            "extends_to(i3, c4).",
            "extends_to(i3, c6).",
            "extends_to(i3, i5).",
            "extends_to(i3, i6).",
            "extends_to(i3, i7).",
            "extends_to(i3, i8).",
            "extends_to(i4, c5).",
            "extends_to(i4, c6).",
            "extends_to(i4, i9).",
            "extends_to(i4, i10).",
            "extends_to(i4, i11).",
            "extends_to(i4, i12).",
            "extends_to(i4, i13).",
            "extends_to(i4, i14).",
            "symbols(c1;c2;c3;c4;c5;c6;i1;i10;i11;i12;i13;i14;i2;i3;i4;i5;i6;i7;i8;i9).",
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
            exp,
            hh_codesynthesis.extract_logic_rules(deps.replace(",\n", ",").split("\n")),
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


class DoReasoningTest(unittest.TestCase):
    def test_clingo_exception(self) -> None:
        deps = ["rule_without_period(symbol1, symbol2)"]
        raw_codegen = hh_codesynthesis.CodeGenerator()
        with self.assertRaises(expected_exception=RuntimeError, msg="parsing failed"):
            hh_codesynthesis.do_reasoning(
                additional_programs=deps, generator=raw_codegen
            )

    def test_extends_dependency(self) -> None:
        exp = [
            "class(b)",
            "class(i)",
            "extends(a,t)",
            "extends(b,i)",
            "extends_to(a,b)",
            "extends_to(i,b)",
            "extends_to(t,a)",
            "implements(b,a)",
            "indirect_extends_to(a,b)",
            "indirect_extends_to(i,b)",
            "indirect_extends_to(t,a)",
            "indirect_extends_to(t,b)",
            "interface(a)",
            "interface(t)",
            "symbols(a)",
            "symbols(b)",
            "symbols(i)",
            "symbols(t)",
        ]
        rules = [
            "extends_to(a, b).",
            "extends_to(i, b).",
            "extends_to(t, a).",
            "symbols(a;b;i;t).",
        ]
        raw_codegen = hh_codesynthesis.CodeGenerator()
        hh_codesynthesis.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_extends_dependency_with_rule_extraction(self) -> None:
        exp = [
            "class(b)",
            "class(i)",
            "extends(a,t)",
            "extends(b,i)",
            "extends_to(a,b)",
            "extends_to(i,b)",
            "extends_to(t,a)",
            "implements(b,a)",
            "indirect_extends_to(a,b)",
            "indirect_extends_to(i,b)",
            "indirect_extends_to(t,a)",
            "indirect_extends_to(t,b)",
            "interface(a)",
            "interface(t)",
            "symbols(a)",
            "symbols(b)",
            "symbols(i)",
            "symbols(t)",
        ]
        deps = """\
Extends A -> Type B
Extends I -> Type B
Extends T -> Type A
Type A -> Type B
Type I -> Type B
Type T -> Type A, Type B
"""
        raw_codegen = hh_codesynthesis.CodeGenerator()
        additional_programs = hh_codesynthesis.extract_logic_rules(deps.split("\n"))
        hh_codesynthesis.do_reasoning(
            additional_programs=additional_programs, generator=raw_codegen
        )
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_extends_dependency_hack_codegen(self) -> None:
        exp = """\
<?hh
class B extends I implements A {}
class I   {}
interface A extends T {}
interface T  {}
"""
        rules = [
            "extends_to(a, b).",
            "extends_to(i, b).",
            "extends_to(t, a).",
            "symbols(a;b;i;t).",
        ]
        hack_codegen = hackGenerator.HackCodeGenerator()
        hh_codesynthesis.do_reasoning(additional_programs=rules, generator=hack_codegen)
        self.assertEqual(str(hack_codegen), exp)

    def test_extends_dependency_with_rule_extraction_hack_codegen(self) -> None:
        exp = """\
<?hh
class B extends I implements A {}
class I   {}
interface A extends T {}
interface T  {}
"""
        deps = """\
Extends A -> Type B
Extends I -> Type B
Extends T -> Type A
Type A -> Type B
Type I -> Type B
Type T -> Type A, Type B
"""
        hack_codegen = hackGenerator.HackCodeGenerator()
        additional_programs = hh_codesynthesis.extract_logic_rules(deps.split("\n"))
        hh_codesynthesis.do_reasoning(
            additional_programs=additional_programs, generator=hack_codegen
        )
        self.assertEqual(str(hack_codegen), exp)


class ReadFromFileTest(unittest.TestCase):
    def test_read(self) -> None:
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
Type T -> Type A, Type B
"""
        with tempfile.NamedTemporaryFile(mode="w") as fp:
            fp.write(deps)
            fp.flush()
            self.assertListEqual(
                exp,
                hh_codesynthesis.extract_logic_rules(
                    hh_codesynthesis.read_from_file_or_stdin(fp.name)
                ),
            )

    def test_non_exist(self) -> None:
        test_file = "non_exist.in"
        with self.assertRaises(expected_exception=FileNotFoundError):
            hh_codesynthesis.extract_logic_rules(
                hh_codesynthesis.read_from_file_or_stdin(test_file)
            )


class WriteToFileTest(unittest.TestCase):
    def test_hack_output(self) -> None:
        exp = """\
<?hh
class C1   {}
class C2 extends C1 implements I1 {}
interface I1  {}
"""
        generator = hackGenerator.HackCodeGenerator()
        generator._add_class("C1")
        generator._add_class("C2")
        generator._add_interface("I1")
        generator._add_extend("C2", "C1")
        generator._add_implement("C2", "I1")
        with tempfile.NamedTemporaryFile("r") as fp:
            hh_codesynthesis.output_to_file_or_stdout(generator, fp.name)
            lines = fp.readlines()
            self.assertEqual("".join(lines), exp)
