#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
import tempfile
import unittest

from hphp.hack.src.hh_codesynthesis import agentGenerator, hackGenerator
from hphp.hack.src.hh_codesynthesis.agentGenerator import ClingoContext


class GenerateLogicRulesTest(unittest.TestCase):
    def test_depth_less_than_nodes(self) -> None:
        solving_context = ClingoContext(number_of_nodes=12, min_depth=3)
        exp = [
            'internal_symbols("S0", 0;"S1", 1;"S2", 2;"S3", 3;"S4", 4;"S5", 5;"S6",'
            ' 6;"S7", 7;"S8", 8;"S9", 9;"S10", 10;"S11", 11).',
            'extends_to("S0", "S4").',
            'extends_to("S4", "S8").',
        ]
        self.assertListEqual(exp, agentGenerator.generate_logic_rules(solving_context))

    def test_depth_more_than_nodes(self) -> None:
        # In this case, the graph has no way to satisfy the min_depth requirement.
        # The user, or the higher level wrapper should make sure given proper
        # parameters. Otherwise, we will create the following output.
        solving_context = ClingoContext(number_of_nodes=3, min_depth=5)
        with self.assertRaises(
            expected_exception=RuntimeError, msg="Received unreasonable parameters."
        ):
            agentGenerator.generate_logic_rules(solving_context)

    def test_depth_equals_to_nodes(self) -> None:
        solving_context = ClingoContext(number_of_nodes=7, min_depth=7)
        exp = [
            'internal_symbols("S0", 0;"S1", 1;"S2", 2;"S3", 3;"S4", 4;"S5",'
            ' 5;"S6", 6).',
            'extends_to("S0", "S1").',
            'extends_to("S1", "S2").',
            'extends_to("S2", "S3").',
            'extends_to("S3", "S4").',
            'extends_to("S4", "S5").',
            'extends_to("S5", "S6").',
        ]
        self.assertListEqual(exp, agentGenerator.generate_logic_rules(solving_context))

    def test_degree_distribution(self) -> None:
        solving_context = ClingoContext(
            number_of_nodes=12, degree_distribution=[1, 3, 5]
        )
        exp = [
            'internal_symbols("S0", 0;"S1", 1;"S2", 2;"S3", 3;"S4", 4;"S5", 5;"S6",'
            ' 6;"S7", 7;"S8", 8;"S9", 9;"S10", 10;"S11", 11).',
            ":- #count{X : in_degree(X, 0)} < 1.",
            ":- #count{X : in_degree(X, 1)} < 3.",
            ":- #count{X : in_degree(X, 2)} < 5.",
        ]
        self.assertListEqual(exp, agentGenerator.generate_logic_rules(solving_context))

    def test_sum_of_degrees_greater_than_nodes(self) -> None:
        solving_context = ClingoContext(
            number_of_nodes=12, degree_distribution=[3, 5, 7]
        )
        with self.assertRaises(
            expected_exception=RuntimeError, msg="Received unreasonable parameters."
        ):
            agentGenerator.generate_logic_rules(solving_context)

    def test_hack_code_gen(self) -> None:
        solving_context = ClingoContext(
            number_of_nodes=12,
            min_depth=3,
            min_classes=3,
            min_interfaces=4,
            lower_bound=1,
            higher_bound=5,
            min_stub_classes=4,
            min_stub_interfaces=1,
            degree_distribution=[1, 3, 5],
        )

        hack_codegen = hackGenerator.HackCodeGenerator(solving_context)
        agentGenerator.do_reasoning(
            additional_programs=agentGenerator.generate_logic_rules(solving_context),
            generator=hack_codegen,
        )
        self.assertTrue(hack_codegen.validate())

    def test_unsatisfiable_parameters(self) -> None:
        # Given 5 nodes, but asking for 3 classes + 4 interfaces with
        solving_context = ClingoContext(
            number_of_nodes=5, min_classes=3, min_interfaces=4
        )
        hack_codegen = hackGenerator.HackCodeGenerator(solving_context)

        with self.assertRaises(expected_exception=RuntimeError, msg="Unsatisfiable."):
            agentGenerator.do_reasoning(
                additional_programs=agentGenerator.generate_logic_rules(
                    solving_context
                ),
                generator=hack_codegen,
            )


