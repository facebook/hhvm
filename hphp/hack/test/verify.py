#!/usr/bin/env python3
# pyre-strict

import argparse
import difflib
import os
import os.path
import re
import shlex
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from enum import Enum
from typing import Callable, Dict, List, Optional, Tuple

from hphp.hack.test.parse_errors import Error, parse_errors, sprint_errors

DEFAULT_OUT_EXT = ".out"
DEFAULT_EXP_EXT = ".exp"

flags_pessimise_unsupported = [
    "--complex-coercion",
    "--enable-higher-kinded-types",
    "--enable-class-level-where-clauses",
    "--enable-global-access-check",
]
max_workers = 48
verbose = False
dump_on_failure = False
batch_size = 500


@dataclass
class TestCase:
    file_path: str
    input: Optional[str]
    expected: str


@dataclass
class Result:
    test_case: TestCase
    output: str
    is_failure: bool


class VerifyPessimisationOptions(Enum):
    no = "no"
    all = "all"
    added = "added"
    removed = "removed"
    full = "full"

    def __str__(self) -> str:
        return self.value


"""
Per-test flags passed to test executable. Expected to be in a file with
same name as test, but with .flags extension.
"""


def compare_errors_by_line_no(
    errors_exp: List[Error], errors_out: List[Error]
) -> Tuple[List[Error], List[Error]]:
    i_out = 0
    i_exp = 0
    len_out = len(errors_out)
    len_exp = len(errors_exp)
    errors_in_out_not_in_exp = []
    errors_in_exp_not_in_out = []
    while i_out < len_out and i_exp < len_exp:
        err_out = errors_out[i_out]
        err_exp = errors_exp[i_exp]
        l_out = err_out.message.position.line
        l_exp = err_exp.message.position.line
        if l_out < l_exp:
            errors_in_out_not_in_exp.append(err_out)
            i_out += 1
        elif l_exp < l_out:
            errors_in_exp_not_in_out.append(err_exp)
            i_exp += 1
        else:
            i_out += 1
            i_exp += 1
    if i_out >= len_out:
        for i in range(i_exp, len_exp):
            errors_in_exp_not_in_out.append(errors_exp[i])
    elif i_exp >= len_exp:
        for i in range(i_out, len_out):
            errors_in_out_not_in_exp.append(errors_out[i])
    return (errors_in_exp_not_in_out, errors_in_out_not_in_exp)


def compare_output_files_error_lines_only(
    file_out: str, file_exp: str
) -> Tuple[bool, str]:
    out = ""
    failed = False
    try:
        errors_out = parse_errors(file_out)
        errors_exp = parse_errors(file_exp)
        (
            errors_in_exp_not_in_out,
            errors_in_out_not_in_exp,
        ) = compare_errors_by_line_no(errors_out=errors_out, errors_exp=errors_exp)

        failed = bool(errors_in_exp_not_in_out) or bool(errors_in_out_not_in_exp)
        if errors_in_exp_not_in_out:
            out += f"""\033[93mExpected errors which were not produced:\033[0m
{sprint_errors(errors_in_exp_not_in_out)}
"""
        if errors_in_out_not_in_exp:
            out += f"""\033[93mProduced errors which were not expected:\033[0m
{sprint_errors(errors_in_out_not_in_exp)}
"""
    except IOError as e:
        out = f"Warning: {e}"
    return (failed, out)


def check_output_error_lines_only(
    test: str, out_ext: str = DEFAULT_OUT_EXT, exp_ext: str = DEFAULT_EXP_EXT
) -> Tuple[bool, str]:
    file_out = test + out_ext
    file_exp = test + exp_ext
    return compare_output_files_error_lines_only(file_out=file_out, file_exp=file_exp)


def get_test_flags(path: str) -> List[str]:
    prefix, _ext = os.path.splitext(path)
    path = prefix + ".flags"

    if not os.path.isfile(path):
        return []
    with open(path) as file:
        return shlex.split(file.read().strip())


