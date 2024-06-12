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

from __future__ import annotations

import timeit

import click

from tabulate import tabulate


NAMESPACES = {
    "py-deprecated": "ttypes",
    "py3": "types",
    "python": "thrift_types",
}

table = []


INIT_STATEMENT_MyStruct = """
inst = MyStruct(
    val_bool=True,
    val_i32=42,
    val_i64=64,
    val_string="hello world",
    val_binary=b"hello world",
    val_list={val_list},
    val_set={val_set},
    val_map={val_map},
)
"""


def get_import(flavor) -> str:
    return (
        f"from thrift.benchmark.struct.{NAMESPACES[flavor]} import MyStruct, Included"
    )


REPEAT = 10000
LOOP = 1


def benchmark_import():
    def benchmark_single(namespace) -> float:
        timer = timeit.Timer(
            stmt="importlib.reload(mm)",
            setup=f"import thrift.benchmark.struct.{namespace} as mm\nimport importlib",
        )
        results = timer.repeat(REPEAT, 1)
        min_value_ms = min(results) * 1000
        return f"{min_value_ms:.6f} ms"

    table = [
        [name, benchmark_single(namespace)] for name, namespace in NAMESPACES.items()
    ]
    print(
        tabulate(
            table,
            headers=["Import", ""],
            tablefmt="github",
        )
    )


def benchmark_init(init_statement: str, desc: str):
    def benchmark_single(flavor) -> float:
        timer = timeit.Timer(stmt=init_statement, setup=get_import(flavor))
        results = timer.repeat(REPEAT, 1)
        min_value_ms = min(results) * 1000
        return f"{min_value_ms:.6f} ms"

    table = [[flavor, benchmark_single(flavor)] for flavor in NAMESPACES]
    print(
        tabulate(
            table,
            headers=[f"Init ({desc})", ""],
            tablefmt="github",
        )
    )


def benchmark_field_access():
    def benchmark_single(flavor, field_name, cached) -> float:
        access = f"_ = inst.{field_name}"
        val_list = list(range(100))
        val_set = set(range(100))
        init_statement = INIT_STATEMENT_MyStruct.format(
            val_list=val_list, val_set=val_set, val_map={}
        )
        setup = f"{get_import(flavor)}; {init_statement}"
        if cached:
            setup = f"{setup}\n{access}"
        timer = timeit.Timer(
            stmt=access,
            setup=setup,
        )
        results = timer.repeat(REPEAT, LOOP)
        min_value_ms = min(results) * 1000 / LOOP
        return f"{min_value_ms:.6f} ms"

    fields = [
        "val_bool",
        "val_i32",
        "val_i64",
        "val_string",
        "val_binary",
    ]
    table = [
        [flavor]
        + [
            benchmark_single(
                flavor,
                field,
                False,
            )
            for field in fields
        ]
        for flavor in NAMESPACES
    ]
    print(
        tabulate(
            table,
            headers=["Field Access (First/Uncached)"] + fields,
            tablefmt="github",
        )
    )

    print("\n")

    table = [
        [flavor]
        + [
            benchmark_single(
                flavor,
                field,
                True,
            )
            for field in fields
        ]
        for flavor in NAMESPACES
    ]
    print(
        tabulate(
            table,
            headers=["Field Access (Repeated/Cached)"] + fields,
            tablefmt="github",
        )
    )


def benchmark_containers():
    INIT_FOR_CONTAINER = """
val_map = {
    i: f"str_{i}"
    for i in range(30)
}

val_map_structs = {
    k: Included(vals=[f"str_{i}_{k}" for i in range(10)])
    for k in range(30)
}
inst = MyStruct(
    val_list=list(range(30)),
    val_set=set(range(30)),
    val_map=val_map,
    val_map_structs=val_map_structs,
)
"""

    def benchmark_single(flavor, st, cached) -> float:
        setup = f"{get_import(flavor)}; {INIT_FOR_CONTAINER}"
        if cached:
            setup = f"{setup}\n{st}"
        timer = timeit.Timer(
            stmt=st,
            setup=setup,
        )
        results = timer.repeat(REPEAT, LOOP)
        min_value_ms = min(results) * 1000 / LOOP
        return f"{min_value_ms:.6f} ms"

    fields = {
        "list field iter": "_ = [item for item in inst.val_list]",
        "list field idx": "_ = inst.val_list[10]",
        "set field iter": "_ = [item for item in inst.val_set]",
        "set field lookup": "_ = 10 in inst.val_set",
        "map field lookup one": "_ = inst.val_map[10]",
        "map field lookup all": "_ = [inst.val_map[k] for k in range(30)]",
        "map struct list field": "_ = [string for incl in inst.val_map_structs.values() for string in incl.vals]",
    }

    table = [
        [flavor]
        + [
            benchmark_single(
                flavor,
                st,
                False,
            )
            for st in fields.values()
        ]
        for flavor in NAMESPACES
    ]
    print(
        tabulate(
            table,
            headers=["Container Field Access (First/Uncached)"] + list(fields.keys()),
            tablefmt="github",
        )
    )

    print("\n")

    table = [
        [flavor]
        + [
            benchmark_single(
                flavor,
                st,
                True,
            )
            for st in fields.values()
        ]
        for flavor in NAMESPACES
    ]
    print(
        tabulate(
            table,
            headers=["Container Field Access (Repeated/Cached)"] + list(fields.keys()),
            tablefmt="github",
        )
    )