class ExtractLogicRulesTest(unittest.TestCase):
    def test_wrong_format(self) -> None:
        exp = [
            'extends_to("A", "B").',
            'extends_to("I", "B").',
            'extends_to("T", "A").',
            'method("A", "foo", "B").',
            'type("A", "B").',
            'type("I", "B").',
            'type("T", "A").',
            'type("T", "B").',
            'symbols("A";"B";"I";"T").',
        ]
        deps = """\
Extends A -> Type B
Extends I -> Type B
Extends T -> Type A, Broke
Method A::foo -> Type B
Type A -> Type B
Type I -> Type B
Type T -> Type A, Type B"""
        # pyre-fixme[6]: For 1st param expected `List[str]` but got
        #  `List[typing_extensions.LiteralString]`.
        self.assertListEqual(exp, agentGenerator.extract_logic_rules(deps.split("\n")))

    def test_unexpected_rhs(self) -> None:
        deps = """\
Type A -> SMethod B::foo
"""
        with self.assertRaises(
            expected_exception=NotImplementedError,
            msg="Not supported SMethod on the right hand side.",
        ):
            # pyre-fixme[6]: For 1st param expected `List[str]` but got
            #  `List[typing_extensions.LiteralString]`.
            agentGenerator.extract_logic_rules(deps.split("\n"))

    def test_multiple_lines(self) -> None:
        exp = [
            'extends_to("I1", "C1").',
            'extends_to("I1", "C2").',
            'extends_to("I1", "C3").',
            'extends_to("I1", "I2").',
            'extends_to("I3", "C4").',
            'extends_to("I4", "C5").',
            'symbols("C1";"C2";"C3";"C4";"C5";"I1";"I2";"I3";"I4").',
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
        # pyre-fixme[6]: For 1st param expected `List[str]` but got
        #  `List[typing_extensions.LiteralString]`.
        self.assertListEqual(exp, agentGenerator.extract_logic_rules(deps.split("\n")))

    def test_multiple_lines_all(self) -> None:
        # T92303034 Temporary handle for the multiple lines using replace(",\n", ","),
        exp = [
            'extends_to("I1", "C1").',
            'extends_to("I1", "C2").',
            'extends_to("I1", "C3").',
            'extends_to("I1", "I2").',
            'extends_to("I3", "C4").',
            'extends_to("I3", "C6").',
            'extends_to("I3", "I5").',
            'extends_to("I3", "I6").',
            'extends_to("I3", "I7").',
            'extends_to("I3", "I8").',
            'extends_to("I4", "C5").',
            'extends_to("I4", "C6").',
            'extends_to("I4", "I9").',
            'extends_to("I4", "I10").',
            'extends_to("I4", "I11").',
            'extends_to("I4", "I12").',
            'extends_to("I4", "I13").',
            'extends_to("I4", "I14").',
            'symbols("C1";"C2";"C3";"C4";"C5";"C6";"I1";"I10";"I11";"I12";"I13";"I14";"I2";"I3";"I4";"I5";"I6";"I7";"I8";"I9").',
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
            # pyre-fixme[6]: For 1st param expected `List[str]` but got
            #  `List[typing_extensions.LiteralString]`.
            agentGenerator.extract_logic_rules(deps.replace(",\n", ",").split("\n")),
        )

    def test_extends_type_method_dependency(self) -> None:
        exp = [
            'extends_to("A", "B").',
            'extends_to("I", "B").',
            'extends_to("T", "A").',
            'method("A", "foo", "B").',
            'type("A", "B").',
            'type("I", "B").',
            'type("T", "A").',
            'type("T", "B").',
            'symbols("A";"B";"I";"T").',
        ]
        deps = """\
Extends A -> Type B
Extends I -> Type B
Extends T -> Type A
Method A::foo -> Type B
Type A -> Type B
Type I -> Type B
Type T -> Type A, Type B"""
        # pyre-fixme[6]: For 1st param expected `List[str]` but got
        #  `List[typing_extensions.LiteralString]`.
        self.assertListEqual(exp, agentGenerator.extract_logic_rules(deps.split("\n")))

    def test_extends_type_smethod_dependency(self) -> None:
        exp = [
            'extends_to("I", "A").',
            'method("I", "bar", "A").',
            'static_method("A", "foo", "B").',
            'static_method("A", "foo", "T").',
            'type("A", "B").',
            'type("A", "T").',
            'type("I", "A").',
            'symbols("A";"I";"T").',
            'funcs("B").',
        ]
        deps = """\
Extends I -> Type A
Method I::bar -> Type A
SMethod A::foo -> Fun B, Type T
Type A -> Fun B, Type T
Type I -> Type A"""
        # pyre-fixme[6]: For 1st param expected `List[str]` but got
        #  `List[typing_extensions.LiteralString]`.
        self.assertListEqual(exp, agentGenerator.extract_logic_rules(deps.split("\n")))

    def test_extends_type_fun_dependency(self) -> None:
        exp = [
            'extends_to("I", "A").',
            'method("I", "bar", "A").',
            'invoked_by("F", "A").',
            'type("A", "B").',
            'type("A", "T").',
            'type("I", "A").',
            'symbols("A";"B";"I";"T").',
            'funcs("F").',
        ]
        deps = """\
Extends I -> Type A
Method I::bar -> Type A
Fun F -> Type A
Type A -> Type B, Type T
Type I -> Type A"""
        # pyre-fixme[6]: For 1st param expected `List[str]` but got
        #  `List[typing_extensions.LiteralString]`.
        self.assertListEqual(exp, agentGenerator.extract_logic_rules(deps.split("\n")))

    def test_unsupported_type_dependency(self) -> None:
        # T94428437 Temporary skipping all built-in functions for now.
        exp = ['extends_to("A", "B").', 'type("A", "B").', 'symbols("A";"B").']
        deps = r"""
Extends A -> Type B
Type A -> Type B
Type HH\Capabilities\AccessGlobals -> Type B
Type HH\Contexts\Unsafe\globals -> Type A"""
        # pyre-fixme[6]: For 1st param expected `List[str]` but got
        #  `List[typing_extensions.LiteralString]`.
        self.assertListEqual(exp, agentGenerator.extract_logic_rules(deps.split("\n")))


class DoReasoningTest(unittest.TestCase):
    def test_clingo_exception(self) -> None:
        deps = ["rule_without_period(symbol1, symbol2)"]
        raw_codegen = agentGenerator.CodeGenerator()
        with self.assertRaises(expected_exception=RuntimeError, msg="parsing failed"):
            agentGenerator.do_reasoning(additional_programs=deps, generator=raw_codegen)

    def test_extends_dependency(self) -> None:
        exp = [
            'class("B")',
            'class("I")',
            'extends("A","T")',
            'extends("B","I")',
            'implements("B","A")',
            'interface("A")',
            'interface("T")',
        ]
        rules = [
            'extends_to("A", "B").',
            'extends_to("I", "B").',
            'extends_to("T", "A").',
            'symbols("A";"B";"I";"T").',
        ]
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_type_dependency(self) -> None:
        # This one covered the 'has_method_with_parameter'.
        exp = ['class("B")', 'has_method_with_parameter("C","B")', 'interface("C")']
        rules = ['type("B", "C").', 'symbols("B"; "C").']
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_method_dependency(self) -> None:
        # This one covered the 'invokes_in_method', as well as the
        # 'has_method_with_parameter', since we need to pass the object as parameter,
        # then invoke its method.
        exp = [
            'add_method("B","foo")',
            'class("C")',
            'has_method_with_parameter("C","B")',
            'interface("B")',
            'invokes_in_method("C","B","foo")',
        ]
        rules = ['method("B", "foo", "C").', 'symbols("B"; "C").']
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_smethod_type_dependencies(self) -> None:
        # This one covered the 'invokes_static_method' with 'Type', and we don't need to
        # pass the object as parameter, so that we directly invoke the static method.
        exp = [
            'add_static_method("B","foo")',
            'class("B")',
            'class("C")',
            'interface("A")',
            'invokes_static_method("C","B","foo")',
        ]
        rules = ['static_method("B", "foo", "C").', 'symbols("A"; "B"; "C").']
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_smethod_fun_dependencies(self) -> None:
        # This one covered the 'invokes_static_method' with 'Fun', and we don't need to
        # pass the object as parameter, so that we directly invoke the static method.
        exp = [
            'add_static_method("B","foo")',
            'class("B")',
            'funcs("C")',
            'interface("A")',
            'invokes_static_method("C","B","foo")',
        ]
        rules = ['static_method("B", "foo", "C").', 'symbols("A"; "B").funcs("C").']
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_smethod_dependency_exception(self) -> None:
        # This one covered the unsatifiable part, that we can't find an answer.
        # Here we are forcing symbol("B") to get interface("B").
        rules = ['static_method("A", "foo", "B").', 'interface("B").symbols("A"; "B").']
        raw_codegen = agentGenerator.CodeGenerator()
        with self.assertRaises(expected_exception=RuntimeError, msg="Unsatisfiable."):
            agentGenerator.do_reasoning(
                additional_programs=rules, generator=raw_codegen
            )

    def test_method_type_extends_dependencies(self) -> None:
        # This one covered the 'override' in the "Extend" and "Method" edge.
        exp = [
            'add_method("B","foo")',
            'add_method("C","foo")',
            'class("C")',
            'implements("C","B")',
            'interface("B")',
        ]
        rules = [
            'extends_to("B", "C").',
            'method("B", "foo", "C").',
            'type("B", "C").',
            'symbols("B"; "C").',
        ]
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_fun_type_dependencies(self) -> None:
        # This one covered the 'invokes_function' with 'Type'.
        exp = [
            'class("A")',
            'funcs("Fn")',
            'interface("B")',
            'invokes_function("A","Fn")',
        ]
        rules = ['invoked_by("Fn", "A").', 'symbols("A"; "B").', 'funcs("Fn").']
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_fun_fun_dependencies(self) -> None:
        # This one covered the 'invokes_function' with 'Fun'.
        exp = [
            'class("A")',
            'funcs("FnA")',
            'funcs("FnB")',
            'interface("B")',
            'invokes_function("FnA","FnB")',
        ]
        rules = [
            'invoked_by("FnB", "FnA").',
            'symbols("A"; "B").',
            'funcs("FnA"; "FnB").',
        ]
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_fun_dependency_exception(self) -> None:
        # This one covered the unsatifiable part, that we can't find an answer.
        # Here we are forcing symbol("A") to get interface("A").
        rules = [
            'class("B").',
            'invoked_by("Fn", "A").',
            'interface("A").',
            'symbols("A"; "B").',
            'funcs("Fn").',
        ]
        raw_codegen = agentGenerator.CodeGenerator()
        with self.assertRaises(expected_exception=RuntimeError, msg="Unsatisfiable."):
            agentGenerator.do_reasoning(
                additional_programs=rules, generator=raw_codegen
            )

    def test_class_method_fun_dependency(self) -> None:
        # This one covered the 'creates_in_body' with <'Method', 'Fun'>.
        exp = [
            'add_method("B","foo")',
            'class("B")',
            'creates_in_body("Fn","B")',
            'funcs("Fn")',
            'implements("B","A")',
            'interface("A")',
            'invokes_in_body("Fn","B","foo")',
        ]
        rules = [
            'method("B", "foo", "Fn").'
            'symbols("A"; "B").'
            'funcs("Fn").'
            'extends_to("A","B").'
        ]
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_interface_method_fun_dependency(self) -> None:
        # This one covered the 'has_parameter_and_argument' with <'Method', 'Fun'>.
        exp = [
            'add_method("A","foo")',
            'class("B")',
            'funcs("Fn")',
            'has_parameter_and_argument("Fn","A","B")',
            'implements("B","A")',
            'interface("A")',
            'invokes_in_body("Fn","A","foo")',
        ]
        rules = [
            'method("A", "foo", "Fn").'
            'symbols("A"; "B").'
            'funcs("Fn").'
            'extends_to("A","B").'
        ]
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_class_type_fun_dependency(self) -> None:
        # This one covered the 'creates_in_body' with <'Type', 'Fun'>.
        exp = [
            'class("B")',
            'creates_in_body("Fn","B")',
            'funcs("Fn")',
            'implements("B","A")',
            'interface("A")',
        ]
        rules = [
            'type("B", "Fn").',
            'symbols("A"; "B").',
            'funcs("Fn").',
            'extends_to("A","B").',
        ]
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_interface_type_fun_dependency(self) -> None:
        # This one covered the 'has_parameter_and_argument' with <'Type', 'Fun'>.
        exp = [
            'class("B")',
            'funcs("Fn")',
            'has_parameter_and_argument("Fn","A","B")',
            'implements("B","A")',
            'interface("A")',
        ]
        rules = [
            'type("A", "Fn").',
            'symbols("A"; "B").',
            'funcs("Fn").',
            'extends_to("A","B").',
        ]
        raw_codegen = agentGenerator.CodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=raw_codegen)
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)

    def test_extends_dependency_with_rule_extraction(self) -> None:
        exp = [
            'add_method("A","foo")',
            'add_method("B","foo")',
            'class("B")',
            'class("I")',
            'extends("A","T")',
            'extends("B","I")',
            'implements("B","A")',
            'interface("A")',
            'interface("T")',
        ]
        deps = """\
Extends A -> Type B
Extends I -> Type B
Extends T -> Type A
Method A::foo -> Type B
Type A -> Type B
Type I -> Type B
Type T -> Type A, Type B
"""
        raw_codegen = agentGenerator.CodeGenerator()
        # pyre-fixme[6]: For 1st param expected `List[str]` but got
        #  `List[typing_extensions.LiteralString]`.
        additional_programs = agentGenerator.extract_logic_rules(deps.split("\n"))
        agentGenerator.do_reasoning(
            additional_programs=additional_programs, generator=raw_codegen
        )
        self.assertListEqual(sorted(str(raw_codegen).split()), exp)


