# Copyright 2022-present Facebook. All Rights Reserved.

from . import base

class PrettyPrinterTestCase(base.LLDBTestBase):

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
