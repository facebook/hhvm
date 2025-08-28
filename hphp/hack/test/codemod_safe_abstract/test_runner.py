#!/usr/bin/env python3

"""
Generates snapshots of the codemod safe abstract orchestrator output for testing.
See ./BUCK for how to run this.
Example output: ./test_simple_abstraxt/.hhconfig.exp

"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def run_command(cmd, cwd=None):
    """Run a command and return stdout, stderr, and return code."""
    try:
        result = subprocess.run(
            cmd, cwd=cwd, capture_output=True, text=True, shell=True
        )
        return result.stdout, result.stderr, result.returncode
    except Exception as e:
        return "", str(e), 1


def get_changes_diff(working_dir, initial_commit):
    """Get diff showing changes made by the orchestrator using sapling."""
    # Use sapling diff to show changes from the initial commit to current state
    cmd = f"sl diff -r {initial_commit}"
    stdout, stderr, returncode = run_command(cmd, cwd=working_dir)

    if returncode == 0:
        return stdout.strip()
    else:
        # If sapling diff fails, return empty (no changes)
        return ""


def main():
    parser = argparse.ArgumentParser(
        description="Test runner for codemod safe abstract orchestrator"
    )
    parser.add_argument("config_filename", help="Config filename (e.g., .hhconfig)")
    parser.add_argument("orchestrator_binary", help="Path to the orchestrator binary")
    parser.add_argument("codemod_binary", help="Path to the codemod binary")
    parser.add_argument("hh_distc_binary", help="Path to the hh_distc binary")
    parser.add_argument("worker_binary", help="Path to the worker binary")

    args = parser.parse_args()

    # The verify.py script runs us from the test directory, so we can use the current working directory
    test_dir = Path.cwd()
    config_path = test_dir / args.config_filename

    if not config_path.exists():
        print(f"Config file {config_path} not found", file=sys.stderr)
        sys.exit(1)

    if not test_dir.exists():
        print(f"Test directory {test_dir} does not exist", file=sys.stderr)
        sys.exit(1)

    # Create temporary directories
    with tempfile.TemporaryDirectory() as tmp_base:
        original_dir = os.path.join(tmp_base, "original")
        working_dir = os.path.join(tmp_base, "working")

        # Copy test directory to both original and working directories
        shutil.copytree(test_dir, original_dir)
        shutil.copytree(test_dir, working_dir)

        # Initialize sapling repository in working directory (required for the orchestrator)
        stdout, stderr, returncode = run_command("sl init", cwd=working_dir)
        if returncode != 0:
            print(f"Failed to init sl repo: {stderr}", file=sys.stderr)
            sys.exit(1)

        # Add all files to sapling
        stdout, stderr, returncode = run_command("sl add .", cwd=working_dir)
        if returncode != 0:
            print(f"Failed to add files to sl: {stderr}", file=sys.stderr)
            sys.exit(1)

        # Make initial commit and get its hash
        stdout, stderr, returncode = run_command(
            "sl commit -m 'Initial commit'", cwd=working_dir
        )
        if returncode != 0:
            print(f"Failed to make initial commit: {stderr}", file=sys.stderr)
            sys.exit(1)

        # Get the initial commit hash
        initial_commit_stdout, _, returncode = run_command("sl id", cwd=working_dir)
        if returncode != 0:
            print("Failed to get initial commit hash", file=sys.stderr)
            sys.exit(1)
        initial_commit = initial_commit_stdout.strip()

        # Construct the full command with all required binaries
        cmd = f"{args.orchestrator_binary} --codemod-safe-abstract {args.codemod_binary} --hh_distc {args.hh_distc_binary} --worker {args.worker_binary} --root {working_dir}"
        stdout, stderr, returncode = run_command(cmd, cwd=working_dir)

        # Always try to show the diff first, as the orchestrator may return non-zero even on success
        diff_output = get_changes_diff(working_dir, initial_commit)

        if diff_output.strip():
            # There are changes, show the diff
            print(diff_output)
        elif returncode != 0:
            # No changes but command failed, show error output
            if stdout.strip():
                print(stdout)
            elif stderr.strip():
                print(stderr)
            else:
                print(f"Command failed with exit code {returncode}")
        else:
            # No changes and command succeeded (this is fine for some test cases)
            print("")


if __name__ == "__main__":
    main()