def check_output(
    case: TestCase,
    out_extension: str,
    default_expect_regex: Optional[str],
    ignore_error_text: bool,
    only_compare_error_lines: bool,
    verify_pessimisation: VerifyPessimisationOptions,
    check_expected_included_in_actual: bool,
) -> Result:
    if only_compare_error_lines:
        (failed, out) = check_output_error_lines_only(case.file_path)
        return Result(test_case=case, output=out, is_failure=failed)
    else:
        out_path = (
            case.file_path + out_extension
            if verify_pessimisation == VerifyPessimisationOptions.no
            or verify_pessimisation == VerifyPessimisationOptions.full
            or out_extension == ".pess.out"
            else case.file_path + ".pess" + out_extension
        )
        try:
            with open(out_path, "r") as f:
                output: str = f.read()
        except FileNotFoundError:
            out_path = os.path.realpath(out_path)
            output = "Output file " + out_path + " was not found!"
        return check_result(
            case,
            default_expect_regex,
            ignore_error_text,
            verify_pessimisation,
            check_expected_included_in_actual=check_expected_included_in_actual,
            out=output,
        )


def debug_cmd(cwd: str, cmd: List[str]) -> None:
    if verbose:
        print("From directory", os.path.realpath(cwd))
        print("Executing", " ".join(cmd))
        print()


def run_batch_tests(
    test_cases: List[TestCase],
    program: str,
    default_expect_regex: Optional[str],
    ignore_error_text: bool,
    no_stderr: bool,
    force_color: bool,
    mode_flag: List[str],
    get_flags: Callable[[str], List[str]],
    out_extension: str,
    verify_pessimisation: VerifyPessimisationOptions,
    check_expected_included_in_actual: bool,
    only_compare_error_lines: bool = False,
) -> List[Result]:
    """
    Run the program with batches of files and return a list of results.
    """
    # Each directory needs to be in a separate batch because flags are different
    # for each directory.
    # Compile a list of directories to test cases, and then
    dirs_to_files: Dict[str, List[TestCase]] = {}
    for case in test_cases:
        test_dir = os.path.dirname(case.file_path)
        dirs_to_files.setdefault(test_dir, []).append(case)

    # run a list of test cases.
    # The contract here is that the program will write to
    # filename.out_extension for each file, and we read that
    # for the output.
    # Remark: if verify_pessimisation is set, then the result
    # is in filename.pess.out_extension.
    def run(test_cases: List[TestCase]) -> List[Result]:
        if not test_cases:
            raise AssertionError()
        first_test = test_cases[0]
        test_dir = os.path.dirname(first_test.file_path)
        flags = get_flags(test_dir)
        test_flags = get_test_flags(first_test.file_path)
        if verify_pessimisation != VerifyPessimisationOptions.no:
            path = os.path.join(test_dir, "NO_PESS")
            if os.path.isfile(path):
                return []
            for flag in flags_pessimise_unsupported:
                if flag in flags:
                    return []
        cmd = [program]
        cmd += mode_flag
        cmd += ["--batch-files", "--out-extension", out_extension]
        cmd += flags + test_flags
        cmd += [os.path.basename(case.file_path) for case in test_cases]
        debug_cmd(test_dir, cmd)
        env = os.environ.copy()
        env["FORCE_ERROR_COLOR"] = "true" if force_color else "false"
        try:
            return_code = subprocess.call(
                cmd,
                stderr=None if no_stderr else subprocess.STDOUT,
                cwd=test_dir,
                universal_newlines=True,
                env=env,
            )
        except subprocess.CalledProcessError:
            # we don't care about nonzero exit codes... for instance, type
            # errors cause hh_single_type_check to produce them
            return_code = None
        if return_code == -11:
            print(
                "Segmentation fault while running the following command "
                + "from directory "
                + os.path.realpath(test_dir)
            )
            print(" ".join(cmd))
            print()
        results = []
        for case in test_cases:
            result = check_output(
                case,
                out_extension=out_extension,
                default_expect_regex=default_expect_regex,
                ignore_error_text=ignore_error_text,
                only_compare_error_lines=only_compare_error_lines,
                verify_pessimisation=verify_pessimisation,
                check_expected_included_in_actual=check_expected_included_in_actual,
            )
            results.append(result)
        return results

    # Create a list of batched cases.
    all_batched_cases: List[List[TestCase]] = []

    # For each directory, we split all the test cases
    # into chunks of batch_size. Then each of these lists
    # is a separate job for each thread in the threadpool.
    for cases in dirs_to_files.values():
        batched_cases: List[List[TestCase]] = [
            cases[i : i + batch_size] for i in range(0, len(cases), batch_size)
        ]
        all_batched_cases += batched_cases

    executor = ThreadPoolExecutor(max_workers=max_workers)
    futures = [executor.submit(run, test_batch) for test_batch in all_batched_cases]

    results = [future.result() for future in futures]
    # Flatten the list
    return [item for sublist in results for item in sublist]


