#!/usr/bin/env python3
# pyre-strict

import os
import shutil
import subprocess
import sys
from typing import List


def arg_extract(args: List[str], arg: str) -> str:
    index = args.index(arg)
    value = args.pop(index + 1)
    del args[index]
    return value


if __name__ == "__main__":
    # Parsing the arguments is not straightforward as pessimise_and_single_type_check
    # must be aware of all the arguments supported by hh_single_type_check, but we do
    # not want to duplicate the list here (which is long and moving fast).
    # We thus use the following ad-hoc strategy:
    #
    # 1. extract the paths to hh_stc, hh_pessimisation, pessimised_hhi from the arguments
    # 2. delete the binary name
    # 3. extract all the *.php strings, assuming these are the files being tested
    # 4. the remaining arguments are the flags to be passed to hh_single_type_checks

    arguments: List[str] = sys.argv
    hh_stc_path: str = arg_extract(arguments, "--hh-stc-path")
    hh_pessimisation_path: str = arg_extract(arguments, "--hh-pessimisation-path")
    pessimised_hhi_path: str = arg_extract(arguments, "--pessimised-hhi-path")
    del arguments[0]

    hh_stc_arguments: List[str] = []
    files: List[str] = []

    for a in arguments:
        if a.endswith(".php"):
            files.append(a)
        else:
            hh_stc_arguments.append(a)

    # invoke hh_pessimise on the all the current tests (say foo.php)
    for file in files:
        cmd: List[str] = [
            hh_pessimisation_path,
            "--file=" + file,
            "--update",
            "--enable-multifile-support",
        ]
        subprocess.run(cmd)

    # identify the extra builtins used
    extra_builtins: List[str] = []
    while True:
        try:
            extra_builtins.append(arg_extract(hh_stc_arguments, "--extra-builtin"))
        except ValueError:
            break

    # pessimise the extra builtins and build the cli options
    # - if extra_builtin refer to <path>/<file>, the pessimised extra builtin is
    #   generated inside <path>/pessimised_hhi_<PID>/<file>
    # - the <PID> is required to avoid a race condition when multiple instances of
    #   this script are invoked concurrently by buck
    # - the <path>/pessimised_hhi_<PID>/ directories are deleted at the end of the script
    extra_builtins_opts: List[str] = []
    pessimised_extra_builtins_dirs: List[str] = []

    for file in extra_builtins:
        (file_dir, file_name) = os.path.split(file)
        output_file = file + ".pess"
        unique_pessimised_file_dir = os.path.join(
            file_dir, "pessimised_hhi_" + str(os.getpid())
        )
        os.makedirs(unique_pessimised_file_dir, exist_ok=True)
        pessimised_file = os.path.join(unique_pessimised_file_dir, file_name)
        if not (os.path.exists(pessimised_file)):
            cmd: List[str] = [
                hh_pessimisation_path,
                "--file",
                file,
                "--output-file",
                pessimised_file,
            ]
            subprocess.run(cmd)
        extra_builtins_opts.append("--extra-builtin")
        extra_builtins_opts.append(pessimised_file)
        pessimised_extra_builtins_dirs.append(unique_pessimised_file_dir)

    # pessimised tests have extension file+".pess"
    # invoke hh_single_type_check on the pessimised tests
    pessimised_files: List[str] = [file + ".pess" for file in files]

    cmd: List[str] = (
        [hh_stc_path]
        + hh_stc_arguments
        + extra_builtins_opts
        + [
            "--enable-sound-dynamic-type",
            "--like-type-hints",
            "--config",
            "pessimise_builtins=true",
            "--force-allow-builtins-in-custom-hhi-path-FLAKEY",
        ]
        + ["--custom-hhi-path", pessimised_hhi_path]
        + pessimised_files
    )
    subprocess.run(cmd)

    # delete the temporary directories where extra builtins files have been pessimised
    for dir in pessimised_extra_builtins_dirs:
        shutil.rmtree(dir)

    # Remark: in batch-mode the out files will have extension ".pess.out"
    # instead of ".out"
