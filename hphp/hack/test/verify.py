#!/usr/bin/env python3

import argparse
import os.path
import os
import re
import subprocess
import sys
import difflib
import shlex
from collections import namedtuple
from concurrent.futures import ThreadPoolExecutor
from typing import Callable, Dict, List, Optional

max_workers = 48
verbose = False
dump_on_failure = False
batch_size = 500


TestCase = namedtuple('TestCase', ['file_path', 'input', 'expected'])


Result = namedtuple('Result', ['test_case', 'output', 'is_failure'])


"""
Per-test flags passed to test executable. Expected to be in a file with
same name as test, but with .flags extension.
"""


def get_test_flags(path: str) -> List[str]:
    prefix, _ext = os.path.splitext(path)
    path = prefix + '.flags'

    if not os.path.isfile(path):
        return []
    with open(path) as file:
        return shlex.split(file.read().strip())


def run_batch_tests(test_cases: List[TestCase],
                    program: str,
                    default_expect_regex,
                    ignore_error_text,
                    get_flags: Callable[[str], List[str]],
                    out_extension: str) -> List[Result]:
    """
    Run the program with batches of files and return a list of results.
    """
    # Each directory needs to be in a separate batch because flags are different
    # for each directory.
    # Compile a list of directories to test cases, and then
    dirs_to_files : Dict[str, List[TestCase]] = {}
    for case in test_cases:
        test_dir = os.path.dirname(case.file_path)
        dirs_to_files.setdefault(test_dir, []).append(case)

    # run a list of test cases.
    # The contract here is that the program will write to
    # filename.out_extension for each file, and we read that
    # for the output.
    def run(test_cases : List[TestCase]):
        if not test_cases:
            assert False
        first_test = test_cases[0]
        test_dir = os.path.dirname(first_test.file_path)
        flags = get_flags(test_dir)
        test_flags = get_test_flags(first_test.file_path)
        cmd = [program, "--batch-files"]
        cmd += flags + test_flags
        cmd += [os.path.basename(case.file_path) for case in test_cases]
        if verbose:
            print('Executing', ' '.join(cmd))
        try:
            subprocess.call(
                cmd, stderr=subprocess.STDOUT, cwd=test_dir,
                universal_newlines=True)
        except subprocess.CalledProcessError as e:
            # we don't care about nonzero exit codes... for instance, type
            # errors cause hh_single_type_check to produce them
            pass
        results = []
        for case in test_cases:
            with open(case.file_path + out_extension, "r") as f:
                output : str = f.read()
                result = check_result(case, default_expect_regex,
                  ignore_error_text, output)
                results.append(result)
        return results
    # Create a list of batched cases.
    all_batched_cases : List[List[TestCase]] = []

    # For each directory, we split all the test cases
    # into chunks of batch_size. Then each of these lists
    # is a separate job for each thread in the threadpool.
    for cases in dirs_to_files.values():
        batched_cases : List[List[TestCase]] = \
            [cases[i:i + batch_size] for i in range(0, len(cases), batch_size)]
        all_batched_cases += batched_cases

    executor = ThreadPoolExecutor(max_workers=max_workers)
    futures = [executor.submit(run, test_batch) for test_batch in all_batched_cases]

    results = [future.result() for future in futures]
    # Flatten the list
    return [item for sublist in results for item in sublist]

