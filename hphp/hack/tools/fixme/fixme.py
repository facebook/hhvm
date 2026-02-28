# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

# pyre-unsafe

# fixme.py -- Adds HH_FIXME annotations in your code to clean up after a new
#             check in the typechecker or after an upgrade. Takes two arguments:
#               1) A Filename, which is the output of `hh_client --json`.
#               2) An explanation to insert into each "fixme" comment.

# Known problems:
#
# - Will insert HH_FIXME comments in the body of XHP elements, where they will
#   be rendered to the page. Thankfully the typechecker does not accept them at
#   this location, and so it won't actually silence the error and can be fixed
#   manually.

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import re
import sys
import textwrap
from collections import defaultdict


class ParseException(Exception):
    pass


def build_fixmes(raw_json_obj):
    fixmes = defaultdict(lambda: defaultdict(set))
    for error in raw_json_obj["errors"]:
        main_message = error["message"][0]

        path = main_message["path"]
        line = main_message["line"] - 1  # Hack is 1-indexed, py lists 0-indexed
        code = main_message["code"]

        fixmes[path][line].add(code)

    return fixmes


WHITESPACE_PATTERN = re.compile(r"\s*")


def is_parse_error(code):
    return code >= 1000 and code < 2000


def patch(path, patches, explanation):
    with open(path, "r") as f:
        file_lines = f.readlines()

    # Insert fixme lines bottom up, so that changes don't change line numbers
    # later in the file.
    for line, codes in sorted(patches.items(), reverse=True):
        target_line = file_lines[line]
        whitespace = WHITESPACE_PATTERN.match(target_line).group()

        for code in codes:
            if is_parse_error(code):
                raise ParseException()
            fixme_message = "HH_FIXME[%d]: %s" % (code, explanation)
            # Wrap before 80 characters
            # Don't forget comment delimiters and whitespace
            # TODO: account for tab size?
            fixme_lines = textwrap.wrap(fixme_message, 74 - len(whitespace))
            num_lines = len(fixme_lines)
            if num_lines == 1:
                full_line = "%s/* %s */\n" % (whitespace, fixme_lines[0])
                file_lines.insert(line, full_line)
            else:
                for i in reversed(range(num_lines)):
                    fixme_line = fixme_lines[i]
                    if i == 0:
                        full_line = "%s/* %s\n" % (whitespace, fixme_line)
                    elif i + 1 == num_lines:
                        full_line = "%s   %s */\n" % (whitespace, fixme_line)
                    else:
                        full_line = "%s   %s\n" % (whitespace, fixme_line)
                    file_lines.insert(line, full_line)

    with open(path, "w") as f:
        f.writelines(file_lines)


def main(args):
    if len(args) != 3:
        print("Usage: fixme.py hh_output.json explanation")
        return 1

    explanation = args[2]

    with open(args[1], "r") as f:
        raw_data = f.read()

    raw_json_obj = json.loads(raw_data)
    fixmes = build_fixmes(raw_json_obj)
    failures = 0

    for path, patches in fixmes.items():
        try:
            patch(path, patches, explanation)
        except ParseException:
            failures += 1
            print("Not patching %s as it has parse errors" % path)

    print("Patched %d files with HH_FIXME" % (len(fixmes) - failures))

    return 0


def invoke_main() -> None:
    sys.exit(main(sys.argv))


if __name__ == "__main__":
    invoke_main()
