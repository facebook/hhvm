# pyre-strict

import concurrent.futures
import json
import os
import subprocess
import tempfile
from concurrent.futures import as_completed
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

import regex as re


@dataclass
class Failure:
    path: str
    contents: str
    stdout: bytes
    stderr: bytes
    commands: List[List[str]] = field(default_factory=list)
    returncode: Optional[int] = None


def diagnostic_codes(output: str) -> Optional[List[str]]:
    codes = []
    legacy_header = False
    for line in output.splitlines():
        if re.match(r"^(?:ERROR|WARN|WARNING): File\b", line):
            if legacy_header:
                return None
            legacy_header = True
        elif re.match(r"^(?:error|warning|warn):", line, re.IGNORECASE):
            match = re.match(
                r"^(?:error|warning|warn): ([A-Za-z][A-Za-z_]*\[\d+\])(?:\s|$)",
                line,
                re.IGNORECASE,
            )
            if match is None or legacy_header:
                return None
            codes.append(match.group(1))
        elif legacy_header:
            match = re.search(r"\(([A-Za-z][A-Za-z_]*\[\d+\])\)$", line)
            if match is not None:
                codes.append(match.group(1))
                legacy_header = False
    return None if legacy_header else codes


def matches_expected_output(
    pattern: str, result: subprocess.CompletedProcess[bytes]
) -> bool:
    output = (
        result.stdout.decode(errors="replace")
        + "\n"
        + result.stderr.decode(errors="replace")
    )
    codes = diagnostic_codes(output)
    if result.returncode not in (0, 2) or codes is None:
        return False
    if codes:
        if re.search(r"(?m)^No errors\s*$", output) is not None and any(
            not code.startswith("Warn[") for code in codes
        ):
            return False
        if result.returncode == 0 and any(
            not code.startswith("Warn[") for code in codes
        ):
            return False
        return all(re.fullmatch(pattern, code) is not None for code in codes)
    return (
        result.returncode == 0
        and re.search(r"(?m)^No errors\s*$", output) is not None
        and re.fullmatch(pattern, "No errors") is not None
    )


def record_failure(failure: Failure) -> None:
    with open(failure.path + ".failure.json", "w") as output:
        json.dump(
            {
                "program": failure.path,
                "commands": failure.commands,
                "returncode": failure.returncode,
                "stdout": failure.stdout.decode(errors="replace"),
                "stderr": failure.stderr.decode(errors="replace"),
            },
            output,
            indent=2,
        )
        output.write("\n")


def bad_exit(res: Failure) -> None:
    print("***")
    print(f"* Error in {res.path}:")
    print("***")
    print("* File contents:")
    print("***")
    print(res.contents)
    print("***")
    print("* hh_single_type_check STDOUT:")
    print("***")
    print(res.stdout.decode(errors="replace"))
    print("***")
    print("* hh_single_type_check STDERR:")
    print("***")
    print(res.stderr.decode(errors="replace"))


def milner_and_type_check(
    milner_exe: str,
    hhstc_exe: str,
    out_dir: str,
    template_file: str,
    hhstc_pattern: str,
    seed: int,
    skip_hhstc: bool,
    timeout: float = 180,
) -> Optional[Failure]:
    basename = os.path.basename(template_file)
    temp_file = os.path.join(out_dir, f"{basename}.{seed}.out")
    commands = [[milner_exe, os.path.abspath(template_file), "--seed", str(seed)]]

    def failure(stdout: bytes, stderr: bytes, returncode: Optional[int]) -> Failure:
        try:
            with open(temp_file, "r", errors="replace") as generated:
                contents = generated.read()
        except OSError:
            contents = ""
        return Failure(temp_file, contents, stdout, stderr, commands, returncode)

    # Run the generator on the template file and the number
    try:
        with open(temp_file, "w") as out:
            result = subprocess.run(
                commands[-1], stdout=out, stderr=subprocess.PIPE, timeout=timeout
            )
    except subprocess.TimeoutExpired as error:
        return failure(
            error.stdout or b"", (error.stderr or b"") + b"\nGenerator timed out", None
        )
    except OSError as error:
        return failure(b"", str(error).encode(), None)

    if result.returncode != 0:
        return failure(b"", result.stderr, result.returncode)

    # Run the verifier on the temporary file
    if not skip_hhstc:
        commands.append([hhstc_exe, temp_file])
        try:
            result = subprocess.run(commands[-1], capture_output=True, timeout=timeout)
        except subprocess.TimeoutExpired as error:
            return failure(
                error.stdout or b"",
                (error.stderr or b"") + b"\nTypechecker timed out",
                None,
            )
        except OSError as error:
            return failure(b"", str(error).encode(), None)

        if not matches_expected_output(hhstc_pattern, result):
            return failure(result.stdout, result.stderr, result.returncode)

    return None


