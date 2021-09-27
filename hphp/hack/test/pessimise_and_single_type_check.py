#!/usr/bin/env python3
# pyre-strict

import subprocess
import sys
from typing import List

if __name__ == "__main__":
    # Parsing the arguments is not straightforward as pessimise_and_single_type_check
    # must be aware of all the arguments supported by hh_single_type_check, but we do
    # not want to duplicate the list here (which is long and moving fast).
    # We thus use the following ad-hoc strategy:
    #
    # 1. extract the paths to hh_stc and hh_pessimisation from the arguments
    # 2. delete the binary name
    # 3. extract all the *.php strings, assuming these are the files being tested
    # 4. the remaining arguments are the flags to be passed to hh_single_type_checks

    arguments: List[str] = sys.argv
    hh_stc_path_index: int = arguments.index("--hh-stc-path")
    hh_stc_path: str = arguments.pop(hh_stc_path_index + 1)
    del arguments[hh_stc_path_index]

    hh_pessimisation_path_index: int = arguments.index("--hh-pessimisation-path")
    hh_pessimisation_path: str = arguments.pop(hh_pessimisation_path_index + 1)
    del arguments[hh_pessimisation_path_index]

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
        cmd: List[str] = [hh_pessimisation_path, "--file=" + file, "--update"]
        subprocess.run(cmd)

    # pessimised tests have extension file+".pess"
    # invoke hh_single_type_check on the pessimised tests
    pessimised_files: List[str] = list(map(lambda file: file + ".pess", files))
    cmd: List[str] = (
        [hh_stc_path]
        + hh_stc_arguments
        + ["--enable-sound-dynamic-type", "--like-type-hints"]
        + pessimised_files
    )
    subprocess.run(cmd)

    # Remark: in batch-mode the out files will have extension ".pess.out"
    # instead of ".out"
