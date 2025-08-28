# Copyright 2025-present Facebook. All Rights Reserved.

# pyre-strict
from . import base  # usort: skip (must be first, needed for sys.path side-effects)

from typing import override


class AliasesTestCase(base.TestHHVMTypesBinary):
    def kindOfTest(self) -> str:
        return "aliases"

    def test_aliases_in_process(self) -> None:
        with self.subTest("return/inspect"):
            self.run_until_breakpoint("takeInt")
            self.run_commands(["return 84"])
            self.run_until_breakpoint("takeAnotherInt")
            # TODO(michristensen) Not sure why this doesn't work, because
            # using "inspect" in manual runs does work.
            # _, output = self.run_commands(["inspect v"])
            # self.assertEqual(output, "(uint32_t) 84")

        with self.subTest("disable/enable"):
            self.run_commands(["disable 1"])
            self.assertFalse(self.target.breakpoints[0].IsEnabled())

            self.run_commands(["enable 1"])
            self.assertTrue(self.target.breakpoints[0].IsEnabled())

        with self.subTest("info"):
            _, output = self.run_commands(["info break"])
            actual_lines = [line.strip() for line in output.split("\n") if line]
            self.assertEqual(actual_lines[0], "Current breakpoints:")

            _, output = self.run_commands(["info threads"])
            actual_lines = [line.strip() for line in output.split("\n") if line]
            self.assertTrue(len(actual_lines) >= 2)
            self.assertRegex(actual_lines[0], r"Process \d+ stopped")
            self.assertRegex(actual_lines[1], r"\* thread #1: tid.*")

            _, output = self.run_commands(["info registers"])
            actual_lines = [line.strip() for line in output.split("\n") if line]
            self.assertEqual(actual_lines[0], "General Purpose Registers:")

            _, output = self.run_commands(["info shared"])
            actual_lines = [line.strip() for line in output.split("\n") if line]
            self.assertRegex(actual_lines[0], r"\[\s*\d+\].*")

        with self.subTest("delete"):
            self.run_commands(["delete 1 2"])
            self.assertEmpty(self.target.breakpoints)

        with self.subTest("bt"):
            expected = r"\* thread #1, name = .*"
            _, output = self.run_commands(["backtrace"])
            actual_lines = [line.strip() for line in output.split("\n") if line]
            self.assertRegex(actual_lines[0], expected)

            _, output = self.run_commands(["where"])
            actual_lines = [line.strip() for line in output.split("\n") if line]
            self.assertRegex(actual_lines[0], expected)


class AliasesNoLaunchTestCase(base.TestHHVMTypesBinary):
    def kindOfTest(self) -> str:
        return "aliases"

    @override
    def launchProcess(self) -> bool:
        return False

    def test_regex_aliases_produce_same_output(self) -> None:
        _, output_a = self.run_commands(["show environment"])
        _, output_b = self.run_commands(["settings show target.env-vars"])
        self.assertEqual(output_a.strip(), output_b.strip())
        _, output_b = self.run_commands(["show env"])
        self.assertEqual(output_a.strip(), output_b.strip())

        _, output_a = self.run_commands(["show args"])
        _, output_b = self.run_commands(["settings show target.run-args"])
        self.assertEqual(output_a.strip(), output_b.strip())

        self.run_commands(["settings set target.env-vars FOO=BAR"])
        self.assertEqual(self.target.GetEnvironment().Get("FOO"), "BAR")
        self.run_commands(["unset env FOO"])
        self.assertNone(self.target.GetEnvironment().Get("FOO"))