class CodeEmittingTest(unittest.TestCase):
    def extract_run_and_compare(
        self, deps: str, exp: str, generator: agentGenerator.CodeGenerator
    ) -> None:
        additional_programs = agentGenerator.extract_logic_rules(deps.split("\n"))
        agentGenerator.do_reasoning(
            additional_programs=additional_programs, generator=generator
        )
        self.assertEqual(str(generator), exp)

    def test_extends_dependency_hack_codegen(self) -> None:
        exp = """\
<?hh
class B extends I implements A {}
class I   {}
interface A extends T {}
interface T  {}
"""
        rules = [
            'extends_to("A", "B").',
            'extends_to("I", "B").',
            'extends_to("T", "A").',
            'symbols("A";"B";"I";"T").',
        ]
        hack_codegen = hackGenerator.HackCodeGenerator()
        agentGenerator.do_reasoning(additional_programs=rules, generator=hack_codegen)
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
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_method_and_extends_dependency_with_rule_extraction_hack_codegen_override(
        self,
    ) -> None:
        exp = """\
<?hh
class B  implements A {
public function foo(): void{}
}
interface A  {
public function foo(): void;
}
"""
        deps = """\
Extends A -> Type B
Method A::foo -> Type B
Type A -> Type B
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_type_dependency_with_rule_extraction_hack_codegen(self) -> None:
        exp = """\
