#!/usr/bin/env python3

import argparse
import inspect
import pathlib as pl
import re
import sys


def out2expectf_main():
    this_file = pl.Path(inspect.getfile(inspect.currentframe())).resolve()
    root = this_file.parent.parent.parent
    root_re = re.escape(str(root))

    parser = argparse.ArgumentParser(
        description="Remove machine or test run specific data from test output."
    )
    parser.add_argument(
        "test_directory",
        help="Temporary test directory of recent hphp/test/run",
        type=pl.Path,
    )
    parser.add_argument(
        "test_files",
        nargs="+",
        help="List of test files to grab output of, relative to fbcode root",
        type=pl.Path,
    )

    args = parser.parse_args()

    for test in args.test_files:
        if (not str(test).endswith(".php")) and (not str(test).endswith(".hack")):
            print("%s doesn't end in .php. Pass the .php file to this script." % test)
            sys.exit(1)

        try:
            test_file = args.test_directory / pl.Path(str(test) + '.out')
            data = open(test_file).read()
        except IOError:
            print("%s doesn't exist, skipping" % test_file)
            continue

        try:
            # the first match has to be in a try incase there is bad unicode
            re.sub(r"a", r"a", data)
        except UnicodeDecodeError:
            print("%s has invalid unicode, skipping" % test)
            continue

        # try to do relative paths
        data = re.sub(
            r'string\(\d+\) "(#\d+) ' + root_re + r"(/hphp)?", r'string(%d) "\1 %s', data
        )
        data = re.sub(r'string\(\d+\) "' + root_re + r"(/hphp)?", r'string(%d) "%s', data)
        data = re.sub(root_re + r"(/hphp)?", r"%s", data)

        # The debugger prints the path given on the command line, which is often
        # relative. All such debugger tests live under something/debugger/foo.php.
        data = re.sub(r"[^ ]*/debugger(/[^ ]*.(php|hack))", r"%s\1", data)

        # Generator method names are, well, generated!
        # See ParserBase::getAnonFuncName.
        data = re.sub(r" [0-9]+_[0-9]+\(", r" %d_%d(", data)

        # Closure names change
        data = re.sub(r"Closure\$\$[0-9a-f]+\$", r"Closure%s", data)

        # Closure class name
        data = re.sub(r'string\(7\) "Closure"', r'string(%d) "Closure%s"', data)

        # Left over Closure class names
        data = re.sub(r"Closure(?!%s)", r"Closure%s", data)

        if "%" in data:
            name = str(test) + ".expectf"
        else:
            name = str(test) + ".expect"

        open(name, "w").write(data)
        print("Copied %s.out to %s" % (args.test_directory / test, name))


if __name__ == "__main__":
    out2expectf_main()
