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

import unittest

from testing.thrift_types import Nested1 as NestedPython


# By convention, srcs in `py3/test/auto_migrate/` are used by two different test targets:
#   - one target in `py3/test:` runs without auto-migrate
#   - another target in `py3/test/auto_migrate:` runs with auto-migrate enabled
# This utility method helps writing test code that supports both setups.
def is_auto_migrated() -> bool:
    # pyre-ignore[16]: Undefined attribute; using non-public API
    return NestedPython._fbthrift_auto_migrate_enabled()


def brokenInAutoMigrate():  # pyre-ignore[3] unittest isn't very well typed
    # kinda janky way of testing if we're in auto migrate mode or not
    if is_auto_migrated():
        return unittest.expectedFailure
    return lambda func: func
