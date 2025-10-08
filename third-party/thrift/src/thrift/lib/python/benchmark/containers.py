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
import typing
from enum import Enum

import click

from tabulate import tabulate


class Flavor(Enum):
    PYTHON = 0
    PY3 = 1
    PY_DEPRECATED = 2
    PYTHON_FULLY_POPULATE_CACHE = 3
    MUTABLE_PYTHON = 4


def import_statement(flavor: Flavor) -> str:
    NAMESPACES = {
        Flavor.PYTHON: "thrift_types",
        Flavor.PY3: "types",
        Flavor.PY_DEPRECATED: "ttypes",
        Flavor.PYTHON_FULLY_POPULATE_CACHE: "thrift_types",
        Flavor.MUTABLE_PYTHON: "thrift_mutable_types",
    }
    return (
        f"from thrift.benchmark.struct.{NAMESPACES[flavor]} import Included, MyStruct"
    )


def import_container_wrappers(flavor: Flavor) -> str:
    if flavor == Flavor.MUTABLE_PYTHON:
        return "from thrift.python.mutable_types import to_thrift_list, to_thrift_set, to_thrift_map"

    return ""


@click.pass_context
def int_list_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    if flavor == Flavor.MUTABLE_PYTHON:
        return f"inst = MyStruct(val_list=to_thrift_list(list(range({size}))))"

    return f"inst = MyStruct(val_list=list(range({size})))"


@click.pass_context
def str_list_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    if flavor == Flavor.MUTABLE_PYTHON:
        return (
            f"inst = MyStruct(str_list=to_thrift_list([str(i) for i in range({size})]))"
        )

    return f"inst = MyStruct(str_list=[str(i) for i in range({size})])"


@click.pass_context
def int_set_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    if flavor == Flavor.MUTABLE_PYTHON:
        return f"inst = MyStruct(val_set=to_thrift_set(set(range({size}))))"

    return f"inst = MyStruct(val_set=set(range({size})))"


@click.pass_context
def str_set_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    if flavor == Flavor.MUTABLE_PYTHON:
        return (
            f"inst = MyStruct(str_set=to_thrift_set([str(i) for i in range({size})]))"
        )

    return f"inst = MyStruct(str_set=[str(i) for i in range({size})])"


@click.pass_context
def map_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    if flavor == Flavor.MUTABLE_PYTHON:
        return f"""
inst = MyStruct(val_map=to_thrift_map({{
    i: f\"str_{{i}}"
    for i in range({size})
}}))
"""
    return f"""
inst = MyStruct(val_map={{
    i: f\"str_{{i}}"
    for i in range({size})
}})
"""


@click.pass_context
def nested_map_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    inner_size = ctx.params["inner_size"]
    if flavor == Flavor.MUTABLE_PYTHON:
        return f"""
inst = MyStruct(
    val_map_structs = to_thrift_map({{
        i: Included(
            vals=to_thrift_list([
                f\"str_{{i}}_{{j}}"
                for j in range({inner_size})
            ])
        )
        for i in range({size})
    }})
)
"""

    return f"""
inst = MyStruct(
    val_map_structs = {{
        i: Included(
            vals=[
                f\"str_{{i}}_{{j}}"
                for j in range({inner_size})
            ]
        )
        for i in range({size})
    }}
)
"""


# pyre-fixme[7]: Expected `str` but got implicit return value of `None`.
def import_serializer_statement(flavor: Flavor) -> str:
    if flavor == Flavor.PYTHON:
        return "from thrift.python.serializer import deserialize, serialize"
    elif flavor == Flavor.PY3:
        return "from thrift.py3.serializer import deserialize, serialize"
    elif flavor == Flavor.PY_DEPRECATED:
        return """
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
"""
    elif flavor == Flavor.PYTHON_FULLY_POPULATE_CACHE:
        return """
from thrift.python.serializer import deserialize as d, serialize

def deserialize(klass, bytes):
    return d(klass, bytes, fully_populate_cache=True)
"""
    elif flavor == Flavor.MUTABLE_PYTHON:
        return "from thrift.python.mutable_serializer import deserialize, serialize"


def serialze_statement(flavor: Flavor) -> str:
    return "data = serialize(inst)"


def deserialze_statement(flavor: Flavor) -> str:
    return "inst = deserialize(MyStruct, data)"


@click.pass_context
def int_list_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"_ = inst.val_list[{size//2}]"


@click.pass_context
def str_list_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"_ = inst.str_list[{size//2}]"


@click.pass_context
def int_set_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"{size//2} in inst.val_set"


@click.pass_context
def str_set_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"{size//2} in inst.str_set"


@click.pass_context
def map_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"_ = inst.val_map[{size//2}]"


