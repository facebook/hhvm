# pyre-strict

import logging
import os
import re
from typing import Dict, List

import attr

# pyre-fixme[24]: Generic type `re.Pattern` expects 1 type parameter.
MULTIFILE_SPLITTER: re.Pattern = re.compile(r"^////\s*(.*?)\s*$")

# pyre-fixme[24]: Generic type `re.Pattern` expects 1 type parameter.
FILENAME_MATCHER: re.Pattern = re.compile(r"^(base|changed)-(.+?)\s*$")


@attr.s(auto_attribs=True, str=True)
class MultifileParseError(Exception):
    filename: str
    line_number: int
    error: str


@attr.s(auto_attribs=True, str=True)
class FanoutTestReadError(Exception):
    filename: str
    error: str


def parse_multifile_contents(filename: str) -> Dict[str, str]:
    result = {}
    current_file = None
    current_body = []

    with open(filename, "r") as fp:
        for line_no, line in enumerate(fp):
            line_no = line_no + 1

            m = MULTIFILE_SPLITTER.match(line)
            if m:
                if current_file is not None:
                    result[current_file] = "".join(current_body)
                current_file = m.group(1)
                current_body = []
                if len(current_file) == 0:
                    raise MultifileParseError(
                        filename, line_no, "No filename specified on this line."
                    )
                if current_file in result:
                    raise MultifileParseError(
                        filename, line_no, "Duplicate filename on this line."
                    )
            else:
                if current_file is None:
                    raise MultifileParseError(
                        filename, line_no, "Expected a multifile filename on this line."
                    )
                current_body.append(line)

    if current_file is not None:
        result[current_file] = "".join(current_body)

    return result


@attr.s(auto_attribs=True)
class FanoutTest:
    """Represents a fanout test scenario.

    We make use of the multifile format, e.g.:

    ```
    //// base-a.php
    <?hh
    class A {}
    //// base-b.php
    class B extends A {}

    //// changed-a.php
    <?hh
    class A { public function foo(): void {}}
    //// changed-b.php
    <?hh
    class B extends A {}
    ```

    Corresponds to a repo with two files `a.php` and `b.php`
    """

    filename: str
    base_php_contents: Dict[str, str]
    changed_php_contents: Dict[str, str]

    @staticmethod
    def from_file(filename: str) -> "FanoutTest":
        multifile = parse_multifile_contents(filename)
        base_php_contents = {}
        changed_php_contents = {}
        for fn, contents in multifile.items():
            m = FILENAME_MATCHER.match(fn)
            if not m:
                raise FanoutTestReadError(
                    filename=filename,
                    error="Could not interpret multifile item with name {}".format(fn),
                )
            kind = m.group(1)
            subfn = m.group(2)
            if kind == "base":
                base_php_contents[subfn] = contents
            elif kind == "changed":
                changed_php_contents[subfn] = contents
            else:
                raise AssertionError()

        return FanoutTest(
            filename=filename,
            base_php_contents=base_php_contents,
            changed_php_contents=changed_php_contents,
        )

    def prepare_base_php_contents(self, destination_dir: str) -> None:
        for fn, contents in self.base_php_contents.items():
            fn = os.path.join(destination_dir, fn)
            logging.debug("Writing to %s:\n%s\nEOF", fn, contents)
            with open(fn, "w") as fp:
                fp.write(contents)

    def prepare_changed_php_contents(self, destination_dir: str) -> List[str]:
        changed_files = [
            fn for fn in self.base_php_contents if fn not in self.changed_php_contents
        ]
        for fn in changed_files:
            fn = os.path.join(destination_dir, fn)
            logging.debug("Removing %s", fn)
            os.unlink(fn)
        for fn, contents in self.changed_php_contents.items():
            if self.base_php_contents.get(fn, "").strip() != contents.strip():
                changed_files.append(fn)
                fn = os.path.join(destination_dir, fn)
                logging.debug("Writing to %s:\n%s\nEOF", fn, contents)
                with open(fn, "w") as fp:
                    fp.write(contents)
        return changed_files

    def all_base_php_files(self) -> List[str]:
        return list(self.base_php_contents.keys())
