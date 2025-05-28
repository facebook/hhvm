# pyre-strict
from . import base  # usort: skip (must be first, needed for sys.path side-effects)
import hphp.tools.lldb.sizeof as sizeof
import hphp.tools.lldb.utils as utils


class SizeofCommandHHVMTestCase(base.TestHHVMBinary):
    def file(self) -> str:
        return "quick/properties2.php"

    def interp(self) -> bool:
        return True

    def test_sizeof_helper_indexed_string_map(self) -> None:
        self.run_commands(["b newObjImpl", "continue", "thread step-out"])
        this_ = self.thread.return_value
        cls = utils.rawptr(utils.get(this_, "m_cls"))
        self.assertIsNotNone(cls)
        props_size = sizeof.sizeof(utils.get(cls, "m_declProperties"))
        self.assertIsNotNone(props_size)
        self.assertEqual(props_size, 1)

    def test_sizeof_command_indexed_string_map(self) -> None:
        self.run_commands(["b newObjImpl", "continue", "thread step-out", "next"])
        _, output = self.run_commands(["sizeof this_->m_cls->m_declProperties"])
        self.assertTrue(output, "1")

    def test_sizeof_command_bad_container(self) -> None:
        self.run_commands(["b newObjImpl", "continue", "thread step-out", "next"])
        status, output = self.run_commands(["sizeof this_->m_cls"], check=False)
        self.assertNotEqual(status, 0)
        self.assertEqual(output.strip(), "error: unrecognized container")


class SizeofCommandTypesTestCase(base.TestHHVMTypesBinary):
    def kindOfTest(self) -> str:
        return "sizeof-values"

    def test_sizeof(self) -> None:
        with self.subTest("Array (Dict)"):
            self.run_until_breakpoint("takeArray")
            _, output = self.run_commands(["sizeof v"])
            self.assertEqual(output.strip(), "3")

        with self.subTest("ArrayData<Dict>"):
            self.run_until_breakpoint("takeArrayData")
            _, output = self.run_commands(["sizeof v"])
            self.assertEqual(output.strip(), "3")

        with self.subTest("Array (Vec)"):
            self.run_until_breakpoint("takeArray")
            _, output = self.run_commands(["sizeof v"])
            self.assertEqual(output.strip(), "5")

        with self.subTest("ArrayData<Vec>"):
            self.run_until_breakpoint("takeArrayData")
            _, output = self.run_commands(["sizeof v"])
            self.assertEqual(output.strip(), "5")

        with self.subTest("Array (Keyset)"):
            self.run_until_breakpoint("takeArray")
            _, output = self.run_commands(["sizeof v"])
            self.assertEqual(output.strip(), "2")

        with self.subTest("ArrayData<Keyset>"):
            self.run_until_breakpoint("takeArrayData")
            _, output = self.run_commands(["sizeof v"])
            self.assertEqual(output.strip(), "2")
