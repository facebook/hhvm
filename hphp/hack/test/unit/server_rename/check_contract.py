# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

from __future__ import annotations

import argparse
import re
from pathlib import Path


def check_case(source: Path, allow_broken: bool) -> None:
    native = Path(f"{source}.wrapper.exp").read_text()
    diagnostic = Path(f"{source}.exp").read_text()
    assert native.startswith("<?hh"), f"Invalid generated source: {source}"
    assert len(re.findall(r"\bfunction renamed\(", native)) == 1, source
    suppressed = "// @no-wrapper" in source.read_text()
    expected_wrappers = 0 if suppressed else 1
    assert (
        native.count('<<__Deprecated("Use `renamed` instead")>>') == expected_wrappers
    ), source
    assert len(re.findall(r"\bfunction original\(", native)) == expected_wrappers, (
        source
    )
    records = re.findall(r"\((Parsing|Naming|NastCheck|Typing)\[(\d+)\]\)", diagnostic)
    if diagnostic != "No errors\n":
        assert re.match(
            r'^ERROR: File "[^"]+", line \d+, characters \d+-\d+:', diagnostic
        ), (source, diagnostic)
        assert len(re.findall(r"^ERROR:", diagnostic, re.MULTILINE)) == len(records), (
            source,
            diagnostic,
        )
        assert not any(
            marker in diagnostic
            for marker in (
                "Traceback",
                "Uncaught exception",
                "Fatal error",
                "Internal error",
            )
        ), (source, diagnostic)
    if source.stem.endswith("_bad"):
        assert records, (source, diagnostic)
        codes = {int(code) for _, code in records}
        expected_codes = {
            "missing_named_bad": {4499, 4105},
            "named_type_bad": {4110},
            "deprecated_api_bad": {4128},
            "nameless_positional_variadic_bad": {1002},
        }
        if source.stem in expected_codes:
            expected = expected_codes[source.stem]
            assert expected <= codes if allow_broken else expected == codes, (
                source,
                diagnostic,
            )
    elif not allow_broken:
        assert diagnostic == "No errors\n", (source, diagnostic)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("directory", type=Path)
    parser.add_argument("--allow-broken", action="store_true")
    args = parser.parse_args()
    directory = args.directory
    if Path.cwd().name == "fbsource":
        directory = Path("fbcode") / directory
    sources = set(directory.glob("*.php"))
    assert sources, "No wrapper fixtures found"
    for golden in directory.glob("*.exp"):
        suffix = ".wrapper.exp" if golden.name.endswith(".wrapper.exp") else ".exp"
        assert golden.with_name(golden.name.removesuffix(suffix)) in sources, (
            f"Orphan golden: {golden}"
        )
    for source in sorted(sources):
        check_case(source, args.allow_broken)
    print(f"Validated {len(sources)} native/checker snapshot pairs")
