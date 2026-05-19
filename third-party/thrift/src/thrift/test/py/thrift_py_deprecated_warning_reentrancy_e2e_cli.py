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

# pyre-strict

import importlib
import logging
import sys
import warnings
from types import ModuleType

from thrift import Thrift as thrift_module

_INITIAL_GENERATED_MODULE = "thrift.test.py.PythonReservedKeywords.ttypes"
_NESTED_GENERATED_MODULE = "thrift.test.py.constants.ttypes"


class InstagramE2ETestExecutionUUIDFilterShape(logging.Filter):
    def __init__(self) -> None:
        super().__init__()
        self.filter_calls = 0

    def filter(self, record: logging.LogRecord) -> bool:
        self.filter_calls += 1
        _trigger_nested_generated_warning()
        record.test_execution_uuid = "none"
        return True


def _trigger_initial_generated_warning() -> ModuleType:
    return importlib.import_module(_INITIAL_GENERATED_MODULE)


def _trigger_nested_generated_warning() -> ModuleType:
    generated_module = importlib.import_module(_NESTED_GENERATED_MODULE)
    return importlib.reload(generated_module)


def _install_warning_logging_filter() -> InstagramE2ETestExecutionUUIDFilterShape:
    warning_logger = logging.getLogger("py.warnings")
    warning_logger.filters.clear()
    warning_logger.handlers.clear()
    warning_logger.propagate = False

    handler = logging.StreamHandler(sys.stderr)
    handler.setFormatter(logging.Formatter("%(message)s"))
    warning_logger.addHandler(handler)

    execution_uuid_filter = InstagramE2ETestExecutionUUIDFilterShape()
    warning_logger.addFilter(execution_uuid_filter)
    logging.captureWarnings(True)
    return execution_uuid_filter


def _remove_cpython_warning_originals() -> None:
    for attribute in ("_showwarning_orig", "_formatwarning_orig"):
        if hasattr(warnings, attribute):
            delattr(warnings, attribute)


def _remove_cpython_formatwarning_original() -> None:
    if hasattr(warnings, "_formatwarning_orig"):
        delattr(warnings, "_formatwarning_orig")


def _enable_lazy_import_probe() -> None:
    def is_lazy_imports_enabled() -> bool:
        return True

    importlib.__dict__["is_lazy_imports_enabled"] = is_lazy_imports_enabled


def main() -> None:
    scenario = sys.argv[1]
    warnings.resetwarnings()
    warnings.simplefilter("always", thrift_module.ThriftPyDeprecatedWarning)
    execution_uuid_filter = _install_warning_logging_filter()
    if scenario == "missing-cpython-formatwarning-original":
        _remove_cpython_formatwarning_original()
    if scenario == "lazy-missing-cpython-originals":
        _remove_cpython_warning_originals()
    if scenario == "lazy-missing-cpython-originals":
        _enable_lazy_import_probe()

    _trigger_initial_generated_warning()
    print(f"filter calls: {execution_uuid_filter.filter_calls}")