def verify_well_typed(
    template_file: str,
    hhstc_pattern: str,
    out_dir: str,
    seed_range: Tuple[int, int],
    skip_hhstc: bool,
    milner_exe: str,
    hhstc_exe: str,
    timeout: float = 180,
) -> int:
    # In parallel generate programs with seeds
    # (seed_range[0]...seed_range[1]) with milner and verify that they are
    # well-typed with hh_single_type_check
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for seed in range(seed_range[0], seed_range[1]):
            futures.append(
                executor.submit(
                    milner_and_type_check,
                    milner_exe,
                    hhstc_exe,
                    out_dir,
                    template_file,
                    hhstc_pattern,
                    seed,
                    skip_hhstc,
                    timeout,
                )
            )

        # If there were any errors, display the first failure and fail
        exit_code = 0
        smallest_res = None
        for future in as_completed(futures):
            # pyrefly: ignore [bad-assignment]
            res: Failure = future.result()
            if res is not None:
                record_failure(res)
                exit_code = 1
                if smallest_res is None or len(res.contents) < len(
                    smallest_res.contents
                ):
                    smallest_res = res
                bad_exit(res)

    if smallest_res is not None:
        print("*** Failure with the smallest program: ***")
        bad_exit(smallest_res)

    return exit_code


def main() -> None:
    import argparse

    parser = argparse.ArgumentParser(description="Verify well-typed templates")
    parser.add_argument(
        "--template", required=True, help="Directory to search for templates"
    )
    parser.add_argument(
        "--hhstc-pattern",
        required=True,
        help="Pattern to look for in hh_single_type_check output",
    )
    parser.add_argument(
        "--seed-range",
        required=True,
        nargs=2,
        help="Number of templates",
    )
    parser.add_argument("--milner-exe", required=True, help="Path to milner executable")
    parser.add_argument(
        "--hhstc-exe", required=True, help="Path to hh_single_type_check executable"
    )
    parser.add_argument(
        "--skip-hhstc",
        action="store_true",
        help="Skip hh_single_type_check run on generated programs",
    )
    parser.add_argument(
        "--output-dir", help="Keep generated programs and results in a new directory"
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=180,
        help="Maximum seconds for each generator or typechecker process",
    )

    args: argparse.Namespace = parser.parse_args()
    if args.timeout <= 0:
        parser.error("--timeout must be positive")

    seed_range = (int(args.seed_range[0]), int(args.seed_range[1]))

    if args.output_dir is None:
        temporary_out = tempfile.TemporaryDirectory()
        out_dir = temporary_out.name
    else:
        temporary_out = None
        out_dir = os.path.abspath(args.output_dir)
        os.makedirs(out_dir)
    try:
        exit_code = verify_well_typed(
            args.template,
            args.hhstc_pattern,
            out_dir,
            seed_range,
            args.skip_hhstc,
            args.milner_exe,
            args.hhstc_exe,
            args.timeout,
        )
        with open(os.path.join(out_dir, "summary.json"), "w") as output:
            json.dump(
                {"exit_code": exit_code, "arguments": vars(args)}, output, indent=2
            )
            output.write("\n")
    finally:
        if temporary_out is not None:
            temporary_out.cleanup()

    exit(exit_code)


if __name__ == "__main__":
    main()