<?hh
class A   {}
interface B  {
public function dummy_B_method(A $A_obj): void;
}
"""
        deps = """\
Type A -> Type B
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_only_method_dependency_with_rule_extraction_hack_codegen(self) -> None:
        exp = """\
<?hh
class B   {
public function dummy_B_method(A $A_obj): void{
$A_obj->foo();
}
}
interface A  {
public function foo(): void;
}
"""
        deps = """\
Method A::foo -> Type B
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_smethod_dependency_with_rule_extraction_hack_codegen(self) -> None:
        exp = """\
<?hh
class A  implements I {
public static function foo(): void{}
}
class T   {
public function dummy_T_method(A $A_obj): void{
A::foo();
}
}
interface I  {}
function B(): void {
A::foo();
}
function C(): void {
A::foo();
}
"""
        deps = """\
Extends I -> Type A
SMethod A::foo -> Fun B, Fun C, Type T
Type A -> Fun B, Fun C, Type T
Type I -> Type A
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_function_with_rule_extraction_hack_codegen(self) -> None:
        exp = """\
<?hh
class A  implements I {
public static function foo(): void{}

public function dummy_A_method(): void{
F0();
}

public function bar(): void{}
}
class B   {
public function dummy_B_method(A $A_obj): void{
F1();
}
}
class T   {
public function dummy_T_method(A $A_obj): void{
A::foo();
}
}
interface I  {
public function bar(): void;
}
function F0(): void {
A::foo();
}
function F1(): void {
F0();
}
"""
        deps = """\
Extends I -> Type A
SMethod A::foo -> Fun F0, Type T
Method I::bar -> Type A
Fun F0 -> Fun F1, Type A
Fun F1 -> Type B
Type A -> Type B, Type T
Type I -> Type A
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_function_class_with_rule_extraction_hack_codegen(self) -> None:
        exp = """\
