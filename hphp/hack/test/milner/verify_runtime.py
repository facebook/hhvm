# pyre-strict
import concurrent
import json
import os
import random
import re
import shlex
import subprocess
import sys
import tempfile
from concurrent.futures import as_completed
from dataclasses import dataclass
from pathlib import Path
from subprocess import CompletedProcess
from typing import List, Literal, Optional, TextIO, Tuple, Union

from tqdm import tqdm

Mode = Union[Literal["Sandbox"], Literal["HHBBC"]]


@dataclass
class MilnerSuccess:
    seed: int
    program_file: str
    cmds: List[str]


@dataclass
class Failure:
    seed: int
    process: Optional[CompletedProcess[bytes]]
    cmds: List[str]
    program_file: str = ""
    error: Optional[str] = None
    stdout: bytes = b""
    stderr: bytes = b""


def prepare_runtime_files(program_file: str) -> Tuple[str, List[str]]:
    contents = Path(program_file).read_bytes()
    parts = re.split(rb"(?m)^////[ \t]+([^\r\n]+)\r?\n", contents)
    if len(parts) == 1:
        return (program_file, [program_file])
    if parts[0].strip():
        raise ValueError("Unexpected contents before the first virtual file")
    files: List[Tuple[Path, bytes]] = []
    for name, body in zip(parts[1::2], parts[2::2]):
        path = Path(name.decode().strip())
        if path.is_absolute() or not path.parts or ".." in path.parts:
            raise ValueError(f"Invalid virtual file path: {name!r}")
        if any(existing == path for existing, _ in files):
            raise ValueError(f"Duplicate virtual file path: {name!r}")
        files.append((path, body))
    entrypoints = [path for path, body in files if b"__EntryPoint" in body]
    if Path("main.php") in entrypoints:
        entrypoint = Path("main.php")
    elif len(entrypoints) == 1:
        entrypoint = entrypoints[0]
    else:
        raise ValueError("Expected main.php or a unique virtual entrypoint file")
    directory = Path(
        tempfile.mkdtemp(
            prefix=os.path.basename(program_file) + ".files.",
            dir=os.path.dirname(program_file),
        )
    )
    for path, body in files:
        destination = directory / path
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(body)
    return (
        str(directory / entrypoint),
        [str(directory / path) for path, _ in files],
    )


def record_failure(out_dir: str, failure: Failure) -> None:
    filename = failure.program_file or os.path.join(out_dir, str(failure.seed))
    process = failure.process
    with open(filename + ".failure.json", "w") as output:
        json.dump(
            {
                "program": failure.program_file,
                "seed": failure.seed,
                "commands": failure.cmds,
                "returncode": process.returncode if process is not None else None,
                "error": failure.error,
                "stdout": (
                    (process.stdout or b"") if process is not None else failure.stdout
                ).decode(errors="replace"),
                "stderr": (
                    (process.stderr or b"") if process is not None else failure.stderr
                ).decode(errors="replace"),
            },
            output,
            indent=2,
        )
        output.write("\n")


def bad_exit(msg: str, failure: Failure) -> None:
    tqdm.write(msg)
    if failure.error is not None:
        tqdm.write(failure.error)
    tqdm.write("Steps to repro:")
    tqdm.write("\n".join(failure.cmds))
    proc = failure.process
    tqdm.write("stdout:")
    stdout = (proc.stdout or b"") if proc is not None else failure.stdout
    tqdm.write(stdout.decode(errors="replace"))
    tqdm.write("stderr:")
    stderr = (proc.stderr or b"") if proc is not None else failure.stderr
    tqdm.write(stderr.decode(errors="replace"))


def run_command(
    command: List[str],
    seed: int,
    program_file: str,
    cmds: List[str],
    timeout: float,
    stdout: Union[int, TextIO] = subprocess.PIPE,
) -> Union[CompletedProcess[bytes], Failure]:
    cmds.append(shlex.join(command))
    try:
        process = subprocess.run(
            command, stdout=stdout, stderr=subprocess.PIPE, timeout=timeout
        )
    except subprocess.TimeoutExpired as error:
        return Failure(
            seed,
            None,
            cmds,
            program_file,
            f"Command timed out after {timeout} seconds",
            error.stdout or b"",
            error.stderr or b"",
        )
    except OSError as error:
        return Failure(seed, None, cmds, program_file, str(error))
    if process.returncode != 0:
        return Failure(seed, process, cmds, program_file)
    return process


