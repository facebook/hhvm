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


from __future__ import annotations

import json
import timeit

import click
import convertible.thrift_types as python_types  # noqa: F401
import convertible.types as py3_types  # noqa: F401
from tabulate import tabulate
from thrift.py3.converter import to_py3_struct  # noqa: F401
from thrift.python.converter import to_python_struct  # noqa: F401


LANGS = ["python", "py3"]

base_line_cost = None
cost_in_ms = {}


def format_cell(name, src, dest):
    if src == dest:
        return None
    cost = cost_in_ms[name][src][dest]
    cell = "{:.3f} ms".format(cost)
    if base_line_cost:
        base = base_line_cost[name][src][dest]
        pct = (cost - base) * 100.0 / base
        cell += " ({:+.0f}%)".format(pct)
    return cell


@click.pass_context
def benchmark(ctx, name: str, *, stmt: str, setup: str):
    cost_in_ms[name] = {}
    for src in LANGS:
        cost_in_ms[name][src] = {}
        for dest in LANGS:
            if src != dest:
                timer = timeit.Timer(
                    stmt=stmt.format(src=src, dest=dest),
                    setup=setup.format(src=src, dest=dest),
                    globals={
                        "python_types": python_types,
                        "py3_types": py3_types,
                        "to_py3_struct": to_py3_struct,
                        "to_python_struct": to_python_struct,
                    },
                )
                costs = []
                for _ in range(ctx.params["repeat"]):
                    (n, total) = timer.autorange()
                    costs.append(total * 1000.0 / n)
                cost_in_ms[name][src][dest] = min(costs)

    headers = ["from\\to (ms)"] + LANGS
    table = [
        ([src] + [format_cell(name, src, dest) for dest in LANGS]) for src in LANGS
    ]
    print(f"# {name}")
    print("---")
    print(
        tabulate(
            table,
            headers=headers,
            tablefmt="github",
        )
    )
    print()


@click.command()
@click.option(
    "--base_line",
    default="",
    help="Load base line from this file, and compare against it.",
)
@click.option(
    "--dump",
    default="",
    help="Dump benchmark result to this file in JSON format.",
)
@click.option(
    "--repeat",
    default=3,
    help="Repeat each benchmark N times and take the minimum.",
)
def main(base_line, dump, repeat):
    if base_line:
        with open(base_line) as f:
            global base_line_cost
            base_line_cost = json.load(f)

    benchmark(
        "Simple",
        stmt="to_{dest}_struct({dest}_types.Simple, SRC)",
        setup="SRC={src}_types.Simple()",
    )
    benchmark(
        "Simple with large list",
        stmt="to_{dest}_struct({dest}_types.Simple, SRC)",
        setup="SRC={src}_types.Simple(intList=[1] * 10_000)",
    )
    benchmark(
        "Simple with long string",
        stmt="to_{dest}_struct({dest}_types.Simple, SRC)",
        setup="SRC={src}_types.Simple(strField='a' * 10_000_000)",
    )
    benchmark(
        "Nested",
        stmt="to_{dest}_struct({dest}_types.Nested, SRC)",
        setup="SRC={src}_types.Nested()",
    )
    benchmark(
        "Nested with large list",
        stmt="to_{dest}_struct({dest}_types.Nested, SRC)",
        setup="SRC={src}_types.Nested(simpleList=[{src}_types.Simple()] * 1_000)",
    )
    benchmark(
        "Union",
        stmt="to_{dest}_struct({dest}_types.Union, SRC)",
        setup="SRC={src}_types.Union(intField=42)",
    )

    if dump:
        with open(dump, mode="w") as f:
            f.write(json.dumps(cost_in_ms))


if __name__ == "__main__":
    main()
