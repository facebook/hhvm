# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

from __future__ import annotations

import argparse
import re
import subprocess
import sys


def main() -> None:
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument("--checker", required=True)
    options, flags = parser.parse_known_args()
    result = subprocess.run(
        [options.checker, *flags],
        capture_output=True,
        text=True,
        check=False,
    )
    # The timestamp varies, but retain the native JSON, order, and multiplicity.
    stderr = re.sub(
        r"(?m)^\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\] "
        r"(?=\[FBID_IGID_type_logger\])",
        "",
        result.stderr,
    )
    stderr = re.sub(r"/tmp/[^/]*/([a-zA-Z0-9_]+\.hhi)", r"/tmp/hhi_dir/\1", stderr)
    sys.stdout.write(stderr)
    sys.stdout.write(result.stdout)
    sys.exit(result.returncode)
