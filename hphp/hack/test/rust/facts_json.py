#!/usr/bin/env python3
# Copyright (c) 2019, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.;

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
from difflib import unified_diff
from glob import glob, iglob
from typing import Iterable, List

from libfb.py.fbcode_root import get_fbcode_dir


HACK_TEST_DIR_DEFAULT = os.path.join(get_fbcode_dir(), "hphp/hack/test")


def parse_args():
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--dir",
        default=HACK_TEST_DIR_DEFAULT,
        help="root directory containing Hack files to be tested",
    )
    ap.add_argument(
        "--file",
        "-f",
        action="append",
        help="test specific file only (may be repeated)",
    )
    ap.add_argument(
        "--keep-going",
        action="store_true",
        help="do not stop after the first failure (consider with --no-diff)",
    )
    ap.add_argument(
        "--no-diff",
        dest="diff",
        action="store_false",
        help="do not show diff of OCaml vs Rust output, respectively",
    )
    ap.add_argument("--show-ok", action="store_true", help="show passing tests")
    ap.add_argument(
        "--skip-re",
        default=r"^$",
        help="Skip input files matching this PERL-style Regular Expression",
    )
    ap.add_argument("passthrough_args", nargs="*")
    return ap.parse_args()


def find_hack_files(root: str, skip_re) -> Iterable[str]:
    for ext in (".hack", ".hhi", ".php"):
        for path in iglob(os.path.join(root, "**/*" + ext), recursive=True):
            if not skip_re.search(path):
                yield path


def locate_binary_under_test(suffix) -> str:
    path = glob(get_fbcode_dir() + "/buck-out/**/hphp/hack/test/rust/" + suffix)
    assert len(path) == 1, "Found {} binaries (try `buck clean`) with suffix {}".format(
        len(path), suffix
    )
    return path[0]


def test_all(paths: Iterable[str], args: List[str], on_failure, on_success):
    def binary_output_as_pretty_json(suffix: str, path: str) -> List[str]:
        cmd = [locate_binary_under_test(suffix)] + args + ["--file-path", path]
        print("RUN: " + " ".join(map(shlex.quote, cmd)))
        obj = json.loads(subprocess.check_output(cmd))
        return json.dumps(obj, sort_keys=False, indent=2).split("\n")

    correct, total = 0, 0
    for path in paths:
        total += 1
        caml_output = binary_output_as_pretty_json(
            "facts_parse_ocaml/facts_parse_ocaml.opt", path
        )
        rust_output = binary_output_as_pretty_json(
            "facts_parse_rust#binary/facts_parse_rust", path
        )
        ok = rust_output == caml_output
        if ok:
            correct += 1

        print("%d/%d" % (correct, total))
        if not ok:
            print("FAILED:", path)
            on_failure(caml_output, rust_output)
        else:
            on_success(path)


if __name__ == "__main__":
    args = parse_args()

    def on_failed(caml_output, rust_output):
        if args.diff:
            for line in unified_diff(caml_output, rust_output):
                if not (line.startswith("---") or line.startswith("+++")):
                    print(line)
        if not args.keep_going:
            sys.exit(1)

    def on_success(path):
        if args.show_ok:
            print("OK:", path)

    dir_skip_re = re.compile(args.skip_re)
    paths = args.file if args.file else find_hack_files(args.dir, dir_skip_re)
    test_all(paths, args.passthrough_args, on_failed, on_success)
