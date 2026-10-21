# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--generator", required=True)
    parser.add_argument("--checker", required=True)
    parser.add_argument("file")
    args = parser.parse_args()
    generated = subprocess.check_output([args.generator, args.file], text=True)
    checker = os.path.abspath(args.checker)
    # Check the generated source, not a hand-written approximation of the call.
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / Path(args.file).name
        path.write_text(generated)
        result = subprocess.run(
            [
                checker,
                "--config",
                "everything_sdt=false",
                "--config",
                "allowed_fixme_codes_strict=4128",
                "--config",
                "named_variadic_type=true",
                "--config",
                "variadic_named_parameters=true",
                "--error-format",
                "plain",
                path.name,
            ],
            cwd=directory,
            capture_output=True,
            text=True,
        )
        if result.returncode not in (0, 2):
            raise RuntimeError(result.stdout + result.stderr)
        sys.stdout.write(
            (result.stdout + result.stderr).replace(
                str(path), os.path.basename(args.file)
            )
        )
