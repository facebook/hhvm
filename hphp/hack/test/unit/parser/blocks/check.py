#!/usr/bin/env python3

import os
import subprocess
import sys

PHP_EXTENSION = ".php"


def find_files(dir):
    """ Find all PHP files in a directory
    """
    files = [os.path.join(dir, f) for f in os.listdir(dir)]
    return list(filter(lambda f: f.endswith(PHP_EXTENSION), files))


def get_expected(file):
    """ Count the number of '// expect block' in a file
    """
    count = 0
    with open(file, 'r') as f:
        for line in f.read().split('\n'):
            if line == "// expect block":
                count += 1
    return count


def run_hh_single_parse(hh_single_parse, file):

    cmd = [hh_single_parse, file]

    try:
        out = subprocess.check_output(cmd)
        return str(out).count("Block")
    except subprocess.CalledProcessError as e:
        print("Error when running hh_single_parse on %s%s"
            % (file, " with FFP"))
        raise e


def run_test(hh_single_parse, file):
    """ Compare the number of blocks in the AST and the number of blocks
    as declared in the file
    """
    failed = False
    expected = get_expected(file)
    try:
        ffp_result = run_hh_single_parse(hh_single_parse, file)
    except subprocess.CalledProcessError as e:
        return True
    if ffp_result != expected:
        print("%s expected %d blocks, FFP gives %d blocks"
            % (file, expected, ffp_result))
        failed = True
    return failed


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Invalid arguments")
        sys.exit(1)
    dir = sys.argv[1]
    hh_single_parse = sys.argv[2]
    files = find_files(dir)
    results = [run_test(hh_single_parse, f) for f in files]
    if any(results):
        sys.exit(42)