def run_test_program(
    test_cases: List[TestCase],
    program: str,
    default_expect_regex: Optional[str],
    ignore_error_text: bool,
    no_stderr: bool,
    force_color: bool,
    mode_flag: List[str],
    get_flags: Callable[[str], List[str]],
    verify_pessimisation: VerifyPessimisationOptions,
    check_expected_included_in_actual: bool,
    timeout: Optional[float] = None,
) -> List[Result]:

    """
    Run the program and return a list of results.
    """

    def run(test_case: TestCase) -> Result:
        test_dir, test_name = os.path.split(test_case.file_path)
        flags = get_flags(test_dir)
        test_flags = get_test_flags(test_case.file_path)
        cmd = [program]
        cmd += mode_flag
        if test_case.input is None:
            cmd.append(test_name)
        cmd += flags + test_flags
        debug_cmd(test_dir, cmd)
        env = os.environ.copy()
        env["FORCE_ERROR_COLOR"] = "true" if force_color else "false"
        try:
            output = subprocess.check_output(
                cmd,
                stderr=None if no_stderr else subprocess.STDOUT,
                cwd=test_dir,
                universal_newlines=True,
                input=test_case.input,
                timeout=timeout,
                errors="replace",
                env=env,
            )
        except subprocess.TimeoutExpired as e:
            output = "Timed out. " + str(e.output)
        except subprocess.CalledProcessError as e:
            if e.returncode == 126:
                # unable to execute, typically due in buck2 to "Text file is busy" because someone still has a write handle open to it.
                # https://www.internalfb.com/intern/qa/312685/text-file-is-busy---test-is-run-before-fclose-on-e
                # Ugly workaround for now: just skip it
                # This should be removed once T107518211 is closed.
                output = "Timed out. " + str(e.output)
            else:
                # we don't care about nonzero exit codes... for instance, type
                # errors cause hh_single_type_check to produce them
                output = str(e.output)
        return check_result(
            test_case,
            default_expect_regex,
            ignore_error_text,
            verify_pessimisation,
            check_expected_included_in_actual=check_expected_included_in_actual,
            out=output,
        )

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
        drop_line = x.lstrip().startswith("Called") or x.lstrip().startswith("Raised")
        if drop_line:
            pass
        else:
            out.append(x)
    return "\n".join(out)


def filter_temp_hhi_path(text: str) -> str:
    """The .hhi files are stored in a temporary directory whose name
    changes every time. Normalise it.

    /tmp/ASjh5RoWbb/builtins_fb.hhi -> /tmp/hhi_dir/builtins_fb.hhi

    """
    return re.sub(
        r"/tmp/[^/]*/([a-zA-Z0-9_]+\.hhi)",
        "/tmp/hhi_dir/\\1",
        text,
    )


def strip_pess_suffix(text: str) -> str:
    return re.sub(
        r"_pess.php",
        ".php",
        text,
    )


def compare_expected(
    expected: str, out: str, verify_pessimisation: VerifyPessimisationOptions
) -> bool:
    if (
        verify_pessimisation == VerifyPessimisationOptions.no
        or verify_pessimisation == VerifyPessimisationOptions.all
        or verify_pessimisation == VerifyPessimisationOptions.full
    ):
        if expected == "No errors" or out == "No errors":
            return expected == out
        else:
            return True
    elif verify_pessimisation == VerifyPessimisationOptions.added:
        return not (expected == "No errors" and out != "No errors")
    elif verify_pessimisation == VerifyPessimisationOptions.removed:
        return not (expected != "No errors" and out == "No errors")
    else:
        raise Exception(
            "Cannot happen: verify_pessimisation option %s" % verify_pessimisation
        )


# Strip leading and trailing whitespace from every line
def strip_lines(text: str) -> str:
    return "\n".join(line.strip() for line in text.splitlines())


