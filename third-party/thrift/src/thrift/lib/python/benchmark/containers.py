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

import timeit
import typing
from enum import Enum

import click

from tabulate import tabulate


class Flavor(Enum):
    PYTHON = 0
    PY3 = 1
    PY_DEPRECATED = 2


def import_statement(flavor: Flavor) -> str:
    NAMESPACES = {
        Flavor.PYTHON: "thrift_types",
        Flavor.PY3: "types",
        Flavor.PY_DEPRECATED: "ttypes",
    }
    return (
        f"from thrift.benchmark.struct.{NAMESPACES[flavor]} import Included, MyStruct"
    )


@click.pass_context
def list_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"inst = MyStruct(val_list=list(range({size})))"


@click.pass_context
def set_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"inst = MyStruct(val_set=set(range({size})))"


@click.pass_context
def map_init_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
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


def serialze_statement(flavor: Flavor) -> str:
    return "data = serialize(inst)"


def deserialze_statement(flavor: Flavor) -> str:
    return "inst = deserialize(MyStruct, data)"


@click.pass_context
def list_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"_ = inst.val_list[{size//2}]"


@click.pass_context
def set_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"{size//2} in inst.val_set"


@click.pass_context
def map_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    return f"_ = inst.val_map[{size//2}]"


@click.pass_context
def nested_map_random_access_statement(ctx, flavor: Flavor) -> str:
    size = ctx.params["size"]
    inner_size = ctx.params["inner_size"]
    return f"_ = inst.val_map_structs[{size//2}].vals[{inner_size//2}]"


def list_iterate_statement(flavor: Flavor) -> str:
    return """
for _ in inst.val_list:
    pass
"""


def set_iterate_statement(flavor: Flavor) -> str:
    return """
for _ in inst.val_set:
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
def benchmark_list(ctx) -> None:
    size = ctx.params["size"]
    benchmark_container(
        f"List ({size} elements)",
        list_init_statement,
        list_random_access_statement,
        list_iterate_statement,
    )


@click.pass_context
def benchmark_set(ctx) -> None:
    size = ctx.params["size"]
    benchmark_container(
        f"Set ({size} elements)",
        set_init_statement,
        set_random_access_statement,
        set_iterate_statement,
    )


@click.pass_context
def benchmark_map(ctx) -> None:
    size = ctx.params["size"]
    benchmark_container(
        f"Map ({size} elements)",
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
    benchmark_list()
    benchmark_set()
    benchmark_map()
    benchmark_nested_map()
    return 0


if __name__ == "__main__":
    main()