def generate_hhvm_compilation_and_run_commands(
    hhvm_exe: str,
) -> Optional[Tuple[List[str], List[str]]]:
    hhbbc_compilation_args = [
        hhvm_exe,
        "--hphp",
        "-l3",
        "-c",
        "/usr/local/hphpi/cli.hdf",
        "-vParserThreadCount=1",
        "-vRuntime.Eval.JitEnableRenameFunction=0",
        "-vRuntime.Eval.GdbSyncChunks=1",
        "-vRuntime.Eval.AllowHhas=true",
        "-vRuntime.Eval.Jit=1",
        "-vRuntime.Eval.JitRetranslateAllRequest=2",
        "-vRuntime.Eval.JitRetranslateAllSeconds=300",
        "-vRuntime.Hack.Lang.AllowUnstableFeatures=1",
        "-vEval.PreludePath=",
    ]
    hhvm_run_args = [
        hhvm_exe,
        "-c",
        "/usr/local/hphpi/cli.hdf",
        "-vEval.JitEnableRenameFunction=0",
        "-vEval.GdbSyncChunks=1",
        "-vEval.AllowHhas=true",
        "-vEval.Jit=1",
        "--count=4",
        "-vEval.JitRetranslateAllRequest=2",
        "-vEval.JitRetranslateAllSeconds=300",
        "-vRepo.Authoritative=true",
        "-vRuntime.Hack.Lang.AllowUnstableFeatures=1",
        "-vEval.PreludePath=",
    ]

    return (hhbbc_compilation_args, hhvm_run_args)


def generate_program(
    milner_exe: str,
    out_dir: str,
    template: str,
    cmds: List[str],
    timeout: float = 180,
    seed: Optional[int] = None,
) -> Union[MilnerSuccess, Failure]:
    if seed is None:
        seed = random.randint(0, 2**31 - 1)
    basename = os.path.basename(template)
    program_file = os.path.join(out_dir, f"{basename}.{seed}.out")

    # Generate a Hack program with the randomly chosen seed
    cmds = cmds.copy()
    try:
        with open(program_file, "x") as output:
            result = run_command(
                [milner_exe, os.path.abspath(template), "--seed", str(seed)],
                seed,
                program_file,
                cmds,
                timeout,
                stdout=output,
            )
    except OSError as error:
        return Failure(seed, None, cmds, program_file, str(error))
    if isinstance(result, Failure):
        return result

    return MilnerSuccess(seed, program_file, cmds)


def generate_programs(
    milner_exe: str,
    out_dir: str,
    template: str,
    sample_size: int,
    cmds: List[str],
    timeout: float = 180,
) -> Tuple[int, List[MilnerSuccess]]:
    programs: List[MilnerSuccess] = []
    exit_codes = [0]
    seeds = random.sample(range(2**31), sample_size)
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for seed in seeds:
            futures.append(
                executor.submit(
                    generate_program, milner_exe, out_dir, template, cmds, timeout, seed
                )
            )
        res: Union[Failure, MilnerSuccess]
        for future in tqdm(as_completed(futures), total=len(futures)):
            res = future.result()
            if type(res) is Failure:
                exit_codes.append(3)
                record_failure(out_dir, res)
                bad_exit(
                    "milner failed while generating a program with seed {}".format(
                        res.seed
                    ),
                    res,
                )
            elif type(res) is MilnerSuccess:
                programs.append(res)
            else:
                exit_codes.append(255)
    return (max(exit_codes), programs)


def run_hhvm_program(
    hhvm_exe: str,
    out_dir: str,
    hhbbc_compilation_args: List[str],
    hhvm_run_args: List[str],
    program: MilnerSuccess,
    mode: Mode,
    timeout: float = 180,
) -> Optional[Failure]:
    cmds = program.cmds.copy()
    try:
        entrypoint, input_files = prepare_runtime_files(program.program_file)
        match mode:
            case "HHBBC":
                with tempfile.TemporaryDirectory(dir=out_dir) as compilation_dir:
                    result = run_command(
                        hhbbc_compilation_args
                        + ["--output-dir", compilation_dir]
                        + input_files,
                        program.seed,
                        program.program_file,
                        cmds,
                        timeout,
                    )
                    if isinstance(result, Failure):
                        return result
                    result = run_command(
                        hhvm_run_args
                        + [
                            f"-vRepo.Path={compilation_dir}/hhvm.hhbc",
                            "--file",
                            entrypoint,
                        ],
                        program.seed,
                        program.program_file,
                        cmds,
                        timeout,
                    )
                    if isinstance(result, Failure):
                        return result
            case "Sandbox":
                result = run_command(
                    [hhvm_exe, "-vHack.Lang.AllowUnstableFeatures=1", entrypoint],
                    program.seed,
                    program.program_file,
                    cmds,
                    timeout,
                )
                if isinstance(result, Failure):
                    return result
    except (OSError, ValueError) as error:
        return Failure(program.seed, None, cmds, program.program_file, str(error))
    return None