def check_result(
    test_case: TestCase,
    default_expect_regex: Optional[str],
    ignore_error_messages: bool,
    verify_pessimisation: VerifyPessimisationOptions,
    check_expected_included_in_actual: bool,
    out: str,
) -> Result:
    if check_expected_included_in_actual:
        return check_included(test_case, out)
    else:
        return check_expected_equal_actual(
            test_case,
            default_expect_regex,
            ignore_error_messages,
            verify_pessimisation,
            out,
        )


def check_expected_equal_actual(
    test_case: TestCase,
    default_expect_regex: Optional[str],
    ignore_error_messages: bool,
    verify_pessimisation: VerifyPessimisationOptions,
    out: str,
) -> Result:
    """
    Check that the output of the test in :out corresponds to the expected
    output, or if a :default_expect_regex is provided,
    check that the output in :out contains the provided regex.
    """
    expected = filter_temp_hhi_path(strip_lines(test_case.expected))
    normalized_out = (
        filter_temp_hhi_path(strip_lines(out))
        if verify_pessimisation == "no"
        else strip_pess_suffix(filter_temp_hhi_path(strip_lines(out)))
    )
    is_ok = (
        expected == normalized_out
        or (
            (
                ignore_error_messages
                or verify_pessimisation != VerifyPessimisationOptions.no
                and verify_pessimisation != VerifyPessimisationOptions.full
            )
            and compare_expected(expected, normalized_out, verify_pessimisation)
        )
        or expected == filter_ocaml_stacktrace(normalized_out)
        or (
            default_expect_regex is not None
            and re.search(default_expect_regex, normalized_out) is not None
            and expected == ""
        )
    )
    return Result(test_case=test_case, output=out, is_failure=not is_ok)


def check_included(test_case: TestCase, output: str) -> Result:
    elts = set(output.splitlines())
    expected_elts = set(test_case.expected.splitlines())
    is_failure = False
    for expected_elt in expected_elts:
        if expected_elt not in elts:
            is_failure = True
            break
    output = ""
    if is_failure:
        for elt in expected_elts.intersection(elts):
            output += elt + "\n"
    return Result(test_case, output, is_failure)


def record_results(results: List[Result], out_ext: str) -> None:
    for result in results:
        outfile = result.test_case.file_path + out_ext
        with open(outfile, "wb") as f:
            f.write(bytes(result.output, "UTF-8"))


def find_in_ancestors_rec(dir: str, path: str) -> str:
    if path == "" or os.path.dirname(path) == path:
        raise Exception("Could not find directory %s in ancestors." % dir)
    if os.path.basename(path) == dir:
        return path
    return find_in_ancestors_rec(dir, os.path.dirname(path))


def find_in_ancestors(dir: str, path: str) -> str:
    try:
        return find_in_ancestors_rec(dir, path)
    except Exception:
        raise Exception("Could not find directory %s in ancestors of %s." % (dir, path))


def get_exp_out_dirs(test_file: str) -> Tuple[str, str]:
    if (
        os.environ.get("HACK_BUILD_ROOT") is not None
        and os.environ.get("HACK_SOURCE_ROOT") is not None
    ):
        exp_dir = os.environ["HACK_SOURCE_ROOT"]
        out_dir = os.environ["HACK_BUILD_ROOT"]
    else:
        fbcode = find_in_ancestors("fbcode", test_file)
        exp_dir = os.path.join(fbcode, "hphp", "hack")
        out_dir = os.path.dirname(find_in_ancestors("test", test_file))
    return exp_dir, out_dir


