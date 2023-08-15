#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import asyncio
import multiprocessing
import os
import re
import shlex
import shutil
import subprocess
import sys

import pkg_resources

"""
* Invoke from the `/fbsource/fbcode/` directory using `buck run`:

    buck run thrift/compiler/test:build_fixtures

will build all fixtures under `thrift/compiler/test/fixtures`. Or

    buck run thrift/compiler/test:build_fixtures -- --fixture-names [$FIXTURENAMES]

will only build selected fixtures under `thrift/compiler/test/fixtures`.

* Invoke directly (if buck is not available):

    thrift/compiler/test/build_fixtures.py \
            --thrift-bin [$THRIFTBIN]      \
            --fixture-root [$FIXTUREROOT]  \
            --fixture-names [$FIXTURENAMES]

$THRIFTBIN is the absolute or relative path to thrift compiler executable.
$FIXTUREROOT/thrift/compiler/test/fixtures is the fixture dir (defaults
to the current working directory) and $FIXTURENAMES is a list of
fixture names to build specifically (default is to build all fixtures).
"""
FIXTURE_ROOT = "."


def parsed_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--thrift-bin",
        dest="thrift_bin",
        help=(
            "Absolute path or relative path to thrift compiler executable. "
            "For relative path, we will search script's path and all parents"
        ),
        type=str,
        default=None,
    )
    parser.add_argument(
        "--fixture-root",
        dest="fixture_root",
        help=(
            "$FIXTUREROOT/thrift/compiler/test/fixtures is where the"
            " fixtures are located, defaults to the current working dir"
        ),
        type=str,
        default=FIXTURE_ROOT,
    )
    parser.add_argument(
        "--fixture-names",
        dest="fixture_names",
        help="Name of the fixture to build, default is to build all fixtures",
        type=str,
        nargs="*",
        default=None,
    )
    return parser.parse_args()


def ascend_find_exe(path, target):
    if not os.path.isdir(path):
        path = os.path.dirname(path)
    while True:
        test = os.path.join(path, target)
        if os.access(test, os.X_OK):
            return test
        parent = os.path.dirname(path)
        if os.path.samefile(parent, path):
            return None
        path = parent


def read_lines(path):
    with open(path, "r") as f:
        return f.readlines()


args = parsed_args()
fixture_root = args.fixture_root
exe = os.path.join(os.getcwd(), sys.argv[0])
thrift = args.thrift_bin
if args.thrift_bin is None:
    thrift = pkg_resources.resource_filename(__name__, "thrift")
else:
    thrift = ascend_find_exe(exe, args.thrift_bin)
if thrift is None:
    tb = args.thrift_bin
    sys.stderr.write("error: cannot find the Thrift compiler ({})\n".format(tb))
    sys.exit(1)

fixture_dir = os.path.join(fixture_root, "thrift/compiler/test/fixtures")
fixture_names = (
    args.fixture_names
    if args.fixture_names is not None
    else sorted(
        f
        for f in os.listdir(fixture_dir)
        if os.path.isfile(os.path.join(fixture_dir, f, "cmd"))
    )
)

has_errors = False

# Semaphore to limit the number of concurrent fixture builds.
# Otherwise, a swarm of compiler processes results in too much
# CPU contention, consistenly killing people's devservers.
sem = asyncio.Semaphore(value=multiprocessing.cpu_count())


async def run_subprocess(cmd, *, cwd):
    async with sem:
        p = await asyncio.create_subprocess_exec(
            *cmd,
            cwd=cwd,
            close_fds=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        out, err = await p.communicate()
        sys.stdout.write(out.decode(sys.stdout.encoding))
        if p.returncode != 0:
            global has_errors
            has_errors = True
            sys.stderr.write(err.decode(sys.stderr.encoding))


async def main():
    processes = []

    msg_format = "Building fixture {{0:>{w}}}/{{1}}: {{2}}".format(
        w=len(str(len(fixture_names)))
    )
    for name, index in zip(fixture_names, range(len(fixture_names))):
        msg = msg_format.format(index + 1, len(fixture_names), name)
        print(msg, file=sys.stderr)
        fixture_src = os.path.join(fixture_dir, name)
        for fn in set(os.listdir(fixture_src)) - {"cmd", "src"}:
            if fn.startswith("."):
                continue
            shutil.rmtree(os.path.join(fixture_src, fn))
        cmds = read_lines(os.path.join(fixture_src, "cmd"))
        for cmd in cmds:
            if re.match(r"^\s*#", cmd):
                continue
            args = shlex.split(cmd.strip())
            args[1] = os.path.relpath(os.path.join(fixture_src, args[1]), fixture_root)
            base_args = [
                thrift,
                "-r",
                "-I",
                os.path.abspath(fixture_src),
                "-I",
                os.path.abspath(fixture_root),
                "-o",
                os.path.abspath(fixture_src),
                "--gen",
            ]
            if "mstch_cpp" in args[0]:
                path = os.path.join("thrift/compiler/test/fixtures", name)
                extra = "include_prefix=" + path
                join = "," if ":" in args[0] else ":"
                args[0] = args[0] + join + extra
            if (
                "cpp2" in args[0]
                or "schema" in args[0]
                or "mstch_java" in args[0]
                or "mstch_python" in args[0]
            ):
                # TODO: (yuhanhao) T41937765 When use generators that use
                # `mstch_objects` in recursive mode, if included thrift file
                # contains const structs or const union, generater will attempt to
                # de-reference a nullptr in `mstch_const_value::const_struct()`.
                # This is a hack before this is resolved.
                base_args.remove("-r")
            xargs = base_args + args
            processes.append(run_subprocess(xargs, cwd=fixture_root))

    await asyncio.gather(*processes)


asyncio.run(main())

sys.exit(int(has_errors))
