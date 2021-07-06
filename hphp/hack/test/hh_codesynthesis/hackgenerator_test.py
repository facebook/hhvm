#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

import unittest

from hphp.hack.src.hh_codesynthesis.hackGenerator import (
    _HackInterfaceGenerator,
    _HackClassGenerator,
    HackCodeGenerator,
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

    def test_single_method_interface(self) -> None:
        self.obj.add_method("foo")
        self.assertEqual(
            "interface I0  {\npublic function foo(): void;\n}", str(self.obj)
        )

    def test_multiple_methods_interface(self) -> None:
        self.obj.add_method("foo")
        self.obj.add_method("bar")
        self.assertEqual(
            "interface I0  {\npublic function bar(): void;\n\npublic function foo(): void;\n}",
            str(self.obj),
        )

    def test_single_parameter_dummy_method_interface(self) -> None:
        self.obj.add_parameter("C0")
        self.assertEqual(
            "interface I0  {\npublic function dummy_I0_method(C0 $C0_obj): void;\n}",
            str(self.obj),
        )

    def test_multiple_parameters_dummy_method_interface(self) -> None:
        self.obj.add_parameter("C0")
        self.obj.add_parameter("C1")
        self.assertEqual(
            "interface I0  {\npublic function dummy_I0_method(C0 $C0_obj, C1 $C1_obj): void;\n}",
            str(self.obj),
        )


class _HackClassGeneratorTest(unittest.TestCase):
    def setUp(self) -> None:
        self.obj = _HackClassGenerator("C0")

    def test_single_class(self) -> None:
        self.assertEqual("class C0   {}", str(self.obj))

    def test_multiple_implements_interface(self) -> None:
        self.obj.add_implement("I1")
        self.assertEqual("class C0  implements I1 {}", str(self.obj))
        self.obj.add_implement("I2")
        self.assertEqual("class C0  implements I1,I2 {}", str(self.obj))

    def test_single_extend_class_multiple_implements_interface(self) -> None:
        self.obj.add_implement("I1")
        self.assertEqual("class C0  implements I1 {}", str(self.obj))
        self.obj.set_extend("C1")
        self.assertEqual("class C0 extends C1 implements I1 {}", str(self.obj))
        self.obj.add_implement("I2")
        self.assertEqual("class C0 extends C1 implements I1,I2 {}", str(self.obj))
        # invoke set_extend again, will overwrite the previous "C1"
        self.obj.set_extend("C2")
        self.assertEqual("class C0 extends C2 implements I1,I2 {}", str(self.obj))

    def test_single_method_class(self) -> None:
        self.obj.add_method("bar")
        self.assertEqual(
            "class C0   {\npublic function bar(): void{}\n}", str(self.obj)
        )

    def test_multiple_methods_class(self) -> None:
        self.obj.add_method("bar")
        self.obj.add_method("foo")
        self.assertEqual(
            "class C0   {\npublic function bar(): void{}\n\npublic function foo(): void{}\n}",
            str(self.obj),
        )

    def test_single_parameter_dummy_method_class(self) -> None:
        self.obj.add_parameter("C1")
        self.assertEqual(
            "class C0   {\npublic function dummy_C0_method(C1 $C1_obj): void{}\n}",
            str(self.obj),
        )

    def test_multiple_parameters_dummy_method_class(self) -> None:
        self.obj.add_parameter("C2")
        self.obj.add_parameter("C1")
        self.assertEqual(
            "class C0   {\npublic function dummy_C0_method(C1 $C1_obj, C2 $C2_obj): void{}\n}",
            str(self.obj),
        )

    def test_invoke_single_parameter_in_dummy_method_class(self) -> None:
        self.obj.add_parameter("C1")
        self.obj.add_invoke("C1", "foo")
        self.assertEqual(
            "class C0   {\npublic function dummy_C0_method(C1 $C1_obj): void{\n$C1_obj->foo();\n}\n}",
            str(self.obj),
        )

    def test_invoke_multiple_parameters_in_dummy_method_class(self) -> None:
        self.obj.add_parameter("C2")
        self.obj.add_parameter("C1")
        self.obj.add_invoke("C2", "foo")
        self.obj.add_invoke("C1", "foo")
        self.obj.add_invoke("C2", "bar")
        self.assertEqual(
            "class C0   {\npublic function dummy_C0_method(C1 $C1_obj, C2 $C2_obj): void{\n$C1_obj->foo();\n\n$C2_obj->bar();\n\n$C2_obj->foo();\n}\n}",
            str(self.obj),
        )


class HackCodeGeneratorTest(unittest.TestCase):
    def setUp(self) -> None:
        self.obj = HackCodeGenerator()

    def test_single_class(self) -> None:
        exp = """\
<?hh
class C0   {}

"""
        self.obj._add_class("C0")
        self.assertEqual(exp, str(self.obj))

    def test_single_interface(self) -> None:
        exp = """\
<?hh

interface I0  {}
"""
        self.obj._add_interface("I0")
        self.assertEqual(exp, str(self.obj))

    def test_mix_single_class_and_interface(self) -> None:
        exp = """\
<?hh
class C0   {}
interface I0  {}
"""
        self.obj._add_class("C0")
        self.obj._add_interface("I0")
        self.assertEqual(exp, str(self.obj))

    def test_class_extends_class_implements_interface(self) -> None:
        exp = """\
<?hh
class C0   {}
class C1 extends C0 implements I1,I2 {}
interface I0  {}
interface I1 extends I0 {}
interface I2  {}
"""
        self.obj._add_class("C0")
        self.obj._add_class("C1")
        self.obj._add_interface("I0")
        self.obj._add_interface("I1")
        self.obj._add_interface("I2")
        self.obj._add_extend("C1", "C0")
        self.obj._add_extend("I1", "I0")
        self.obj._add_implement("C1", "I1")
        self.obj._add_implement("C1", "I2")
        self.assertEqual(exp, str(self.obj))

    def test_class_implements_interface_with_method_override(self) -> None:
        exp = """\
<?hh
class C0  implements I0 {
public function foo(): void{}
}
interface I0  {
public function foo(): void;
}
"""
        self.obj._add_class("C0")
        self.obj._add_interface("I0")
        self.obj._add_implement("C0", "I0")
        self.obj._add_method("I0", "foo")
        self.obj._add_method("C0", "foo")
        self.assertEqual(exp, str(self.obj))

    def test_class_extends_class_with_method_override(self) -> None:
        exp = """\
<?hh
class C0 extends C1  {
public function foo(): void{}
}
class C1   {
public function foo(): void{}
}

"""
        self.obj._add_class("C0")
        self.obj._add_class("C1")
        self.obj._add_extend("C0", "C1")
        self.obj._add_method("C1", "foo")
        self.obj._add_method("C0", "foo")
        self.assertEqual(exp, str(self.obj))

    def test_interface_extends_interface_with_method_override(self) -> None:
        exp = """\
<?hh

interface I0 extends I1 {
public function bar(): void;
}
interface I1  {
public function bar(): void;
}
"""
        self.obj._add_interface("I0")
        self.obj._add_interface("I1")
        self.obj._add_extend("I0", "I1")
        self.obj._add_method("I1", "bar")
        self.obj._add_method("I0", "bar")
        self.assertEqual(exp, str(self.obj))

    def test_interface_passed_as_parameter_to_class(self) -> None:
        exp = """\
<?hh
class C0   {
public function dummy_C0_method(I0 $I0_obj): void{}
}
interface I0  {}
"""
        self.obj._add_interface("I0")
        self.obj._add_class("C0")
        self.obj._add_to_parameter_set("C0", "I0")
        self.assertEqual(exp, str(self.obj))

    def test_naming_conflict_with_dummy_method(self) -> None:
        exp = """\
<?hh
class C0   {
public function dummy_C0_method_(I0 $I0_obj): void{}

public function dummy_C0_method(): void{}
}
interface I0  {}
"""
        self.obj._add_interface("I0")
        self.obj._add_class("C0")
        self.obj._add_method("C0", "dummy_C0_method")
        self.obj._add_to_parameter_set("C0", "I0")

    def test_class_with_method_passed_as_parameter_to_another_class(self) -> None:
        exp = """\
<?hh
class C1   {
public function foo(): void{}
}
class C0   {
public function dummy_C0_method(C1 $C1_obj): void{
$C1_obj->foo();
}
}

"""
        self.obj._add_class("C1")
        self.obj._add_method("C1", "foo")
        self.obj._add_class("C0")
        self.obj._add_to_parameter_set("C0", "C1")
        self.obj._add_invoke("C0", "C1", "foo")
        self.assertEqual(exp, str(self.obj))
