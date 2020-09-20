#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

import unittest
from typing import Iterator

from hphp.hack.src.hh_asdiff import parsing


def iter_lines(s: str) -> Iterator[str]:
    return iter(s.splitlines(keepends=True))


class SplitTest(unittest.TestCase):
    def test_lines_single_marker_pair_no_filename(self) -> None:
        lines = iter_lines(
            """\
GARBAGE1
#starts here
.main {
  Int 1
  RetC
}
#ends here
GARBAGE2
"""
        )
        line_iter = iter(lines)
        ignored_lines = []
        gen = parsing.split_lines(line_iter, lambda ln: ignored_lines.append(ln))
        exp1 = (None, [".main {\n", "  Int 1\n", "  RetC\n", "}\n"])
        self.assertEqual(next(gen, None), exp1)
        self.assertIsNone(next(gen, None))
        self.assertEqual(ignored_lines, ["GARBAGE1\n", "GARBAGE2\n"])

    def test_split_multiple_files_with_filenames(self) -> None:
        line_iter = iter(
            parsing.split_lines(
                iter_lines(
                    """\
# dir/file1.php starts here
HHAS1
# dir/file1.php ends here
# dir/file2.php starts here
HHAS2a
HHAS2b
# dir/file2.php ends here
"""
                )
            )
        )
        self.assertEqual(next(line_iter, None), ("dir/file1.php", ["HHAS1\n"]))
        self.assertEqual(
            next(line_iter, None), ("dir/file2.php", ["HHAS2a\n", "HHAS2b\n"])
        )

    def test_lines_no_markers_ignores_all(self) -> None:
        count = 0
        lines = ["1\n", "2\n", "3\n"]

        # pyre-fixme[53]: Captured variable `count` is not annotated.
        def count_ignored(_) -> None:
            nonlocal count
            count += 1

        list(parsing.split_lines(iter(lines), count_ignored))  # force evaluation
        self.assertEqual(count, len(lines))
