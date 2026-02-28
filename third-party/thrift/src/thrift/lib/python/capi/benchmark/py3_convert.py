#!/usr/bin/env fbpython
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

import timeit
from collections import defaultdict

import click
import convertible.thrift_types as python_types  # noqa: F401
import convertible.types as py3_types  # noqa: F401
from tabulate import tabulate
from thrift.py3.converter import to_py3_struct  # noqa: F401
from thrift.python.converter import to_python_struct  # noqa: F401

cost_in_ms = defaultdict(list)


@click.pass_context
def benchmark(ctx, klass_name: str, setup: str, qualifier: str = "") -> None:
    title = f"{klass_name} {qualifier}" if qualifier else klass_name
    to_ctrl = f"to_python_struct(python_types.{klass_name}, PY3)"
    to_test = "PY3._to_python()"
    from_ctrl = f"to_py3_struct(py3_types.{klass_name}, PYTHON)"
    from_test = f"py3_types.{klass_name}.from_python(PYTHON)"

    setup_full = "\n".join([setup, "PYTHON=PY3._to_python()"])

    for stmt in (to_ctrl, to_test, from_ctrl, from_test):
        timer = timeit.Timer(
            stmt=stmt,
            setup=setup_full,
            globals={
                "python_types": python_types,
                "py3_types": py3_types,
                "to_python_struct": to_python_struct,
                "to_py3_struct": to_py3_struct,
            },
        )
        costs = []
        for _ in range(ctx.params["repeat"]):
            (n, total) = timer.autorange()
            costs.append(total * 1000.0 / n)
        cost_in_ms[title].append(min(costs))


@click.command()
@click.option(
    "--repeat",
    default=3,
    help="Repeat each benchmark N times and take the minimum.",
)
def main(repeat: int) -> None:
    benchmark(
        "Simple",
        setup="PY3=py3_types.Simple()",
    )
    benchmark(
        "Simple",
        qualifier="with large list",
        setup="PY3=py3_types.Simple(intList=[1] * 10_000)",
    )
    benchmark(
        "Simple",
        qualifier="with long string",
        setup="PY3=py3_types.Simple(strField='a' * 10_000_000)",
    )
    benchmark(
        "Nested",
        setup="PY3=py3_types.Nested()",
    )
    benchmark(
        "Nested",
        qualifier="with large list",
        setup="PY3=py3_types.Nested(simpleList=[py3_types.Simple()] * 1_000)",
    )
    benchmark(
        "Union",
        setup="PY3=py3_types.Union(intField=42)",
    )

    headers = [
        "thrift type",
        "to_python_struct",
        "capi Constructor",
        "to_py3_struct",
        "capi Extractor",
    ]
    table = [
        ([name] + [f"{cost:.3f} ms" for cost in costs])
        for (name, costs) in cost_in_ms.items()
    ]
    print(
        tabulate(
            table,
            headers=headers,
            tablefmt="github",
        )
    )


if __name__ == "__main__":
    main()
