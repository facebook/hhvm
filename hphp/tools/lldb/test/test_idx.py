# Copyright 2022-present Facebook. All Rights Reserved.

from . import base

import hhvm_lldb.idx as idx

class IdxCommandTestCase(base.LLDBTestBase):

    def setUp(self):
        super().setUp(file = "slow/reified-generics/reified-parent.php")

    def test_idx_command_on_fixed_vector(self):
        self.run_until_breakpoint("checkClassReifiedGenericMismatch")
        output = self.run_command("idx c->m_slotIndex 0")
        self.assertTrue(output, "(unsigned short) *&(tmp) = 0")

    def test_idx_helper_on_fixed_vector(self):
        self.run_until_breakpoint("checkClassReifiedGenericMismatch")
        frame = self.getProcess().GetSelectedThread().GetFrameAtIndex(0)
        slot_index = frame.FindVariable("c").GetChildMemberWithName("m_slotIndex")
        val = idx.idx(slot_index, 0)
        self.assertEqual(val.unsigned, 0)
