#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

import unittest
from typing import Iterator

from hphp.hack.src.hh_asdiff import diff


def iter_lines(s: str) -> Iterator[str]:
    return iter(s.splitlines(keepends=True))


class EqualTest(unittest.TestCase):
    def test_equal_lines_same_len(self) -> None:
        exp = """\
#starts here
.main {
  Int 1
  RetC
}
#ends here
        """
        act = """\
#starts here
.main {
  Int 2
  RetC
}
#ends here
        """
        self.assertFalse(diff.equal_lines(iter_lines(exp), iter_lines(act)))
        self.assertTrue(
            diff.equal_lines(iter_lines(exp), iter_lines(act.replace("2", "1")))
        )

    def test_infinite_actual_lines_due_to_infinite_loop(self) -> None:
        COMMON_LINE = "foo\n"

        # pyre-fixme[53]: Captured variable `COMMON_LINE` is not annotated.
        def gen_foo_once() -> Iterator[str]:
            yield COMMON_LINE

        # pyre-fixme[53]: Captured variable `COMMON_LINE` is not annotated.
        def gen_foo_indefinitely() -> Iterator[str]:
            while True:
                yield COMMON_LINE

        self.assertFalse(diff.equal_lines(gen_foo_once(), gen_foo_indefinitely()))

    def test_infinite_actual_lines_due_to_infinite_loop(self) -> None:
        exp_seq = ("a\n", "b\n")
        act_seq = ["a\n", "b\n"]
        self.assertNotEqual(exp_seq, act_seq)

        # unlike __eq__ (operator ==), equal_lines returns True
        self.assertTrue(diff.equal_lines(iter(exp_seq), iter(act_seq)))


class DiffRankerTest(unittest.TestCase):
    def test_new_top_smallest_replaces_old_max(self) -> None:
        ranker = diff.UnifiedDiffRanker(2)
        ranker("file1", ["line1\n", "line2\n"], ["1\n", "2\n"])
        ranker("file2", [], ["1\n", "2\n", "3\n", "4\n", "5\n"])
        ranker("file3", ["a\n", "b\n"], ["a\n", "c\n"])
        act = [diffsize_filename for diffsize_filename, _, _ in ranker]
        exp = [(2, "file3"), (4, "file1")]
        self.assertListEqual(exp, act)
