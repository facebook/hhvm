from . import base

import hhvm_lldb.frame as frame
import hhvm_lldb.utils as utils

class FrameTestCase(base.TestHHVMBinary):
    def setUp(self):
        super().setUp(test_file="slow/reified-generics/reified-parent.php")

    def test_create_native_frame(self):
        self.run_until_breakpoint("checkClassReifiedGenericMismatch")
        ar = utils.reg("fp", self.frame).Cast(utils.Type("HPHP::ActRec", self.target).GetPointerType())
        rip = utils.reg("ip", self.frame)
        self.assertFalse(frame.is_jitted(rip))
        native_frame = frame.create_native(0, ar, rip, self.frame)
        self.assertEqual(native_frame.func.split("(")[0], "HPHP::checkClassReifiedGenericMismatch")
        self.assertEqual(native_frame.file.split("/")[-1], "reified-generics.cpp")
        self.assertIsNotNone(native_frame.line)

    def test_create_php_frame(self):
        self.run_until_breakpoint("checkClassReifiedGenericMismatch")
        frame1 = self.frame.parent
        ar = utils.reg("fp", frame1).Cast(utils.Type("HPHP::ActRec", self.target).GetPointerType())
        rip = utils.reg("ip", frame1)
        self.assertTrue(frame.is_jitted(rip))
        php_frame = frame.create_php(1, ar, rip, frame1)
        self.assertEqual(php_frame.func.split("(")[0], "[PHP] C::86reifiedinit")
        self.assertTrue(php_frame.file.split("/")[-1], "reified-parent.php")
