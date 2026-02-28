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

# pyre-strict

import os
import sys

import pytest
from buck2.tests.e2e_util.api.buck import Buck
from buck2.tests.e2e_util.api.buck_result import TestResult
from buck2.tests.e2e_util.buck_workspace import buck_test


@buck_test(inplace=True)
async def test_par(buck: Buck) -> None:
    target = os.getenv("TARGET")
    if not target:
        pytest.fail("error: Target not provided")

    test_result = await run_buck(buck, target)

    print(test_result.stderr, file=sys.stderr)


async def run_buck(buck: Buck, target: str) -> TestResult:
    args = [target, "-c thrift.py3_auto_migrate=True", "--", "--return-zero-on-skips"]
    result = await buck.test(*args)
    return result
