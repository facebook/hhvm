# pyre-strict
from . import base  # usort: skip (must be first, needed for sys.path side-effects)
import unittest

import hphp.tools.lldb.frame as frame
import hphp.tools.lldb.utils as utils


class FrameTestCase(base.TestHHVMBinary):
    def file(self) -> str:
        return "slow/reified-generics/reified-parent.php"

    @unittest.skip(
        "This test isn't behaving well with non-lowptr; enable once we're testing in lowptr again"
    )
    def test_create_native_frame(self) -> None:
        self.run_until_breakpoint("checkClassReifiedGenericMismatch")
        ar = utils.reg("fp", self.frame).Cast(
            utils.Type("HPHP::ActRec", self.target).GetPointerType()
        )
        rip = utils.reg("ip", self.frame)
        self.assertFalse(frame.is_jitted(rip))
        native_frame = frame.create_native(0, ar, rip, self.frame)
        self.assertEqual(
            native_frame.func.split("(")[0], "HPHP::checkClassReifiedGenericMismatch"
        )
        file = native_frame.file
        self.assertIsNotNone(file)
        self.assertEqual(file.split("/")[-1], "reified-generics.cpp")
        self.assertIsNotNone(native_frame.line)

    def test_create_php_frame(self) -> None:
        self.run_until_breakpoint("checkClassReifiedGenericMismatch")
        frame1 = self.frame.parent
        ar = utils.reg("fp", frame1).Cast(
            utils.Type("HPHP::ActRec", self.target).GetPointerType()
        )
        rip = utils.reg("ip", frame1)
        self.assertTrue(frame.is_jitted(rip))
        php_frame = frame.create_php(1, ar, rip, frame1.pc)
        self.assertEqual(php_frame.func.split("(")[0], "[PHP] C::86reifiedinit")
        file = php_frame.file
        self.assertIsNotNone(file)
        self.assertTrue(file.split("/")[-1], "reified-parent.php")
