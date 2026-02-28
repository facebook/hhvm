# pyre-strict
import concurrent
import os
import random
import subprocess
import sys
import tempfile
from concurrent.futures import as_completed
from dataclasses import dataclass
from subprocess import CompletedProcess
from typing import List, Literal, Optional, Tuple, Union

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
    process: CompletedProcess[bytes]
    cmds: List[str]


def bad_exit(msg: str, proc: CompletedProcess[bytes], cmds: List[str]) -> None:
    tqdm.write(msg)
    tqdm.write("Steps to repro:")
    tqdm.write("\n".join(cmds))
    tqdm.write("stdout:")
    tqdm.write(proc.stdout.decode() if proc.stdout is not None else "N/A")
    tqdm.write("stderr:")
    tqdm.write(proc.stderr.decode() if proc.stderr is not None else "N/A")


def mk_cmd(process: CompletedProcess[bytes]) -> str:
    return " ".join(process.args)


def generate_hhvm_compilation_and_run_commands(
    hhvm_exe: str,
) -> Optional[Tuple[List[str], List[str]]]:
    hhbbc_compilation_args = [
        hhvm_exe,
        "--hphp",
        "-l3",
        "-c",
        "/usr/local/hphpi/cli.hdf",
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
    milner_exe: str, out_dir: str, template: str, cmds: List[str]
) -> Union[MilnerSuccess, Failure]:
    seed = random.randint(0, 2**31 - 1)
    basename = os.path.basename(template)
    program_file = os.path.join(out_dir, f"{basename}.{seed}.out")

    # Generate a Hack program with the randomly chosen seed
    process = subprocess.run(
        [milner_exe, os.path.abspath(template), "--seed", str(seed)],
        stdout=open(program_file, "w"),
        stderr=subprocess.PIPE,
    )
    cmds = cmds.copy()
    cmds.append(mk_cmd(process))
    if process.returncode != 0:
        return Failure(seed, process, cmds)

    return MilnerSuccess(seed, program_file, cmds)


def generate_programs(
    milner_exe: str, out_dir: str, template: str, sample_size: int, cmds: List[str]
) -> Tuple[int, List[MilnerSuccess]]:
    programs: List[MilnerSuccess] = []
    exit_codes = [0]
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for _ in range(0, sample_size):
            futures.append(
                executor.submit(generate_program, milner_exe, out_dir, template, cmds)
            )
        res: Union[Failure, MilnerSuccess]
        for future in tqdm(as_completed(futures), total=len(futures)):
            res = future.result()
            if type(res) is Failure:
                exit_codes.append(3)
                bad_exit(
                    "milner failed while generating a program with seed {}".format(
                        res.seed
                    ),
                    res.process,
                    res.cmds,
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
) -> Optional[Failure]:
    cmds = program.cmds.copy()
    match mode:
        case "HHBBC":
            # Run HHBBC
            compilation_dir = tempfile.TemporaryDirectory(dir=out_dir)
            process = subprocess.run(
                hhbbc_compilation_args
                + ["--output-dir", compilation_dir.name, program.program_file],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            cmds.append(mk_cmd(process))
            if process.returncode != 0:
                return Failure(seed=program.seed, process=process, cmds=cmds)

            # Run HHVM
            process = subprocess.run(
                hhvm_run_args
                + [
                    "-vRepo.Path={}/hhvm.hhbc".format(compilation_dir.name),
                    "--file",
                    program.program_file,
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            cmds.append(mk_cmd(process))
            if process.returncode != 0:
                return Failure(
                    seed=program.seed,
                    process=process,
                    cmds=cmds,
                )
        case "Sandbox":
            # Run HHVM
            process = subprocess.run(
                [
                    hhvm_exe,
                    "-vHack.Lang.AllowUnstableFeatures=1",
                    program.program_file,
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            cmds.append(mk_cmd(process))
            if process.returncode != 0:
                return Failure(
                    seed=program.seed,
                    process=process,
                    cmds=cmds,
                )


def run_hhvm_programs(
    hhvm_exe: str,
    out_dir: str,
    hhbbc_compilation_args: List[str],
    hhvm_run_args: List[str],
    programs: List[MilnerSuccess],
    mode: Mode,
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
                )
            )
        res: Optional[Failure]
        for future in tqdm(as_completed(futures), total=len(futures)):
            res = future.result()
            if res is not None:
                exit_codes.append(4)
                bad_exit(
                    "HHVM while compiling/running a program with seed {}".format(
                        res.seed
                    ),
                    res.process,
                    res.cmds,
                )
    return max(exit_codes)


def verify_runtime(
    milner_exe: str,
    hhvm_exe: str,
    out_dir: str,
    template: str,
    sample_size: int,
    mode: Mode,
) -> int:
    cmds = []
    print("Generating programs...")
    exit_code, programs = generate_programs(
        milner_exe=milner_exe,
        out_dir=out_dir,
        template=template,
        sample_size=sample_size,
        cmds=cmds,
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

    args: argparse.Namespace = parser.parse_args()

    # Set a seed for reproducibility of runs
    seed = args.global_seed
    if seed is None:
        seed = random.randint(0, sys.maxsize)
    print("Global seed for this run: {}".format(seed))
    random.seed(seed)

    # Temporary directory to store generated programs
    out_dir = tempfile.TemporaryDirectory()
    exit_code = verify_runtime(
        milner_exe=args.milner_exe,
        hhvm_exe=args.hhvm_exe,
        template=args.template,
        out_dir=out_dir.name,
        sample_size=args.sample_size,
        mode=args.mode,
    )
    out_dir.cleanup()

    exit(exit_code)


if __name__ == "__main__":
    main()
