import re

from . import base

class WalkStkCommandTestCase(base.TestHHVMBinary):

    def setUp(self):
        super().setUp(test_file="slow/reified-generics/reified-parent.php")

    def test_walkstk_command(self):
        # Just check we have a stack of the correct form; the specific
        # number of frames, line numbers, etc. may vary.
        self.run_until_breakpoint("checkClassReifiedGenericMismatch")
        _, output = self.run_commands(["walkstk"])
        frames = output.split("\n")
        prog = re.compile(r"#\d+\s+0x[a-f0-9]+\s@\s0x[a-f0-9]+:.*")
        for i, f in enumerate(filter(len, frames)):
            with self.subTest(name=str(i)):
                self.assertRegex(f, prog)