<?hh
class A extends I  {}
class I   {}
interface B  {
public function dummy_B_method(A $A_obj): void;
}
interface T  {
public function dummy_T_method(A $A_obj): void;
}
function F0(): void {
$I_obj = new I();
}
"""
        deps = """\
Extends I -> Type A
Type A -> Type B, Type T
Type I -> Type A, Fun F0
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_function_interface_with_rule_extraction_hack_codegen(self) -> None:
        exp = """\
<?hh
class A  implements I {
public static function foo(): void{}
}
class B   {
public function dummy_B_method(A $A_obj): void{
F1();
}
}
class T   {
public function dummy_T_method(A $A_obj): void{
A::foo();
}
}
interface I  {}
function F0(I $I_obj): void {
A::foo();
}
function F1(): void {
$A_obj = new A();
F0($A_obj);
}
"""
        deps = """\
Extends I -> Type A
SMethod A::foo -> Fun F0, Type T
Fun F0 -> Fun F1
Fun F1 -> Type B
Type A -> Type B, Type T
Type I -> Type A, Fun F0
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_function_interface_method_with_rule_extraction_hack_codegen(self) -> None:
        exp = """\
<?hh
class A  implements I {
public static function foo(): void{}

public function bar(): void{}
}
class B   {
public function dummy_B_method(): void{
$A_obj = new A();
F0($A_obj);
}
}
class T   {
public function dummy_T_method(A $A_obj): void{
A::foo();
}
}
interface I  {
public function bar(): void;
}
function F0(I $I_obj): void {
A::foo();

$I_obj->bar();
}
"""
        deps = """\
