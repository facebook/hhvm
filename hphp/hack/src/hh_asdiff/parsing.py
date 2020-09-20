#!/usr/bin/python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

import re
import typing as ty
from typing import AnyStr, Callable, Iterator, List, Optional, Tuple


def canon(string_like: AnyStr) -> str:
    """Canonicalizes a line of HHAS output by escaping non-UTF8 chars"""
    if isinstance(string_like, bytes):
        return string_like.decode("utf-8", "backslashreplace")
    assert isinstance(string_like, str)
    return string_like


# Note: must be consistent with hhbc_hhas.{ml,rs} files
_MARKER_BEGIN_RE: ty.Pattern[str] = re.compile(r"#(.*)starts here")
_MARKER_END_RE: ty.Pattern[str] = re.compile(r"#(.*)ends here")

MarkedFilename = Optional[str]


def split_lines(
    linegen: Iterator[AnyStr], on_ignore: Callable[[str], None] = lambda s: None
) -> Iterator[Tuple[MarkedFilename, ty.Sequence[str]]]:
    """Splits HHAS output (bytecode) that may contain starts/ends-here markers
    into pairs (marked_filename, hhas_lines) by only keeping one file in memory,
    useful splitting potentially huge output of "hh_(single_)compile DIR".
    For extra sanity checking, an optional list to which ignored lines
    (i.e., those outside the marked section for each file) can be passed.
    Example:
        ignored_lines = []
        single_hhas_gen = split_lines(hh_compile_output_lines):
        for filename, hhas in single_hhas_gen:
             print("=== FILENAME: {filename} ===")
             sys.stdout.writelines(hhas)
        assert not ignored_lines  # make sure no lines is silently ignored
    """
    filename: MarkedFilename
    hhas_lines: List[str] = []
    hhas_active = False
    for line0 in linegen:
        line = canon(line0)
        beg_match = _MARKER_BEGIN_RE.match(line)
        if beg_match:
            filename = beg_match.group(1).strip() or None  # None if empty
            hhas_lines = []  # avoid .clear()) so caller doesn't have to copy
            hhas_active = True
        elif _MARKER_END_RE.match(line):
            hhas_active = False
            yield filename, hhas_lines
            hhas_lines = []
        elif hhas_active:
            hhas_lines.append(line)
        elif line:
            on_ignore(line)

    # Include HHAS segments without an end marked in ignored list
    for line in hhas_lines:
        if line:
            on_ignore(line)