def run_test_program(test_cases: List[TestCase],
                     program: str,
                     default_expect_regex,
                     ignore_error_text,
                     get_flags: Callable[[str], List[str]],
                     timeout=None,
                     ) -> List[Result]:

    """
    Run the program and return a list of results.
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
                universal_newlines=True, input=test_case.input, timeout=timeout)
        except subprocess.TimeoutExpired as e:
            output = "Timed out. " + e.output
        except subprocess.CalledProcessError as e:
            # we don't care about nonzero exit codes... for instance, type
            # errors cause hh_single_type_check to produce them
            output = e.output
        return check_result(test_case, default_expect_regex, ignore_error_text, output)

    executor = ThreadPoolExecutor(max_workers=max_workers)
    futures = [executor.submit(run, test_case) for test_case in test_cases]

    return [future.result() for future in futures]


def filter_ocaml_stacktrace(text: str) -> str:
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
    return "\n".join(out)


def compare_expected(expected, out):
    if (expected == "No errors\n" or out == "No errors\n"):
        return expected == out
    else:
        return True


# Strip leading and trailing whitespace from every line
def strip_lines(text: str) -> str:
    lines = [line.strip() for line in text.splitlines()]
    return "\n".join(lines)


def check_result(test_case: TestCase, default_expect_regex,
  ignore_error_messages, out: str) -> Result:
    """
    Check that the output of the test in :out corresponds to the expected
    output, or if a :default_expect_regex is provided,
    check that the output in :out contains the provided regex.
    """
    is_ok = (
        strip_lines(test_case.expected) == strip_lines(out) or
        (ignore_error_messages and compare_expected(test_case.expected, out)) or
        test_case.expected == filter_ocaml_stacktrace(out) or
        (
            default_expect_regex is not None and
            re.search(default_expect_regex, out) is not None
        )
    )

    return Result(test_case=test_case, output=out, is_failure=not is_ok)


def record_results(results: List[Result], out_ext: str) -> None:
    for result in results:
        outfile = result.test_case.file_path + out_ext
        with open(outfile, 'wb') as f:
            f.write(bytes(result.output, 'UTF-8'))


def report_failures(total: int,
                    failures: List[Result],
                    out_extension: str,
                    expect_extension: str,
                    no_copy: bool=False) -> None:
    record_results(failures, out_extension)
    fnames = [failure.test_case.file_path for failure in failures]
    print("To review the failures, use the following command: ")
    print("OUT_EXT=%s EXP_EXT=%s NO_COPY=%s ./hphp/hack/test/review.sh %s" %
            (out_extension,
            expect_extension,
            "true" if no_copy else "false",
            " ".join(fnames)))
    if dump_on_failure:
        dump_failures(failures)


def dump_failures(failures: List[Result]) -> None:
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


def get_hh_flags(test_dir: str) -> List[str]:
    path = os.path.join(test_dir, 'HH_FLAGS')
    if not os.path.isfile(path):
        if verbose:
            print("No HH_FLAGS file found")
        return []
    with open(path) as f:
        return shlex.split(f.read().strip())


def files_with_ext(files: List[str], ext: str) -> List[str]:
    """
    Returns the set of filenames in :files that end in :ext
    """
    filtered_files: List[str] = []
    for file in files:
        prefix, suffix = os.path.splitext(file)
        if suffix == ext:
            filtered_files.append(prefix)
    return filtered_files


def list_test_files(root: str, disabled_ext: str, test_ext: str) -> List[str]:
    if os.path.isfile(root):
        if root.endswith(test_ext):
            return [root]
        else:
            return []
    elif os.path.isdir(root):
        result: List[str] = []
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


def get_content(file_path: str, ext: str='') -> str:
    try:
        with open(file_path + ext, 'r') as fexp:
            return fexp.read()
    except FileNotFoundError:
        return ''


def run_tests(files: List[str],
              expected_extension: str,
              out_extension: str,
              use_stdin: str,
              program: str,
              default_expect_regex,
              batch_mode: str,
              ignore_error_text: str,
              get_flags: Callable[[str], List[str]],
              timeout=None,
              ) -> List[Result]:

    # for each file, create a test case
    test_cases = [
        TestCase(
            file_path=file,
            expected=get_content(file, expected_extension),
            input=get_content(file) if use_stdin else None)
        for file in files]
    if batch_mode:
        results = run_batch_tests(test_cases, program, default_expect_regex,
            ignore_error_text, get_flags, out_extension)
    else:
        results = run_test_program(test_cases, program, default_expect_regex,
            ignore_error_text, get_flags, timeout=timeout)

    failures = [result for result in results if result.is_failure]

    num_results = len(results)
    if failures == []:
        print("All tests in the suite passed! "
              "The number of tests that ran: %d\n" % num_results)
    else:
        print("The number of tests that failed: %d/%d\n"
            % (len(failures), num_results))
        report_failures(
            num_results,
            failures,
            args.out_extension,
            args.expect_extension)
        sys.exit(1)  # this exit code fails the suite and lets Buck know

    return results


def run_idempotence_tests(results: List[Result],
                          expected_extension: str,
                          out_extension: str,
                          program: str,
                          default_expect_regex,
                          get_flags: Callable[[str], List[str]]) -> None:
    idempotence_test_cases = [
        TestCase(
            file_path=result.test_case.file_path,
            expected=result.test_case.expected,
            input=result.output)
        for result in results]

    idempotence_results = run_test_program(
        idempotence_test_cases, program, default_expect_regex, False, get_flags)

    num_idempotence_results = len(idempotence_results)

    idempotence_failures = [
        result for result in idempotence_results if result.is_failure]

    if idempotence_failures == []:
        print("All idempotence tests in the suite passed! The number of "
              "idempotence tests that ran: %d\n" % num_idempotence_results)
    else:
        print("The number of idempotence tests that failed: %d/%d\n"
            % (len(idempotence_failures), num_idempotence_results))
        report_failures(
            num_idempotence_results,
            idempotence_failures,
            out_extension + out_extension,  # e.g., *.out.out
            expected_extension,
            no_copy=True)
        sys.exit(1)  # this exit code fails the suite and lets Buck know


def get_flags_cache(args_flags: List[str]) -> Callable[[str], List[str]]:
    flags_cache: Dict[str, List[str]] = {}

    def get_flags(test_dir: str) -> List[str]:
        if test_dir not in flags_cache:
            flags_cache[test_dir] = get_hh_flags(test_dir)
        flags = flags_cache[test_dir]
        if args_flags is not None:
            flags = flags + args_flags
        return flags

    return get_flags


if __name__ == '__main__':
    # Defining this function to make the Flake8 linter happy (error T484)
    # instead of passing os.path.abspath directly to add_argument. A different
    # linter/type checker may not have this issue.
    def abspath(path: str) -> str:
        return os.path.abspath(path)

    parser = argparse.ArgumentParser()
    parser.add_argument('test_path', help='A file or a directory. ')
    parser.add_argument('--program', type=abspath)
    parser.add_argument('--out-extension', type=str, default='.out')
    parser.add_argument('--expect-extension', type=str, default='.exp')
    parser.add_argument('--default-expect-regex', type=str)
    parser.add_argument('--in-extension', type=str, default='.php')
    parser.add_argument('--disabled-extension', type=str,
                        default='.no_typecheck')
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--idempotence', action='store_true',
                        help="Verify that the output passed to the program "
                        "as input results in the same output.")
    parser.add_argument('--max-workers', type=int, default='48')
    parser.add_argument('--diff', action='store_true',
                        help="On test failure, show the content of "
                        "the files and a diff")
    parser.add_argument('--flags', nargs=argparse.REMAINDER)
    parser.add_argument('--stdin', action='store_true',
                        help='Pass test input file via stdin')
    parser.add_argument('--batch', action='store_true',
                        help='Run tests in batches to the test program')
    parser.add_argument("--ignore-error-text", action='store_true',
                        help='Do not compare error text when verifying output')
    parser.add_argument("--timeout", type=int,
                    help='Timeout in seconds for each test, in non-batch mode.')
    parser.epilog = "%s looks for a file named HH_FLAGS in the same directory" \
                    " as the test files it is executing. If found, the " \
                    "contents will be passed as arguments to " \
                    "<program> in addition to any arguments specified by --flags" % parser.prog
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

    get_flags = get_flags_cache(args.flags)

    results = run_tests(
        files,
        args.expect_extension,
        args.out_extension,
        args.stdin,
        args.program,
        args.default_expect_regex,
        args.batch,
        args.ignore_error_text,
        get_flags,
        timeout=args.timeout)

    # Doesn't make sense to check failures for idempotence
    successes = [result for result in results if not result.is_failure]

    if args.idempotence and successes:
        run_idempotence_tests(
            successes,
            args.expect_extension,
            args.out_extension,
            args.program,
            args.default_expect_regex,
            get_flags)