def report_failures(
    total: int,
    failures: List[Result],
    out_extension: str,
    expect_extension: str,
    fallback_expect_extension: Optional[str],
    verify_pessimisation: VerifyPessimisationOptions = VerifyPessimisationOptions.no,
    no_copy: bool = False,
    only_compare_error_lines: bool = False,
) -> None:
    if only_compare_error_lines:
        for failure in failures:
            print(f"\033[95m{failure.test_case.file_path}\033[0m")
            print(failure.output)
            print()
    elif failures != []:
        record_results(failures, out_extension)
        if dump_on_failure:
            dump_failures(failures)
        fnames = [failure.test_case.file_path for failure in failures]
        print("To review the failures, use the following command: ")

        first_test_file = os.path.realpath(failures[0].test_case.file_path)
        out_dir: str  # for Pyre
        (exp_dir, out_dir) = get_exp_out_dirs(first_test_file)

        # Get a full path to 'review.sh' so this command be run
        # regardless of your current directory.
        review_script = os.path.join(
            os.path.dirname(os.path.realpath(__file__)), "review.sh"
        )
        if not os.path.isfile(review_script):
            review_script = "./hphp/hack/test/review.sh"

        def fname_map_var(f: str) -> str:
            return "hphp/hack/" + os.path.relpath(f, out_dir)

        env_vars = []
        if out_extension != DEFAULT_OUT_EXT:
            env_vars.append("OUT_EXT=%s" % out_extension)
        if expect_extension != DEFAULT_EXP_EXT:
            env_vars.append("EXP_EXT=%s" % expect_extension)
        if fallback_expect_extension is not None:
            env_vars.append("FALLBACK_EXP_EXT=%s " % fallback_expect_extension)
        if verify_pessimisation != VerifyPessimisationOptions.no:
            env_vars.append("VERIFY_PESSIMISATION=true")
        if no_copy:
            env_vars.append("UPDATE=never")

        env_vars.extend(["SOURCE_ROOT=%s" % exp_dir, "OUTPUT_ROOT=%s" % out_dir])

        print(
            "%s %s %s"
            % (
                " ".join(env_vars),
                review_script,
                " ".join(map(fname_map_var, fnames)),
            )
        )

        # If more than 75% of files have changed, we're probably doing
        # a transformation to all the .exp files.
        if len(fnames) >= (0.75 * total):
            print(
                "\nJust want to update all the %s files? Use UPDATE=always with the above command."
                % expect_extension
            )


def dump_failures(failures: List[Result]) -> None:
    for f in failures:
        expected = f.test_case.expected
        actual = f.output
        diff = difflib.ndiff(expected.splitlines(True), actual.splitlines(True))
        print("Details for the failed test %s:" % f.test_case.file_path)
        print("\n>>>>>  Expected output  >>>>>>\n")
        print(expected)
        print("\n=====   Actual output   ======\n")
        print(actual)
        print("\n<<<<< End Actual output <<<<<<<\n")
        print("\n>>>>>       Diff        >>>>>>>\n")
        print("".join(diff))
        print("\n<<<<<     End Diff      <<<<<<<\n")


def get_hh_flags(test_dir: str) -> List[str]:
    path = os.path.join(test_dir, "HH_FLAGS")
    if not os.path.isfile(path):
        if verbose:
            print("No HH_FLAGS file found")
        return []
    with open(path) as f:
        return shlex.split(f.read())


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


def list_test_files(
    root: str, disabled_ext: str, test_ext: str, include_directories: bool
) -> List[str]:
    if os.path.isfile(root):
        if root.endswith(test_ext):
            return [root]
        else:
            return []
    elif os.path.isdir(root):
        result: List[str] = []
        if include_directories and root.endswith(test_ext):
            result.append(root)
        children = os.listdir(root)
        disabled = files_with_ext(children, disabled_ext)
        for child in children:
            if child != "disabled" and child not in disabled:
                result.extend(
                    list_test_files(
                        os.path.join(root, child),
                        disabled_ext,
                        test_ext,
                        include_directories,
                    )
                )
        return result
    elif os.path.islink(root):
        # Some editors create broken symlinks as part of their locking scheme,
        # so ignore those.
        return []
    else:
        raise Exception("Could not find test file or directory at %s" % root)


def get_content_(file_path: str, ext: str) -> str:
    with open(file_path + ext, "r") as fexp:
        return fexp.read()


def get_content(
    file_path: str, ext: str = "", fallback_ext: Optional[str] = None
) -> str:
    try:
        return get_content_(file_path, ext)
    except FileNotFoundError:
        if fallback_ext is not None:
            try:
                return get_content_(file_path, fallback_ext)
            except FileNotFoundError:
                return ""
        else:
            return ""


