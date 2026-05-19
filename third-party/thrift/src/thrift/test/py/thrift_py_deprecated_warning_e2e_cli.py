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
import sys
import warnings

import click
from thrift import Thrift as thrift_module

_SOURCE_A_MODULE = "thrift.test.py.thrift_py_deprecated_warning_e2e_source_a"
_SOURCE_B_MODULE = "thrift.test.py.thrift_py_deprecated_warning_e2e_source_b"


def _configure_warning_mode(warning_mode: str) -> None:
    if warning_mode == "helper-default":
        return
    if warning_mode == "python-default":
        warnings.filterwarnings(
            "default",
            category=thrift_module.ThriftPyDeprecatedWarning,
        )
        return
    if warning_mode == "always":
        warnings.filterwarnings(
            "always",
            category=thrift_module.ThriftPyDeprecatedWarning,
        )
        return
    if warning_mode == "error":
        warnings.filterwarnings(
            "error",
            category=thrift_module.ThriftPyDeprecatedWarning,
        )
        return
    if warning_mode == "ignore":
        warnings.filterwarnings(
            "ignore",
            category=thrift_module.ThriftPyDeprecatedWarning,
        )
        return
    raise click.BadParameter(f"unsupported warning mode: {warning_mode}")


def _run_scenario(scenario: str) -> None:
    if scenario == "source-a":
        importlib.import_module(_SOURCE_A_MODULE)
        return
    if scenario == "source-b":
        importlib.import_module(_SOURCE_B_MODULE)
        return
    if scenario == "two-source-modules":
        importlib.import_module(_SOURCE_A_MODULE)
        importlib.import_module(_SOURCE_B_MODULE)
        return
    if scenario == "lazy-first-use":
        from thrift.test.py import thrift_py_deprecated_warning_e2e_lazy_source

        thrift_py_deprecated_warning_e2e_lazy_source.use_generated_symbol()
        return
    raise click.BadParameter(f"unsupported scenario: {scenario}")


@click.command()
@click.argument("warning_mode")
@click.argument("scenario")
def entrypoint(warning_mode: str, scenario: str) -> None:
    _configure_warning_mode(warning_mode)
    _run_scenario(scenario)
    if warning_mode == "helper-default" and sys.stderr.isatty():
        click.echo("Hello Thrift!!!", err=True)
    if warning_mode == "ignore":
        click.echo(f"completed {warning_mode} {scenario}")
