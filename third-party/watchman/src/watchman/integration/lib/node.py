# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

import distutils.spawn
import os
import subprocess


def _find_node():
    return os.environ.get("NODE_BIN", distutils.spawn.find_executable("node"))


def _find_yarn():
    return os.environ.get("YARN_PATH", distutils.spawn.find_executable("yarn"))


# To avoid CI environments that put broken yarn and node executables in PATH,
# verify they at least run.
def _ensure_can_run(binary_path) -> None:
    if binary_path is None:
        return None
    try:
        if 0 != subprocess.call([binary_path, "--version"], stdout=subprocess.DEVNULL):
            return None
    except OSError:
        return None
    return binary_path


node_bin = _ensure_can_run(_find_node())
yarn_bin = _ensure_can_run(_find_yarn())
