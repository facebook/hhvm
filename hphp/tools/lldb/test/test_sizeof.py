from . import base

import hhvm_lldb.utils as utils
import hhvm_lldb.sizeof as sizeof

class IdxCommandTestCase(base.TestHHVMBinary):

    def setUp(self):
        super().setUp(file = "quick/properties2.php", interp=True)

    def test_sizeof_helper_indexed_string_map(self):
        self.run_commands(["b newObjImpl", "continue", "thread step-out"])
        this_ = self.thread.return_value
        cls = utils.rawptr(utils.get(this_, "m_cls"))
        props_size = sizeof.sizeof(utils.get(cls, "m_declProperties"))
        self.assertEqual(props_size.unsigned, 1)

    def test_sizeof_command_indexed_string_map(self):
        self.run_commands(["b newObjImpl", "continue", "thread step-out", "next"])
        _, output = self.run_commands(["sizeof this_->m_cls->m_declProperties"])
        self.assertTrue(output, "1")

    def test_sizeof_command_bad_container(self):
        self.run_commands(["b newObjImpl", "continue", "thread step-out", "next"])
        status, output = self.run_commands(["sizeof this_->m_cls"], check=False)
        self.assertNotEqual(status, 0)
        self.assertEqual(output.strip(), "error: unrecognized container")