def run_tests(
    files: List[str],
    expected_extension: str,
    fallback_expect_extension: Optional[str],
    out_extension: str,
    use_stdin: str,
    program: str,
    default_expect_regex: Optional[str],
    batch_mode: str,
    ignore_error_text: bool,
    no_stderr: bool,
    force_color: bool,
    verify_pessimisation: VerifyPessimisationOptions,
    mode_flag: List[str],
    get_flags: Callable[[str], List[str]],
    check_expected_included_in_actual: bool,
    timeout: Optional[float] = None,
    only_compare_error_lines: bool = False,
) -> List[Result]:

    # for each file, create a test case
    test_cases = [
        TestCase(
            file_path=file,
            expected=get_content(file, expected_extension, fallback_expect_extension),
            input=get_content(file) if use_stdin else None,
        )
        for file in files
    ]
    if batch_mode:
        results = run_batch_tests(
            test_cases,
            program,
            default_expect_regex,
            ignore_error_text,
            no_stderr,
            force_color,
            mode_flag,
            get_flags,
            out_extension,
            verify_pessimisation,
            check_expected_included_in_actual,
            only_compare_error_lines,
        )
    else:
        results = run_test_program(
            test_cases,
            program,
            default_expect_regex,
            ignore_error_text,
            no_stderr,
            force_color,
            mode_flag,
            get_flags,
            verify_pessimisation,
            check_expected_included_in_actual=check_expected_included_in_actual,
            timeout=timeout,
        )

    failures = [result for result in results if result.is_failure]

    num_results = len(results)
    if failures == []:
        print(
            "All tests in the suite passed! "
            "The number of tests that ran: %d\n" % num_results
        )
    else:
        print("The number of tests that failed: %d/%d\n" % (len(failures), num_results))
        report_failures(
            num_results,
            failures,
            args.out_extension,
            args.expect_extension,
            args.fallback_expect_extension,
            verify_pessimisation=verify_pessimisation,
            only_compare_error_lines=only_compare_error_lines,
        )
        sys.exit(1)  # this exit code fails the suite and lets Buck know

    return results


def run_idempotence_tests(
    results: List[Result],
    expected_extension: str,
    out_extension: str,
    program: str,
    default_expect_regex: Optional[str],
    mode_flag: List[str],
    get_flags: Callable[[str], List[str]],
    check_expected_included_in_actual: bool,
) -> None:
    idempotence_test_cases = [
        TestCase(
            file_path=result.test_case.file_path,
            expected=result.test_case.expected,
            input=result.output,
        )
        for result in results
    ]

    idempotence_results = run_test_program(
        idempotence_test_cases,
        program,
        default_expect_regex,
        False,
        False,
        False,
        mode_flag,
        get_flags,
        VerifyPessimisationOptions.no,
        check_expected_included_in_actual=check_expected_included_in_actual,
    )

    num_idempotence_results = len(idempotence_results)

    idempotence_failures = [
        result for result in idempotence_results if result.is_failure
    ]

    if idempotence_failures == []:
        print(
            "All idempotence tests in the suite passed! The number of "
            "idempotence tests that ran: %d\n" % num_idempotence_results
        )
    else:
        print(
            "The number of idempotence tests that failed: %d/%d\n"
            % (len(idempotence_failures), num_idempotence_results)
        )
        report_failures(
            num_idempotence_results,
            idempotence_failures,
            out_extension + out_extension,  # e.g., *.out.out
            expected_extension,
            None,
            no_copy=True,
        )
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


def get_flags_dummy(args_flags: List[str]) -> Callable[[str], List[str]]:
    def get_flags(_: str) -> List[str]:
        return args_flags

    return get_flags


