# Copyright 2022-present Facebook. All Rights Reserved.

from . import base
import re

class PrettyPrinterTestCase(base.TestHHVMBinary):

    def setUp(self):
        super().setUp(file = "slow/reified-generics/reified-parent.php")

    def test_pp_string_data(self):
        self.run_until_breakpoint("checkReifiedGenericMismatchHelper<false>")
        _, output = self.run_commands(["p *name"])
        self.assertEqual(output.strip(), "(const HPHP::StringData) $0 = C")

    def test_pp_array_data(self):
        self.run_until_breakpoint("checkReifiedGenericMismatchHelper<false>")
        _, output = self.run_commands(["p *reified_generics"])
        self.assertEqual(output.strip(), "(const HPHP::ArrayData) $0 = ArrayData[m_kind]: 2 element(s) refcount=3")

class PrettyPrintTypedValuesTestCase(base.TestHHVMTypesBinary):

    def test_pp_tvs(self):
        # Printing all of them in one execution for efficiency.
        # Thus, the order here matters (i.e. use the order in the DataType enum and
        # order of other calls in the test binary).

        breakpoints_to_outputs = {
            "PersistentString": r'\(HPHP::TypedValue\) \$\d+ = \{ PersistentString, "Hello, world!" \}',
            "String": r'\(HPHP::TypedValue\) \$\d+ = \{ String, "Hello, world!" \}',
            "Object": r'\(HPHP::TypedValue\) \$\d+ = \{ Object, \(HPHP::ObjectData \*\) pobj = 0x.* \("InvalidArgumentException"\) \}',
            "Resource": r'\(HPHP::TypedValue\) \$\d+ = \{ Resource, \(HPHP::ResourceHdr \*\) pres = 0x.*\}',
            "RFunc": r'\(HPHP::TypedValue\) \$\d+ = \{ RFunc, \(HPHP::RFuncData \*\) prfunc = 0x.* \("Exception::__construct"\) \}',
            "RClsMeth": r'\(HPHP::TypedValue\) \$\d+ = \{ RClsMeth, \(HPHP::RClsMethData \*\) prclsmeth = 0x.* \("InvalidArgumentException::Exception::__construct"\) \}',
            "ClsMeth": r'\(HPHP::TypedValue\) \$\d+ = \{ ClsMeth, \(HPHP::ClsMethData\) \*m_data = \(m_cls = \d+, m_func = \d+\) \("InvalidArgumentException::Exception::__construct"\) \}',
            "Boolean": r'\(HPHP::TypedValue\) \$\d+ = \{ Boolean, True \}',
            "Int64": r'\(HPHP::TypedValue\) \$\d+ = \{ Int64, 42 \}',
            "Double": r'\(HPHP::TypedValue\) \$\d+ = \{ Double, 3.1415 \}',
            "Func": r'\(HPHP::TypedValue\) \$\d+ = \{ Func, \(const HPHP::Func \*\) pfunc = 0x.* \("Exception::__construct"\) \}',
            "Class": r'\(HPHP::TypedValue\) \$\d+ = \{ Class, \(HPHP::Class \*\) pclass = 0x.* \("InvalidArgumentException"\) \}',
            "LazyClass": r'\(HPHP::TypedValue\) \$\d+ = \{ LazyClass, \(HPHP::LazyClassData\) plazyclass = \(className = 0x.*\) \("SpecialLazyClass"\) \}',
            "Uninit": r'\(HPHP::TypedValue\) \$\d+ = \{ Uninit \}',
            "Null": r'\(HPHP::TypedValue\) \$\d+ = \{ Null \}',
        }

        for tv_name, expected_output in breakpoints_to_outputs.items():
            with self.subTest(f"TypedValue {tv_name}"):
                self.run_until_breakpoint(f"takeTypedValue{tv_name}")
                _, output = self.run_commands(["p tv"])
                self.assertRegex(output.strip(), re.compile(expected_output))

        with self.subTest("TypedValues (reference)"):
            self.run_until_breakpoint("takeTypedValueRef")
            _, output = self.run_commands(["p tv"])
            expected_output = r"\(HPHP::TypedValue\) \$\d+ = \{ Int64, 42 \}"
            self.assertRegex(output.strip(), re.compile(expected_output))

        with self.subTest("TypedValues (subclasses)"):
            self.run_until_breakpoint("takeVariant")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::Variant\) \$\d+ = \{ Int64, 42 \}'
            self.assertRegex(output.strip(), re.compile(expected_output))

            self.run_until_breakpoint("takeVarNR")
            _, output = self.run_commands(["p v"])
            expected_output = r'\(HPHP::VarNR\) \$\d+ = \{ Double, 2.718 \}'
            self.assertRegex(output.strip(), re.compile(expected_output))
