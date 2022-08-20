# Copyright (c) Facebook, Inc. and its affiliates.
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

import os
import subprocess
import sys


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


def read_file(path):
    with open(path, "r") as f:
        return f.read()


def write_file(path, content):
    if d := os.path.dirname(path):
        os.makedirs(d)
    with open(path, "w") as f:
        f.write(content)


def run_binary(build_rule, path):
    exe = os.path.join(os.getcwd(), sys.argv[0])
    build_rule = ascend_find_exe(exe, build_rule)
    subprocess.call([build_rule, "--gen", "mstch_cpp2", path])
