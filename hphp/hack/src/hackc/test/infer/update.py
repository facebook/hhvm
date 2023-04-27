import argparse
import json
import os
import subprocess
import sys

### Script to rebuild test expect data.  Tests are annotated with 'TEST-CHECK*'
### to describe how to update the CHECK data.
###
### Annotations look like:
### // TEST-CHECK*: phrase
###
### The phrase is matched based on the type of check and then an action is
### performed to update the CHECK statements.
###
### TEST-CHECK-1: Look for the phrase at the start of a line and match a single
###               CHECK with that line.
###
### TEST-CHECK-1*: Look for the phrase anywhere in a line and match a single
###                CHECK with that line.
###
### TEST-CHECK-BAL: Look for the phrase at the start of a line and continue the
###                 CHECK until the parentheses, brackets, and braces are
###                 balanced.
###
### TEST-CHECK-IGN: Ignore the next sequence of CHECK lines.  They must be
###                 manually updated.
###

ROOT = "."
HACKC = "hackc"

# From the current "// TEST-CHECK..." remove "// CHECK..." lines.
def strip_existing_check(input, idx):
    old = []
    while idx < len(input) and (
        input[idx].strip().startswith("// CHECK:") or input[idx].startswith("// CHECK-")
    ):
        old.append(input[idx])
        del input[idx]
    return old


# Find input that matches the expected TEST-CHECK
def fetch_check(filename, line, check, test, old):
    indent = test.find("//")
    if indent == -1:
        indent = 0
    indent = " " * indent
    test = test.strip()

    expect, what = match_and_strip(test, "TEST-CHECK-IGN")
    if what:
        # A section of "CHECK" which is left alone.
        return old, None

    expect, what = match_and_strip(test, "TEST-CHECK-1", "TEST-CHECK-1*")
    if what:
        # A single line. If the asterisk is present then matches anywhere on the line.
        any = what == "TEST-CHECK-1*"
        where = find_line_with(filename, line, check, expect, any)

        output = build_output(indent, check, where, 1)
        return output, where

    expect, what = match_and_strip(test, "TEST-CHECK-BAL")
    if what:
        # Balanced braces
        where = find_line_with(filename, line, check, expect, 0)

        balance = count_balance(check[where])
        lines = 1
        while balance != 0:
            if where + lines >= len(check):
                bail(
                    filename,
                    line,
                    f"Unbalanced tokens starting at '{expect}' on line '{where}'",
                )
            balance += count_balance(check[where + lines])
            lines += 1

        output = build_output(indent, check, where, lines)
        return output, where

    bail(filename, line, f"Unknown test type: {test}")


# Look for any of the checks in the test line. Returns `expect, what` or `None,
# None`.
def match_and_strip(test, *checks):
    for check in checks:
        if test.startswith(f"// {check}:"):
            expect = unquote(test[len(check) + 4 :].strip())
            return expect, check
    return None, None


# Handle indenting and prepending `CHECK`
def build_output(indent, check, where, lines):
    output = []
    for line in check[where : where + lines]:
        if line.startswith("// .line ") or line.startswith("// .file"):
            continue
        output.append(f"{indent}// CHECK: {line}")
    return output


def unquote(input):
    if input.startswith('"') and input.endswith('"'):
        return input[1:-1]
    else:
        return input


def count_balance(input):
    count = 0
    for c in input:
        if c == "(" or c == "{" or c == "[":
            count += 1
        elif c == ")" or c == "}" or c == "]":
            count -= 1
    return count


def is_identifier(c):
    return c.isidentifier() or c == "$" or c == ":"


def is_match(inp, expect, any_match):
    # If inp is smaller then they can't match.
    if len(inp) < len(expect):
        return False

    # If they're exactly equal they must match.
    if inp == expect:
        return True

    if any_match:
        # For any_match mode then just look for any match.
        return expect in inp

    # If inp doesn't start with expect then they can't match.
    if not inp.startswith(expect):
        return False

    # If expect ends with an identifier char then we expect that the next char
    # in inp must NOT be an identifier char.
    if is_identifier(expect[-1]) and is_identifier(inp[len(expect)]):
        return False

    return True


def find_line_with(filename, line, check, expect, any_match):
    idx = 0
    while idx < len(check):
        inp = check[idx]
        if is_match(inp, expect, any_match):
            return idx
        idx += 1
    bail(filename, line, f"Expected string '{expect}' not found")


def update_test(filename, input, idx, check):
    old = strip_existing_check(input, idx + 1)
    check, start = fetch_check(filename, idx, check, input[idx], old)
    input[idx + 1 : idx + 1] = check

    return 1 + len(check), start


def update_file(filename):
    stdout = subprocess.check_output((HACKC, "compile-infer", filename)).decode(
        "utf-8", "ignore"
    )
    check = stdout.split("\n")

    with open(filename, "r") as f:
        file = f.read().split("\n")

    last_seen = 0
    idx = 0
    while idx < len(file):
        inp = file[idx].strip()
        if inp.startswith("// TEST-CHECK:") or inp.startswith("// TEST-CHECK-"):
            count, start = update_test(filename, file, idx, check)
        elif inp.startswith("// CHECK:") or inp.startswith("// CHECK-"):
            warn(filename, idx, "Naked CHECK found")
        else:
            count, start = 1, last_seen

        if start is not None:
            if start < last_seen:
                bail(
                    filename,
                    idx,
                    f"test was found but is out of order (found at {start} but already processed {last_seen})",
                )
            last_seen = start

        idx += count

    with open(filename, "w") as f:
        f.write("\n".join(file))


def run(*args):
    return subprocess.check_output(*args).decode("utf-8", "ignore").strip()


# Query buck for the location of hackc.
def compute_hackc(use_cargo):
    if use_cargo:
        root = run("hg root".split())
        stdout = run(
            (
                root + "/fbcode/hphp/hack/scripts/facebook/cargo.sh",
                "build",
                "-p",
                "hackc",
                "--message-format=json",
            )
        )
        for msg in stdout.split("\n"):
            # print(repr(msg))
            output = json.loads(msg)
            if output["reason"] == "compiler-artifact" and output[
                "package_id"
            ].startswith("hackc "):
                return output["executable"]

        raise Exception("hackc not found")

    else:
        stdout = run(
            (
                "buck2",
                "build",
                "--reuse-current-config",
                "//hphp/hack/src/hackc:hackc",
                "--show-full-output",
            )
        )
        _, hackc = stdout.split(" ", 1)
        return hackc.strip()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("files", metavar="FILE", type=str, nargs="*")
    parser.add_argument("--cargo", action="store_true")
    args = parser.parse_args()

    global HACKC, ROOT
    HACKC = compute_hackc(args.cargo)
    ROOT = os.path.dirname(__file__)

    if not args.files:
        args.files = []
        for root, _, files in os.walk(ROOT):
            for file in files:
                if file.endswith(".hack") and not file.endswith("_tc.hack"):
                    args.files.append(os.path.join(root, file))

    for file in sorted(args.files):
        print(f"Processing {file}")
        update_file(file)


def warn(filename, line, why):
    print(f"{filename}:{line + 1} (warning): {why}", file=sys.stderr)


def bail(filename, line, why):
    print(f"{filename}:{line + 1}: {why}", file=sys.stderr)
    sys.exit(1)


main()
