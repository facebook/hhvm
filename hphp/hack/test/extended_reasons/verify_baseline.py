#!/usr/bin/env python3
# pyre-strict

import argparse
import difflib
import os
import subprocess
import sys
from dataclasses import dataclass
from typing import List, Optional, Tuple


@dataclass
class Failure:
    file: str
    expected: str
    actual: str


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


def run_command(
    program: str, flags: List[str], files: List[str], is_control: bool
) -> None:
    out_extension = ".control.out" if is_control else ".test.out"
    command = (
        [program] + flags + ["--batch-files", "--out-extension", out_extension] + files
    )
    subprocess.run(
        command,
        universal_newlines=True,
    )


def generate_diff(output1: str, output2: str) -> Optional[str]:
    """Generate a diff between two command outputs."""
    diff = difflib.unified_diff(
        output1.splitlines(),
        output2.splitlines(),
        fromfile="control",
        tofile="test",
    )

    diff_str = "\n".join(diff)
    if diff_str == "":
        return None
    else:
        return diff_str


def process_files(files: List[str], program: str, flags: List[str]) -> List[Failure]:
    run_command(program, [], files, True)
    run_command(program, flags, files, False)
    results: List[Failure] = []
    for file in files:
        # Open the control and test output
        with open(f"{file}.control.out", "r") as f1:
            output1 = f1.read()
        with open(f"{file}.test.out", "r") as f1:
            output2 = f1.read()

        if output1 is not None and output2 is not None:
            # Diff the outputs; this shoud be None if they are the same
            actual_diff = generate_diff(output1, output2)

            try:
                with open(f"{file}.diff.exp", "r") as f:
                    expected_diff = f.read()
            except Exception:
                expected_diff = ""

            if actual_diff is not None:
                # We have a difference!
                # Check if we expected this; if the file doesn't exist we
                # implicitly expected no difference

                # If the expected and actual differences are not the same we
                # need to record a failure
                if actual_diff != expected_diff:
                    results.append(
                        Failure(file=file, expected=expected_diff, actual=actual_diff)
                    )
                    with open(f"{file}.diff.out", "w") as f:
                        f.write(actual_diff)
            else:
                # There is no difference; make sure we either have no expected
                # difference or that the expected difference is empty
                if expected_diff != "":
                    results.append(
                        Failure(file=file, expected=expected_diff, actual="")
                    )
                    with open(f"{file}.diff.out", "w") as f:
                        f.write("")

        # Clean up the test artifacts
        os.remove(f"{file}.control.out")
        os.remove(f"{file}.test.out")
    return results


def get_files_from_path(test_path: str, in_extension: str) -> List[str]:
    """Get a list of files from the given path. If it's a directory, list all files in it."""
    if os.path.isdir(test_path):
        return [
            os.path.join(test_path, f)
            for f in os.listdir(test_path)
            if os.path.isfile(os.path.join(test_path, f)) and f.endswith(in_extension)
        ]
    elif os.path.isfile(test_path):
        if test_path.endswith(in_extension):
            return [test_path]
        else:
            return []
    else:
        raise ValueError(
            f"Provided path '{test_path}' is neither a file nor a directory"
        )


def dump_failures(failures: List[Failure]) -> None:
    for f in failures:
        expected = f.expected
        actual = f.actual
        print("Details for the failed test %s:" % f.file)
        print("\n>>>>>  Expected output  >>>>>>\n")
        print(expected)
        print("\n=====   Actual output   ======\n")
        print(actual)
        print("\n<<<<< End Actual output <<<<<<<\n")


def report_failures(failures: List[Failure], dump_on_failure: bool) -> None:
    if dump_on_failure:
        dump_failures(failures)
    fnames = [failure.file for failure in failures]
    print("To review the failures, use the following command: ")

    first_test_file = os.path.realpath(failures[0].file)
    out_dir: str  # for Pyre
    (exp_dir, out_dir) = get_exp_out_dirs(first_test_file)

    review_script = "~/fbsource/fbcode/hphp/hack/test/review.sh"

    def fname_map_var(f: str) -> str:
        return "hphp/hack/" + os.path.relpath(f, out_dir)

    env_vars = []
    env_vars.append("OUT_EXT=.diff.out")
    env_vars.append("EXP_EXT=.diff.exp")
    env_vars.extend(["SOURCE_ROOT=%s" % exp_dir, "OUTPUT_ROOT=%s" % out_dir])
    print(
        "%s %s %s"
        % (
            " ".join(env_vars),
            review_script,
            " ".join(map(fname_map_var, fnames)),
        )
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Process a file or all files in a directory with a program with and without specified flags, generate diffs, and save non-empty outputs."
    )
    parser.add_argument("test_path", help="Path to a file or directory to process.")
    parser.add_argument("--in-extension", type=str, default=".php")
    parser.add_argument("--include-directories", action="store_true")
    parser.add_argument(
        "--diff",
        action="store_true",
        help="On test failure, show the content of the files and a diff",
    )
    parser.add_argument(
        "--program", type=os.path.abspath, help="The program to execute."
    )
    parser.add_argument(
        "--flags",
        nargs=argparse.REMAINDER,
        help="The set of flags to use with the program.",
    )

    args = parser.parse_args()

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

    files: List[str] = get_files_from_path(args.test_path, args.in_extension)
    failures = process_files(files, args.program, args.flags)
    if failures == []:
        print("All tests in the suite passed!")
    else:
        report_failures(failures, dump_on_failure)
        sys.exit(1)


main()
