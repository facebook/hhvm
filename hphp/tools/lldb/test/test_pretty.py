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
        # Thus, the order here matters (i.e. use the order in the DataType enum)
        breakpoints_to_outputs = {
            "String": r'\(HPHP::TypedValue\) \$\d+ = \{ String, "Hello, world!" \}',
            "Resource": r'\(HPHP::TypedValue\) \$\d+ = \{ Resource, \(HPHP::ResourceHdr \*\) pres = 0x.*\}',
            "Boolean": r'\(HPHP::TypedValue\) \$\d+ = \{ Boolean, True \}',
            "Int64": r'\(HPHP::TypedValue\) \$\d+ = \{ Int64, 42 \}',
            "Double": r'\(HPHP::TypedValue\) \$\d+ = \{ Double, 3.1415 \}',
            "Uninit": r'\(HPHP::TypedValue\) \$\d+ = \{ Uninit \}',
            "Null": r'\(HPHP::TypedValue\) \$\d+ = \{ Null \}',
        }

        for tv_name, expected_output in breakpoints_to_outputs.items():
            self.run_until_breakpoint(f"takeTypedValue{tv_name}")
            _, output = self.run_commands(["p tv"])
            self.assertRegex(output.strip(), re.compile(expected_output))
