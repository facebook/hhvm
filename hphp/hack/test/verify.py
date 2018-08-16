#!/usr/bin/env python3

import argparse
import os.path
import os
import subprocess
import sys
import difflib
import shlex
from collections import namedtuple
from concurrent.futures import ThreadPoolExecutor

max_workers = 48
verbose = False
dump_on_failure = False


TestCase = namedtuple('TestCase', ['file_path', 'input', 'expected'])


Result = namedtuple('Result', ['test_case', 'output', 'is_failure'])


"""
Per-test flags passed to test executable. Expected to be in a file with
same name as test, but with .flags extension.
"""


def get_test_flags(f):
    prefix, _ext = os.path.splitext(f)
    path = prefix + '.flags'

    if not os.path.isfile(path):
        return []
    with open(path) as f:
        return shlex.split(f.read().strip())


def run_test_program(test_cases, program, get_flags):
    """
    Run the program and return a list of Failures.
    """
    def run(test_case):
        test_dir, test_name = os.path.split(test_case.file_path)
        flags = get_flags(test_dir)
        test_flags = get_test_flags(test_case.file_path)
        cmd = [program]
        if test_case.input is None:
            cmd.append(test_name)
        cmd += flags + test_flags
        if verbose:
            print('Executing', ' '.join(cmd))
        try:
            output = subprocess.check_output(
                cmd, stderr=subprocess.STDOUT, cwd=test_dir,
                universal_newlines=True, input=test_case.input)
        except subprocess.CalledProcessError as e:
            # we don't care about nonzero exit codes... for instance, type
            # errors cause hh_single_type_check to produce them
            output = e.output
        return check_result(test_case, output)

    executor = ThreadPoolExecutor(max_workers=max_workers)
    futures = [executor.submit(run, test_case) for test_case in test_cases]

    return [future.result() for future in futures]


def filter_ocaml_stacktrace(text):
    """take a string and remove all the lines that look like
    they're part of an OCaml stacktrace"""
    assert isinstance(text, str)
    it = text.splitlines()
    out = []
    for x in it:
        drop_line = (
            x.lstrip().startswith("Called") or
            x.lstrip().startswith("Raised")
        )
        if drop_line:
            pass
        else:
            out.append(x)
    # force trailing newline
    return "\n".join(out) + "\n"


def check_result(test_case, out):
    is_failure = (
        test_case.expected != out and
        test_case.expected != filter_ocaml_stacktrace(out))

    return Result(test_case=test_case, output=out, is_failure=is_failure)


def record_results(results, out_ext):
    for result in results:
        outfile = result.test_case.file_path + out_ext
        with open(outfile, 'wb') as f:
            f.write(bytes(result.output, 'UTF-8'))


def dump_failures(failures):
    for f in failures:
        expected = f.test_case.expected
        actual = f.output
        diff = difflib.ndiff(
            expected.splitlines(1),
            actual.splitlines(1))
        print("Details for the failed test %s:" % f.test_case.file_path)
        print("\n>>>>>  Expected output  >>>>>>\n")
        print(expected)
        print("\n=====   Actual output   ======\n")
        print(actual)
        print("\n<<<<< End Actual output <<<<<<<\n")
        print("\n>>>>>       Diff        >>>>>>>\n")
        print(''.join(diff))
        print("\n<<<<<     End Diff      <<<<<<<\n")


def get_hh_flags(test_dir):
    path = os.path.join(test_dir, 'HH_FLAGS')
    if not os.path.isfile(path):
        if verbose:
            print("No HH_FLAGS file found")
        return []
    with open(path) as f:
        return shlex.split(f.read().strip())


def files_with_ext(files, ext):
    """
    Returns the set of filenames in :files that end in :ext
    """
    result = set()
    for f in files:
        prefix, suffix = os.path.splitext(f)
        if suffix == ext:
            result.add(prefix)
    return result


def list_test_files(root, disabled_ext, test_ext):
    if os.path.isfile(root):
        if root.endswith(test_ext):
            return [root]
        else:
            return []
    elif os.path.isdir(root):
        result = []
        children = os.listdir(root)
        disabled = files_with_ext(children, disabled_ext)
        for child in children:
            if child != 'disabled' and child not in disabled:
                result.extend(
                    list_test_files(
                        os.path.join(root, child),
                        disabled_ext,
                        test_ext))
        return result
    elif os.path.islink(root):
        # Some editors create broken symlinks as part of their locking scheme,
        # so ignore those.
        return []
    else:
        raise Exception('Could not find test file or directory at %s' %
            args.test_path)


def get_content(file_path, ext=''):
    try:
        with open(file_path + ext, 'r') as fexp:
            return fexp.read()
    except FileNotFoundError:
        return ''


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('test_path', help='A file or a directory. ')
    parser.add_argument('--program', type=os.path.abspath)
    parser.add_argument('--out-extension', type=str, default='.out')
    parser.add_argument('--expect-extension', type=str, default='.exp')
    parser.add_argument('--in-extension', type=str, default='.php')
    parser.add_argument('--disabled-extension', type=str,
                        default='.no_typecheck')
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--max-workers', type=int, default='48')
    parser.add_argument('--diff', action='store_true',
                        help="On test failure, show the content of "
                        "the files and a diff")
    parser.add_argument('--flags', nargs=argparse.REMAINDER)
    parser.add_argument('--stdin', action='store_true',
                        help='Pass test input file via stdin')
    parser.epilog = "Unless --flags is passed as an argument, "\
                    "%s looks for a file named HH_FLAGS in the same directory" \
                    " as the test files it is executing. If found, the " \
                    "contents will be passed as arguments to " \
                    "<program>." % parser.prog
    args = parser.parse_args()

    max_workers = args.max_workers
    verbose = args.verbose
    dump_on_failure = args.diff

    if os.getenv('SANDCASTLE') is not None:
        dump_on_failure = True

    if not os.path.isfile(args.program):
        raise Exception('Could not find program at %s' % args.program)

    files = list_test_files(
        args.test_path,
        args.disabled_extension,
        args.in_extension)

    if len(files) == 0:
        raise Exception(
            'Could not find any files to test in ' + args.test_path)

    flags_cache = {}

    def get_flags(test_dir):
        if args.flags is not None:
            flags = args.flags
        else:
            if test_dir not in flags_cache:
                flags_cache[test_dir] = get_hh_flags(test_dir)
            flags = flags_cache[test_dir]
        return flags

    # for each file, create a test case

    test_cases = []
    for f in files:
        exp = get_content(f, args.expect_extension)
        input = get_content(f) if args.stdin else None
        test_cases.append(TestCase(file_path=f, expected=exp, input=input))

    results = run_test_program(test_cases, args.program, get_flags)

    failures = [r for r in results if r.is_failure]

    total = len(files)
    if failures == []:
        print("All %d tests passed!\n" % total)
    else:
        record_results(failures, args.out_extension)
        fnames = [failure.test_case.file_path for failure in failures]
        print("To review the failures, use the following command: ")
        print("OUT_EXT=%s EXP_EXT=%s ./hphp/hack/test/review.sh %s" %
                (args.out_extension, args.expect_extension, " ".join(fnames)))
        if dump_on_failure:
            dump_failures(failures)
        print("Failed %d out of %d tests." % (len(failures), total))
        sys.exit(1)
