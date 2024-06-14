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


from __future__ import annotations

import timeit

from tabulate import tabulate


NAMESPACES = {
    "py-deprecated": "ttypes",
    "py3": "types",
    "python": "thrift_types",
    "mutable-python": "thrift_mutable_types",
}
ENUM_SIZES = [
    1000,
    2000,
    3000,
    5000,
    10000,
]

table = []


def benchmark(name: str, *, stmt: str, setup: str = "pass"):
    row = [name]
    for namespace in NAMESPACES.values():
        timer = timeit.Timer(
            stmt=stmt.format(namespace=namespace),
            setup=setup.format(namespace=namespace),
        )
        (n, total) = timer.autorange()
        row.append("{:.6f} ms".format(total * 1000.0 / n))
    return row


def benchmark_import():
    table = [
        benchmark(
            f"{enum_size}",
            stmt="importlib.reload(mm)",
            setup=f"import thrift.benchmark.enum_{enum_size}.{{namespace}} as mm; import importlib",
        )
        for enum_size in ENUM_SIZES
    ]
    print(
        tabulate(
            table,
            headers=["Import"] + list(NAMESPACES.keys()),
            tablefmt="github",
        )
    )


def benchmark_init():
    table = [
        benchmark(
            f"{enum_size}",
            stmt="n = Number.ZERO",
            setup=f"from thrift.benchmark.enum_{enum_size}.{{namespace}} import Number",
        )
        for enum_size in ENUM_SIZES
    ]
    print(
        tabulate(
            table,
            headers=["Init"] + list(NAMESPACES.keys()),
            tablefmt="github",
        )
    )


def benchmark_iterate():
    table = [
        benchmark(
            f"{enum_size}",
            stmt="[n for n in Number] if isinstance(Number, Iterable) else [n for n in Number._VALUES_TO_NAMES]",
            setup=f"from thrift.benchmark.enum_{enum_size}.{{namespace}} import Number; from typing import Iterable",
        )
        for enum_size in ENUM_SIZES
    ]
    print(
        tabulate(
            table,
            headers=["Iterate"] + list(NAMESPACES.keys()),
            tablefmt="github",
        )
    )


def main() -> None:
    benchmark_import()
    print()
    benchmark_init()
    print()
    benchmark_iterate()


if __name__ == "__main__":
    main()
