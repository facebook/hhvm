#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

import difflib
from heapq import heappush, heappushpop
from itertools import zip_longest
from typing import AnyStr, Iterator, List, Sequence, Tuple


def equal_lines(expected: Iterator[AnyStr], actual: Iterator[AnyStr]) -> bool:
    """Compares each line of expected input against the actual input,
    where the inputs may differ in their type (i.e., opposite);
    returns true if and only if the sequences have the same number of elements
    and each corresponding element is equal as defined by operator `==`.
    Prefer this function over the error prone `==`, which doesn't work if:
    - one line iterator is infinite (e.g., piped input & infinite loop)
    - sequences are of different types (e.g., list vs tuple vs ...)
    """

    # Use zip_longest to properly handle the case when one iterable
    # produces more elements - the other should then produce None, so that
    # inputs aren't equal (the longer cannot produce None due to typing)
    return all(exp == act for exp, act in zip_longest(expected, actual))


Entry = Tuple[Tuple[int, str], Sequence[str], Sequence[str]]


class UnifiedDiffRanker:
    """Ranks the diffs according to how many lines need to be added or deleted,
    counting either with equal weight.  Calling this object registers a diff,
    and iterating over it gives up to `limit` smallest diffs, typed as:
      ((num_lines_changed, key), lines1, lines2): Entry
    where `key` should be unique, and `lines1`/`lines2` correspond
    to the entire chunk of input being diffed, and `num_lines_changed` to the
    number of extra/missing lines (denoted by `+`/`-` in diff output).

    Example:
      ranker = UnifiedDiffRanker(2)
      ranker("file1", ["line1\n", "line2\n"], ["1\n", "2\n"])
      ranker("file2", [], ["1\n", "2\n", "3\n", "4\n", "5\n"])
      ranker("file3", ["a\n", "b\n"], ["a\n", "c\n"])

      for rank, ((filename, _), _, _) in enumerate(ranker, from=1):
          print(f"Rank #{rank}: {filename}")
    Output:
      Rank #1: file3  # because all 1 extra & 1 missing lines (b/c)
      Rank #2: file1  # because 4 lines differ

    Note: calling this function-like object with the same key multiple times
    may incur significantly slower performance (as lines will be compared).
    """

    # Max-heap storing K smallest entries (or less than <K calls on self made)
    # Invariant: after N <= limit calls on self, contains keys for N smallest
    _heap: List[Entry]

    def __init__(self, limit: int) -> None:
        self._heap = []
        self._limit = limit

    def __call__(self, key: str, lines1: Sequence[str], lines2: Sequence[str]) -> None:
        if self._limit == 0:
            return  # skip expensive diffing if limit is 0

        diff_size = 0
        for d in difflib.unified_diff(lines1, lines2, n=0):
            if len(d) >= 1 and (
                (d[0] == "+" and not d.startswith("+++"))
                or (d[0] == "-" and not d.startswith("---"))
            ):
                diff_size += 1

        # Ensure: old max is replaced with a new diff in top `limit` smallest
        op = heappushpop if len(self._heap) >= self._limit else heappush
        op(self._heap, ((-diff_size, key), lines1, lines2))

    def __iter__(self) -> Iterator[Entry]:
        # By induction, the heap holds (up to) `limit` smallest diffs
        for e in sorted(self._heap, key=lambda e: (-e[0][0], e[0][1])):
            (neg_diff_size, path), lines1, lines2 = e
            yield (-neg_diff_size, path), lines1, lines2