def run_hhvm_programs(
    hhvm_exe: str,
    out_dir: str,
    hhbbc_compilation_args: List[str],
    hhvm_run_args: List[str],
    programs: List[MilnerSuccess],
    mode: Mode,
    timeout: float = 180,
) -> int:
    exit_codes = [0]
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for program in programs:
            futures.append(
                executor.submit(
                    run_hhvm_program,
                    hhvm_exe,
                    out_dir,
                    hhbbc_compilation_args,
                    hhvm_run_args,
                    program,
                    mode,
                    timeout,
                )
            )
        res: Optional[Failure]
        for future in tqdm(as_completed(futures), total=len(futures)):
            res = future.result()
            if res is not None:
                exit_codes.append(4)
                record_failure(out_dir, res)
                bad_exit(
                    "HHVM while compiling/running a program with seed {}".format(
                        res.seed
                    ),
                    res,
                )
    return max(exit_codes)


def verify_runtime(
    milner_exe: str,
    hhvm_exe: str,
    out_dir: str,
    template: str,
    sample_size: int,
    mode: Mode,
    timeout: float = 180,
) -> int:
    cmds = []
    print("Generating programs...")
    exit_code, programs = generate_programs(
        milner_exe=milner_exe,
        out_dir=out_dir,
        template=template,
        sample_size=sample_size,
        cmds=cmds,
        timeout=timeout,
    )
    if exit_code != 0:
        return exit_code
    print("Done!")

    print("Generating HHVM compilation/run commands...")
    result = generate_hhvm_compilation_and_run_commands(hhvm_exe=hhvm_exe)
    if result is None:
        return 2
    hhbbc_compilation_args, hhvm_run_args = result
    print("Done!")

    print("Running HHVM...")
    exit_code = run_hhvm_programs(
        hhvm_exe=hhvm_exe,
        out_dir=out_dir,
        hhbbc_compilation_args=hhbbc_compilation_args,
        hhvm_run_args=hhvm_run_args,
        programs=programs,
        mode=mode,
        timeout=timeout,
    )
    if exit_code != 0:
        return exit_code
    print("Done!")

    return 0


def main() -> None:
    import argparse

    parser = argparse.ArgumentParser(
        description="Verify HHVM runs correctly on HHVM generated program"
    )
    parser.add_argument("--template", required=True, help="Path to template")
    parser.add_argument(
        "--sample-size",
        required=True,
        type=int,
        help="Number of programs to generate and run",
    )
    parser.add_argument("--milner-exe", required=True, help="Path to milner executable")
    parser.add_argument("--hhvm-exe", required=True, help="Path to HHVM executable")
    parser.add_argument(
        "--mode",
        choices=["HHBBC", "Sandbox"],
        default="HHBBC",
        help="Picks the HHVM mode to run generated programs",
    )
    parser.add_argument(
        "--global-seed",
        type=int,
        help="Set the global seed to deterministically reproduce another run with a given seed",
    )
    parser.add_argument(
        "--output-dir", help="Keep generated programs and results in a new directory"
    )

    parser.add_argument(
        "--timeout",
        type=float,
        default=180,
        help="Maximum seconds for each generator, compiler, or runtime process",
    )

    args: argparse.Namespace = parser.parse_args()
    if not 0 <= args.sample_size <= 2**31:
        parser.error("--sample-size must be between 0 and 2147483648")
    if args.timeout <= 0:
        parser.error("--timeout must be positive")

    # Set a seed for reproducibility of runs
    seed = args.global_seed
    if seed is None:
        seed = random.randint(0, sys.maxsize)
    print("Global seed for this run: {}".format(seed))
    random.seed(seed)

    if args.output_dir is None:
        temporary_out = tempfile.TemporaryDirectory()
        out_dir = temporary_out.name
    else:
        temporary_out = None
        out_dir = os.path.abspath(args.output_dir)
        os.makedirs(out_dir)
    try:
        exit_code = verify_runtime(
            milner_exe=args.milner_exe,
            hhvm_exe=args.hhvm_exe,
            template=args.template,
            out_dir=out_dir,
            sample_size=args.sample_size,
            mode=args.mode,
            timeout=args.timeout,
        )
        with open(os.path.join(out_dir, "summary.json"), "w") as output:
            json.dump(
                {"exit_code": exit_code, "global_seed": seed, "arguments": vars(args)},
                output,
                indent=2,
            )
            output.write("\n")
    finally:
        if temporary_out is not None:
            temporary_out.cleanup()

    exit(exit_code)


if __name__ == "__main__":
    main()