Extends I -> Type A
SMethod A::foo -> Fun F0, Type T
Method I::bar -> Type A, Fun F0
Fun F0 -> Type B
Type A -> Fun F0, Type T
Type I -> Fun F0, Type A, Fun F0
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())

    def test_circular_type_method_dependency_with_rule_extraction_hack_codegen(
        self,
    ) -> None:
        exp = """\
<?hh
class C  implements Compare {
public function dummy_C_method(C $C_obj): void{
$C_obj->content();
}

public function content(): void{}

public function eq(): void{}
}
interface Compare  {
public function dummy_Compare_method(C $C_obj): void;

public function eq(): void;
}
"""
        deps = """\
Extends Compare -> Type C
Method C::content -> Type C
Method Compare::eq -> Type C
Type C -> Type Compare
Type Compare -> Type C
"""
        self.extract_run_and_compare(deps, exp, hackGenerator.HackCodeGenerator())


class ReadFromFileTest(unittest.TestCase):
    def test_read(self) -> None:
        exp = [
            'extends_to("A", "B").',
            'extends_to("I", "B").',
            'extends_to("T", "A").',
            'method("A", "foo", "B").',
            'static_method("A", "bar", "B").',
            'static_method("A", "bar", "F").',
            'invoked_by("F", "A").',
            'type("A", "B").',
            'type("I", "B").',
            'type("T", "A").',
            'type("T", "B").',
            'symbols("A";"B";"I";"T").',
            'funcs("F").',
        ]
        deps = """\
Extends A -> Type B
Extends I -> Type B
Extends T -> Type A
Method A::foo -> Type B
SMethod A::bar -> Type B, Fun F
Fun F -> Type A
Type A -> Type B
Type I -> Type B
Type T -> Type A, Type B
"""
        with tempfile.NamedTemporaryFile(mode="w") as fp:
            fp.write(deps)
            fp.flush()
            self.assertListEqual(
                exp,
                agentGenerator.extract_logic_rules(
                    agentGenerator.read_from_file_or_stdin(fp.name)
                ),
            )

    def test_non_exist(self) -> None:
        test_file = "non_exist.in"
        with self.assertRaises(expected_exception=FileNotFoundError):
            agentGenerator.extract_logic_rules(
                agentGenerator.read_from_file_or_stdin(test_file)
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
            agentGenerator.output_to_file_or_stdout(generator, fp.name)
            lines = fp.readlines()
            self.assertEqual("".join(lines), exp)