SERIALIZER_IMPORT = {
    "py-deprecated": """
from thrift.util.Serializer import deserialize as d, serialize as s
from thrift.protocol.TCompactProtocol import TCompactProtocolFactory

def serialize(inst):
    return s(
        TCompactProtocolFactory(),
        inst,
    )

def deserialize(klass, bytes):
    return d(
        TCompactProtocolFactory(),
        bytes,
        klass()
    )
""",
    "py3": """
from thrift.py3.serializer import deserialize, serialize
""",
    "python": """
from thrift.python.serializer import deserialize, serialize
""",
}


def get_serialize_setup(flavor: str) -> str:
    val_list = list(range(100))
    val_set = set(range(100))
    init_statement = INIT_STATEMENT_MyStruct.format(
        val_list=val_list, val_set=val_set, val_map={}
    )
    return f"{get_import(flavor)}\n{init_statement}\n{SERIALIZER_IMPORT[flavor]}"


def get_deserialize_setup(flavor: str) -> str:
    return f"{get_serialize_setup(flavor)}\nserialized = serialize(inst)"


def benchmark_serializer():
    def benchmark_serialize(flavor: str) -> float:
        timer = timeit.Timer(
            setup=get_serialize_setup(flavor),
            stmt="_ = serialize(inst)",
        )
        results = timer.repeat(REPEAT, 1)
        min_value_ms = min(results) * 1000
        return f"{min_value_ms:.6f} ms"

    def benchmark_deserialize(flavor: str) -> float:
        timer = timeit.Timer(
            setup=get_deserialize_setup(flavor),
            stmt="_ = deserialize(MyStruct, serialized)",
        )
        results = timer.repeat(REPEAT, 1)
        min_value_ms = min(results) * 1000
        return f"{min_value_ms:.6f} ms"

    table = [
        [
            flavor,
            benchmark_serialize(
                flavor,
            ),
            benchmark_deserialize(
                flavor,
            ),
        ]
        for flavor in NAMESPACES
    ]
    print(
        tabulate(
            table,
            headers=["Serializer", "serialize", "deserialize"],
            tablefmt="github",
        )
    )


@click.group()
def cli():
    pass


@click.command()
def import_benchmark() -> None:
    benchmark_import()


@click.command()
def init_benchmark() -> None:
    for size in [1, 100, 1000]:
        val_list = list(range(size))
        val_set = set(range(size))
        init_statement = INIT_STATEMENT_MyStruct.format(
            val_list=val_list, val_set=val_set, val_map={}
        )
        benchmark_init(
            init_statement, f"MyStruct with {size} elements for set and list"
        )
        print("\n")

    for size in [1, 100, 1000]:
        val_map = {i: f"str_{i}" for i in range(size)}
        init_statement = INIT_STATEMENT_MyStruct.format(
            val_list=[], val_set=[], val_map=val_map
        )
        benchmark_init(init_statement, f"MyStruct with {size} elements for map")
        print("\n")


@click.command()
def field_access_benchmark() -> None:
    benchmark_field_access()


@click.command()
def container_benchmark() -> None:
    benchmark_containers()


@click.command()
def serializer_benchmark() -> None:
    benchmark_serializer()


@click.command()
@click.pass_context
def run_all(ctx) -> None:
    ctx.invoke(import_benchmark)
    print("\n")
    ctx.invoke(init_benchmark)
    print("\n")
    ctx.invoke(field_access_benchmark)
    print("\n")
    ctx.invoke(container_benchmark)
    print("\n")
    ctx.invoke(serializer_benchmark)


def main() -> None:
    cli.add_command(run_all)
    cli.add_command(import_benchmark)
    cli.add_command(init_benchmark)
    cli.add_command(field_access_benchmark)
    cli.add_command(container_benchmark)
    cli.add_command(serializer_benchmark)
    cli()


if __name__ == "__main__":
    main()  # pragma: no cover