@click.pass_context
def nested_map_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    inner_size = ctx.params["inner_size"]
    return f"_ = inst.val_map_structs[{size//2}].vals[{inner_size//2}]"


def int_list_iterate_statement(flavor: Flavor) -> str:
    return """
for _ in inst.val_list:
    pass
"""


def str_list_iterate_statement(flavor: Flavor) -> str:
    return """
for _ in inst.str_list:
    pass
"""


def int_set_iterate_statement(flavor: Flavor) -> str:
    return """
for _ in inst.val_set:
    pass
"""


def str_set_iterate_statement(flavor: Flavor) -> str:
    return """
for _ in inst.str_set:
    pass
"""


def map_iterate_statement(flavor: Flavor) -> str:
    return """
for _, _ in inst.val_map.items():
    pass
"""


def nested_map_iterate_statement(flavor: Flavor) -> str:
    return """
for _, v in inst.val_map_structs.items():
    for _ in v.vals:
        pass
"""


@click.pass_context
def benchmark_steps(ctx, *statements: typing.List[str]) -> typing.List[str]:
    result = []
    setup = ""
    for stmt in statements:
        timer = timeit.Timer(
            # pyre-fixme[6]: For 1st argument expected `Union[typing.Callable[[],
            #  object], str]` but got `List[str]`.
            stmt=stmt,
            setup=setup,
        )
        costs = timer.repeat(ctx.params["repeat"], 1)
        min_cost_in_ms = min(costs) * 1000
        result.append(min_cost_in_ms)
        setup = f"{setup}\n{stmt}"
    total = sum(result)
    result.append(total)
    return [f"{t:.6f} ms" for t in result]


def flavor_name(flavor: Flavor) -> str:
    return flavor.name.lower()


def benchmark_container(
    title: str,
    init_statement_func,
    random_access_statement_func,
    iterate_statement_func,
) -> None:
    print(f"# {title}\n---")
    headers = [
        title,
        "Import Types",
        "Import Container Wrappers",
        "Construct",
        "Import Serializer",
        "Serialize",
        "Deserialize",
        "Random Access",
        "Iterate",
        "Total",
    ]
    table = [
        [flavor_name(flavor)]
        + benchmark_steps(
            import_statement(flavor),
            import_container_wrappers(flavor),
            init_statement_func(flavor),
            import_serializer_statement(flavor),
            serialze_statement(flavor),
            deserialze_statement(flavor),
            random_access_statement_func(flavor),
            iterate_statement_func(flavor),
        )
        for flavor in Flavor
    ]
    print(
        tabulate(
            table,
            headers=headers,
            tablefmt="github",
        )
    )
    print("\n")


@click.pass_context
def benchmark_int_list(ctx) -> None:
    size = ctx.params["size"]
    benchmark_container(
        f"List<i64> ({size} elements)",
        int_list_init_statement,
        int_list_random_access_statement,
        int_list_iterate_statement,
    )


@click.pass_context
def benchmark_str_list(ctx) -> None:
    size = ctx.params["size"]
    benchmark_container(
        f"List<string> ({size} elements)",
        str_list_init_statement,
        str_list_random_access_statement,
        str_list_iterate_statement,
    )


@click.pass_context
def benchmark_int_set(ctx) -> None:
    size = ctx.params["size"]
    benchmark_container(
        f"Set<i32> ({size} elements)",
        int_set_init_statement,
        int_set_random_access_statement,
        int_set_iterate_statement,
    )


@click.pass_context
def benchmark_str_set(ctx) -> None:
    size = ctx.params["size"]
    benchmark_container(
        f"Set<string> ({size} elements)",
        str_set_init_statement,
        str_set_random_access_statement,
        str_set_iterate_statement,
    )


@click.pass_context
def benchmark_map(ctx) -> None:
    size = ctx.params["size"]
    benchmark_container(
        f"Map<i32, string> ({size} elements)",
        map_init_statement,
        map_random_access_statement,
        map_iterate_statement,
    )


@click.pass_context
def benchmark_nested_map(ctx) -> None:
    size = ctx.params["size"]
    inner_size = ctx.params["inner_size"]
    benchmark_container(
        f"Nested Map ({size} * {inner_size})",
        nested_map_init_statement,
        nested_map_random_access_statement,
        nested_map_iterate_statement,
    )


@click.command()
@click.option(
    "--size",
    default=1000,
    help="Size of containers.",
)
@click.option(
    "--inner_size",
    default=10,
    help="Size of inner containers when nested.",
)
@click.option(
    "--repeat",
    default=10,
    help="Repeat each benchmark N times and take the minimum.",
)
def main(size, inner_size, repeat) -> int:
    benchmark_int_list()
    benchmark_str_list()
    benchmark_int_set()
    benchmark_str_set()
    benchmark_map()
    benchmark_nested_map()
    return 0


if __name__ == "__main__":
    main()
