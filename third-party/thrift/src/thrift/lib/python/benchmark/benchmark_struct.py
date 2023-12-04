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

from tabulate import tabulate


NAMESPACES = {
    "py-deprecated": "ttypes",
    "py3": "types",
    "python": "thrift_types",
}

table = []

val_list = list(range(100))
val_set = set(range(100))


INIT_STATEMENT = f"""
inst = MyStruct(
    val_bool=True,
    val_i32=42,
    val_i64=64,
    val_string="hello world",
    val_binary=b"hello world",
    val_list={val_list},
    val_set={val_set},
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


def benchmark_init():
    def benchmark_single(flavor) -> float:
        timer = timeit.Timer(stmt=INIT_STATEMENT, setup=get_import(flavor))
        results = timer.repeat(REPEAT, 1)
        min_value_ms = min(results) * 1000
        return f"{min_value_ms:.6f} ms"

    table = [[flavor, benchmark_single(flavor)] for flavor in NAMESPACES]
    print(
        tabulate(
            table,
            headers=["Init", ""],
            tablefmt="github",
        )
    )


def benchmark_field_access():
    def benchmark_single(flavor, field_name, cached) -> float:
        access = f"_ = inst.{field_name}"
        setup = f"{get_import(flavor)}; {INIT_STATEMENT}"
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
    for i in range(100)
}

val_map_structs = {
    i: Included()
    for i in range(100)
}
val_map_structs[50] = Included(vals=[f"str_{i}" for i in range(100)])
inst = MyStruct(
    val_list=list(range(100)),
    val_set=set(range(100)),
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
        "list field": "_ = inst.val_list[50]",
        "set field": "_ = 50 in inst.val_set",
        "map field": "_ = inst.val_map[50]",
        "map struct list field": "_ = inst.val_map_structs[50].vals[50]",
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
    return f"{get_import(flavor)}\n{INIT_STATEMENT}\n{SERIALIZER_IMPORT[flavor]}"


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


def main() -> None:
    benchmark_import()
    print("\n")
    benchmark_init()
    print("\n")
    benchmark_field_access()
    print("\n")
    benchmark_containers()
    print("\n")
    benchmark_serializer()


if __name__ == "__main__":
    main()  # pragma: no cover
