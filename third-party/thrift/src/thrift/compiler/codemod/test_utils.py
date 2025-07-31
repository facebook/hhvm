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

# pyre-unsafe

import os
import subprocess


def read_file(path):
    with open(path, "r") as f:
        return f.read()


def write_file(path, content):
    if d := os.path.dirname(path):
        os.makedirs(d, exist_ok=True)
    with open(path, "w") as f:
        f.write(content)


def run_binary(build_rule, path):
    subprocess.check_call([build_rule, path])
