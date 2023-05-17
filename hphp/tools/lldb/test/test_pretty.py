# Copyright 2022-present Facebook. All Rights Reserved.

from . import base
import re

class PrettyPrinterTestCase(base.TestHHVMBinary):

    def setUp(self):
        super().setUp(test_file="slow/reified-generics/reified-parent.php")

    def test_pp_string_data(self):
        self.run_until_breakpoint("checkReifiedGenericMismatchHelper<false>")
        _, output = self.run_commands(["p *name"])
        self.assertEqual(output.strip(), "(const HPHP::StringData) C")

class PrettyPrintTypedValuesTestCase(base.TestHHVMTypesBinary):

    def setUp(self):
        super().setUp(test_type = "typed-values")

    def test_pp_tvs(self):
        # Printing all of them in one execution for efficiency.
        # Thus, the order here matters (i.e. use the order in the DataType enum and
        # order of other calls in the test binary).

        breakpoints_to_outputs = {
            "PersistentString": r'\(HPHP::TypedValue\) \{ PersistentString, "Hello, world!" \}',
            "String": r'\(HPHP::TypedValue\) \{ String, "Hello, world!" \}',
            "Object": r'\(HPHP::TypedValue\) \{ Object, \(HPHP::ObjectData \*\) pobj = 0x.* \("InvalidArgumentException"\) \}',
            "Resource": r'\(HPHP::TypedValue\) \{ Resource, \(hdr = 0x.*, data = 0x.*\) *\}',
            "RFunc": r'\(HPHP::TypedValue\) \{ RFunc, \(HPHP::RFuncData \*\) prfunc = 0x.* \("Exception::__construct"\) \}',
            "RClsMeth": r'\(HPHP::TypedValue\) \{ RClsMeth, \(HPHP::RClsMethData \*\) prclsmeth = 0x.* \("InvalidArgumentException::Exception::__construct"\) \}',
            "ClsMeth": r'\(HPHP::TypedValue\) \{ ClsMeth, \(HPHP::ClsMethData\) \*m_data = \(m_cls = \d+, m_func = \d+\) \("InvalidArgumentException::Exception::__construct"\) \}',
            "Boolean": r'\(HPHP::TypedValue\) \{ Boolean, True \}',
            "Int64": r'\(HPHP::TypedValue\) \{ Int64, 42 \}',
            "Double": r'\(HPHP::TypedValue\) \{ Double, 3.1415 \}',
            "Func": r'\(HPHP::TypedValue\) \{ Func, \(const HPHP::Func \*\) pfunc = 0x.* \("Exception::__construct"\) \}',
            "Class": r'\(HPHP::TypedValue\) \{ Class, \(HPHP::Class \*\) pclass = 0x.* \("InvalidArgumentException"\) \}',
            "LazyClass": r'\(HPHP::TypedValue\) \{ LazyClass, \(HPHP::LazyClassData\) plazyclass = (?s).* \("SpecialLazyClass"\) \}',
            "Uninit": r'\(HPHP::TypedValue\) \{ Uninit \}',
            "Null": r'\(HPHP::TypedValue\) \{ Null \}',
        }

        for tv_name, expected_output in breakpoints_to_outputs.items():
            with self.subTest(f"TypedValue {tv_name}"):
                self.run_until_breakpoint(f"takeTypedValue{tv_name}")
                _, output = self.run_commands(["p tv"])
                self.assertRegex(output.strip(), expected_output)

        with self.subTest("TypedValues (reference)"):
            self.run_until_breakpoint("takeTypedValueRef")
            _, output = self.run_commands(["p tv"])
            expected_output = r"\(HPHP::TypedValue &\) 0x.* \{ Int64, 42 \}.*"
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("TypedValues (subclasses -- Variant)"):
            self.run_until_breakpoint("takeVariant")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::Variant\) \{ Int64, 42 \}'
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("TypedValues (subclasses -- VarNR)"):
            self.run_until_breakpoint("takeVarNR")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::VarNR\) \{ Double, 2.718 \}'
            self.assertRegex(output.strip(), expected_output)


