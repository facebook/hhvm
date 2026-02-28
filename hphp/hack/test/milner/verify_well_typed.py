# pyre-strict

import concurrent.futures
import os
import subprocess
import tempfile
from concurrent.futures import as_completed
from dataclasses import dataclass
from typing import Optional, Tuple

import regex as re


@dataclass
class Failure:
    path: str
    contents: str
    stdout: bytes
    stderr: bytes


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
    print(res.stdout.decode() if res.stdout is not None else "N/A")
    print("***")
    print("* hh_single_type_check STDERR:")
    print("***")
    print(res.stderr.decode() if res.stderr is not None else "N/A")


def milner_and_type_check(
    milner_exe: str,
    hhstc_exe: str,
    out_dir: str,
    template_file: str,
    hhstc_pattern: str,
    seed: int,
    skip_hhstc: bool,
) -> Optional[Failure]:
    basename = os.path.basename(template_file)
    temp_file = os.path.join(out_dir, f"{basename}.{seed}.out")

    # Run the generator on the template file and the number
    with open(temp_file, "w") as out:
        result = subprocess.run(
            [milner_exe, os.path.abspath(template_file), "--seed", str(seed)],
            stdout=out,
            timeout=120,
        )

    if result.returncode != 0:
        with open(temp_file, "r") as out:
            contents = out.read()
        return Failure(
            temp_file,
            contents,
            result.stdout,
            result.stderr,
        )

    # Run the verifier on the temporary file
    if not skip_hhstc:
        result = subprocess.run(
            [hhstc_exe, temp_file],
            capture_output=True,
            timeout=120,
        )

        # Check if the verifier found any errors
        stdout = result.stdout.decode()
        stderr = result.stderr.decode()
        if (re.search(hhstc_pattern, stdout) is None) and (
            re.search(hhstc_pattern, stderr) is None
        ):
            with open(temp_file, "r") as out:
                contents = out.read()
            return Failure(
                temp_file,
                contents,
                result.stdout,
                result.stderr,
            )

    return None


def verify_well_typed(
    template_file: str,
    hhstc_pattern: str,
    out_dir: str,
    seed_range: Tuple[int, int],
    skip_hhstc: bool,
    milner_exe: str,
    hhstc_exe: str,
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
                )
            )

        # If there were any errors, display the first failure and fail
        exit_code = 0
        smallest_res = None
        for future in as_completed(futures):
            res: Failure = future.result()
            if res is not None:
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

    args: argparse.Namespace = parser.parse_args()

    seed_range = (int(args.seed_range[0]), int(args.seed_range[1]))

    # Temporary directory to store generated programs
    out_dir = tempfile.TemporaryDirectory()
    exit_code = verify_well_typed(
        args.template,
        args.hhstc_pattern,
        out_dir.name,
        seed_range,
        args.skip_hhstc,
        args.milner_exe,
        args.hhstc_exe,
    )
    out_dir.cleanup()

    exit(exit_code)


if __name__ == "__main__":
    main()
