# Copyright 2022-present Facebook. All Rights Reserved.

# pyre-unsafe

from . import base  # usort: skip (must be first, needed for sys.path side-effects)


class NameOfCommandTestCase(base.TestHHVMTypesBinary):
    def setUp(self):
        super().setUp(test_type="nameof-values")

    def test_nameof(self):
        with self.subTest("nameof Class"):
            self.run_until_breakpoint("takeClass")
            _, output = self.run_commands(["nameof v"])
            # Yes, the name of the class is actually InvalidArgumentException;
            # we're not testing for errors here.
            self.assertEqual(output, "InvalidArgumentException")
            _, output = self.run_commands(["nameof *v"])
            self.assertEqual(output, "InvalidArgumentException")

        with self.subTest("nameof LazyClassData"):
            self.run_until_breakpoint("takeLazyClassData")
            _, output = self.run_commands(["nameof v"])
            self.assertEqual(output, "SpecialLazyClass")

        with self.subTest("nameof Object"):
            self.run_until_breakpoint("takeObject")
            _, output = self.run_commands(["nameof v"])
            self.assertEqual(output, "InvalidArgumentException")

        with self.subTest("nameof ObjectData"):
            self.run_until_breakpoint("takeObjectData")
            _, output = self.run_commands(["nameof v"])
            self.assertEqual(output, "InvalidArgumentException")
            _, output = self.run_commands(["nameof *v"])
            self.assertEqual(output, "InvalidArgumentException")

        with self.subTest("nameof Func"):
            self.run_until_breakpoint("takeFunc")
            _, output = self.run_commands(["nameof v"])
            self.assertEqual(output, "Exception::__construct")
            _, output = self.run_commands(["nameof *v"])
            self.assertEqual(output, "Exception::__construct")