class PrettyPrintOtherValuesTestCase(base.TestHHVMTypesBinary):

    def setUp(self):
        super().setUp(test_type = "other-values")

    def test_pp_other_values(self):
        with self.subTest("StringData"):
            self.run_until_breakpoint("takeStringData")
            _, output = self.run_commands(["p *v"])
            expected_output = r"\(HPHP::StringData\) hello"
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("String"):
            self.run_until_breakpoint("takeString")
            _, output = self.run_commands(["p v"])
            expected_output = r"\(HPHP::String\) hello"
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("StaticString"):
            self.run_until_breakpoint("takeStaticString")
            _, output = self.run_commands(["p v"])
            expected_output = r"\(HPHP::StaticString\) hello"
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("StrNR"):
            self.run_until_breakpoint("takeStrNR")
            _, output = self.run_commands(["p v"])
            expected_output = r"\(HPHP::StrNR\) hello"
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("Resource"):
            self.run_until_breakpoint("takeResource")
            _, output = self.run_commands(["p v"])
            expected_output = r"\(HPHP::Resource\) \(hdr = 0x.*, data = 0x.*\)"
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("Object"):
            self.run_until_breakpoint("takeObject")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::Object\) InvalidArgumentException'
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("req::ptr"):
            self.run_until_breakpoint("takeReqPtr")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::req::ptr<HPHP::ObjectData>\) InvalidArgumentException'
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("HPHP::Optional (Some)"):
            self.run_until_breakpoint("takeOptional")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::Optional<HPHP::String>\) \(HPHP::String\) Value = hello'
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("HPHP::Optional (None)"):
            self.run_until_breakpoint("takeOptional")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::Optional<HPHP::String>\) None'
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("HPHP::LowPtr"):
            self.run_until_breakpoint("takeLowPtr")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::LowPtr<HPHP::Class>\) InvalidArgumentException'
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("HPHP::LowStrPtr"):
            # This is used e.g. as an alias in types.h, so let's make sure
            # we can handle aliases too.
            self.run_until_breakpoint("takeLowStrPtr")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::LowStrPtr\) hello'
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("HPHP::Extension"):
            self.run_until_breakpoint("takeExtension")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::Extension\) test-extension \(version: 0.5, oncall: test-oncall\)'
            self.assertRegex(output.strip(), expected_output)

        with self.subTest("HPHP::Array (Vec)"):
            self.run_until_breakpoint("takeArrayVec")
            _, output = self.run_commands(["p v"])
            expected_line1 = r'\(HPHP::Array\) \(0x[0-9a-f]+\) ArrayData\[Vec\]: 4 element\(s\) refcount=2'
            expected_line2 = "{ (Showing elements not yet implemented) }"
            actual_line1, actual_line2, _ = output.split("\n")
            self.assertRegex(actual_line1.strip(), expected_line1)
            self.assertEqual(actual_line2.strip(), expected_line2)

        with self.subTest("HPHP::Array (Dict)"):
            self.run_until_breakpoint("takeArrayDict")
            _, output = self.run_commands(["p v"])
            expected_line1 = r'\(HPHP::Array\) \(0x[0-9a-f]+\) ArrayData\[Dict\]: 3 element\(s\) refcount=2'
            expected_line2 = "{ (Showing elements not yet implemented) }"
            actual_line1, actual_line2, _ = output.split("\n")
            self.assertRegex(actual_line1.strip(), expected_line1)
            self.assertEqual(actual_line2.strip(), expected_line2)

        with self.subTest("HPHP::Array (Keyset)"):
            self.run_until_breakpoint("takeArrayDict")
            _, output = self.run_commands(["p v"])
            expected_line1 = r'\(HPHP::Array\) \(0x[0-9a-f]+\) ArrayData\[Dict\]: 3 element\(s\) refcount=2'
            expected_line2 = "{ (Showing elements not yet implemented) }"
            actual_line1, actual_line2, _ = output.split("\n")
            self.assertRegex(actual_line1.strip(), expected_line1)
            self.assertEqual(actual_line2.strip(), expected_line2)