def main() -> None:
    global max_workers, dump_on_failure, verbose, args
    parser = argparse.ArgumentParser()
    parser.add_argument("test_path", help="A file or a directory. ")
    parser.add_argument("--program", type=os.path.abspath)
    parser.add_argument("--out-extension", type=str, default=DEFAULT_OUT_EXT)
    parser.add_argument("--expect-extension", type=str, default=DEFAULT_EXP_EXT)
    parser.add_argument("--fallback-expect-extension", type=str)
    parser.add_argument("--default-expect-regex", type=str)
    parser.add_argument("--in-extension", type=str, default=".php")
    parser.add_argument("--include-directories", action="store_true")
    parser.add_argument("--disabled-extension", type=str, default=".no_typecheck")
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument(
        "--idempotence",
        action="store_true",
        help="Verify that the output passed to the program "
        "as input results in the same output.",
    )
    parser.add_argument("--max-workers", type=int, default="48")
    parser.add_argument(
        "--diff",
        action="store_true",
        help="On test failure, show the content of " "the files and a diff",
    )
    parser.add_argument("--mode-flag", type=str)
    parser.add_argument("--flags", nargs=argparse.REMAINDER)
    parser.add_argument(
        "--stdin", action="store_true", help="Pass test input file via stdin"
    )
    parser.add_argument(
        "--no-stderr",
        action="store_true",
        help="Do not include stderr output in the output file",
    )
    parser.add_argument(
        "--batch", action="store_true", help="Run tests in batches to the test program"
    )
    parser.add_argument(
        "--ignore-error-text",
        action="store_true",
        help="Do not compare error text when verifying output",
    )
    parser.add_argument(
        "--only-compare-error-lines",
        action="store_true",
        help="Does not care about exact expected error message, "
        "but only compare the error line numbers.",
    )
    parser.add_argument(
        "--check-expected-included-in-actual",
        action="store_true",
        help="Check that the set of lines in the expected file is included in the set"
        " of lines in the output",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        help="Timeout in seconds for each test, in non-batch mode.",
    )
    parser.add_argument(
        "--force-color",
        action="store_true",
        help="Set the FORCE_ERROR_COLOR environment variable, "
        "which causes the test output to retain terminal escape codes.",
    )
    parser.add_argument(
        "--no-hh-flags", action="store_true", help="Do not read HH_FLAGS files"
    )
    parser.add_argument(
        "--verify-pessimisation",
        type=VerifyPessimisationOptions,
        choices=list(VerifyPessimisationOptions),
        default=VerifyPessimisationOptions.no,
        help="Experimental test suite for hh_pessimisation",
    )
    parser.epilog = (
        "%s looks for a file named HH_FLAGS in the same directory"
        " as the test files it is executing. If found, the "
        "contents will be passed as arguments to "
        "<program> in addition to any arguments "
        "specified by --flags" % parser.prog
    )
    args = parser.parse_args()

    max_workers = args.max_workers
    verbose = args.verbose

    dump_on_failure = args.diff
    if os.getenv("SANDCASTLE") is not None:
        dump_on_failure = True

    if not os.path.isfile(args.program):
        raise Exception("Could not find program at %s" % args.program)

    # 'args.test_path' is a path relative to the current working
    # directory. buck1 runs this test from fbsource/fbocde, buck2 runs
    # it from fbsource.
    if os.path.basename(os.getcwd()) != "fbsource":

        # If running under buck1 then we are in fbcode, if running
        # under dune then some ancestor directory of fbcode. These two
        # cases are handled by the logic of this script and
        # 'review.sh' and there are no adjustments to make.
        pass
    else:

        # The buck2 case has us running in fbsource. This puts us in
        # fbcode.
        os.chdir("fbcode")

    files: List[str] = list_test_files(
        args.test_path,
        args.disabled_extension,
        args.in_extension,
        args.include_directories,
    )

    if len(files) == 0:
        raise Exception("Could not find any files to test in " + args.test_path)

    mode_flag: List[str] = [] if args.mode_flag is None else [args.mode_flag]
    get_flags: Callable[[str], List[str]] = (
        get_flags_dummy(args.flags) if args.no_hh_flags else get_flags_cache(args.flags)
    )

    results: List[Result] = run_tests(
        files,
        args.expect_extension,
        args.fallback_expect_extension,
        args.out_extension,
        args.stdin,
        args.program,
        args.default_expect_regex,
        args.batch,
        args.ignore_error_text,
        args.no_stderr,
        args.force_color,
        args.verify_pessimisation,
        mode_flag,
        get_flags,
        check_expected_included_in_actual=args.check_expected_included_in_actual,
        timeout=args.timeout,
        only_compare_error_lines=args.only_compare_error_lines,
    )

    # Doesn't make sense to check failures for idempotence
    successes: List[Result] = [result for result in results if not result.is_failure]

    if args.idempotence and successes:
        run_idempotence_tests(
            successes,
            args.expect_extension,
            args.out_extension,
            args.program,
            args.default_expect_regex,
            mode_flag,
            get_flags,
            check_expected_included_in_actual=args.check_expected_included_in_actual,
        )


args: argparse.Namespace


if __name__ == "__main__":
    main()
