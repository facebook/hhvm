# pyre-strict

import concurrent.futures
import itertools
import os
import subprocess
import tempfile
from dataclasses import dataclass
from typing import Optional


@dataclass
class Failure:
    path: str
    contents: str
    stdout: bytes
    stderr: bytes


def milner_and_type_check(
    milner_exe: str,
    hhstc_exe: str,
    out_dir: str,
    template_file: str,
    seed: int,
) -> Optional[Failure]:
    basename = os.path.basename(template_file)
    temp_file = os.path.join(out_dir, f"{basename}.{seed}.out")

    # Run the generator on the template file and the number
    with open(temp_file, "w") as out:
        result = subprocess.run(
            [milner_exe, os.path.abspath(template_file), "--seed", str(seed)],
            stdout=out,
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
    result = subprocess.run([hhstc_exe, temp_file], capture_output=True)

    # Check if the verifier found any errors
    stdout = result.stdout.decode()
    if "No errors" not in stdout:
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
    test_dir: str, out_dir: str, milner_exe: str, hhstc_exe: str
) -> int:
    # Find all template files in the test directory
    template_files = [
        os.path.join(test_dir, f)
        for f in os.listdir(test_dir)
        if f.endswith(".template")
    ]

    # Iterate over each template file, in parallel generate programs with seeds
    # (1...100) with milner and verify that they are well-typed with
    # hh_single_type_check
    product = itertools.product(template_files, range(1, 101))
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for template_file, seed in product:
            futures.append(
                executor.submit(
                    milner_and_type_check,
                    milner_exe,
                    hhstc_exe,
                    out_dir,
                    template_file,
                    seed,
                )
            )
        concurrent.futures.wait(futures)

        # If there were any errors, display the first failure and fail
        for future in futures:
            res: Failure = future.result()
            if res is not None:
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
                return 1

    # If we made it here, then all templates only generated well-typed programs
    print("All templates passed verification!")
    return 0


def main() -> None:
    import argparse

    parser = argparse.ArgumentParser(description="Verify well-typed templates")
    parser.add_argument("root", help="Directory to search for templates")
    parser.add_argument("--milner-exe", required=True, help="Path to milner executable")
    parser.add_argument(
        "--hhstc-exe", required=True, help="Path to hh_single_type_check executable"
    )

    args: argparse.Namespace = parser.parse_args()

    # Temporary directory to store generated programs
    out_dir = tempfile.TemporaryDirectory()
    exit_code = verify_well_typed(
        args.root, out_dir.name, args.milner_exe, args.hhstc_exe
    )
    out_dir.cleanup()

    exit(exit_code)


if __name__ == "__main__":
    main()
