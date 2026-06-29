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

# pyre-strict

from __future__ import annotations

import gc
import os
import time
from typing import Generator

import click
import memray
import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer
from thrift.benchmark.struct.thrift_enums import MyEnum
from thrift.benchmark.struct.thrift_mutable_types import (
    Included as IncludedMutable,
    MyStruct as MyStructMutable,
    StringBucket as StringBucketMutable,
)
from thrift.benchmark.struct.thrift_types import (
    Included as IncludedImmutable,
    MyStruct as MyStructImmutable,
    StringBucket as StringBucketImmutable,
)
from thrift.python.mutable_types import to_thrift_list, to_thrift_map, to_thrift_set
from thrift.python.protocol import Protocol


"""
USAGE:
$ buck2 run @fbcode//mode/opt fbcode//thrift/lib/python/benchmark:struct_memory -- immutable-initialize
$ buck2 run @fbcode//mode/opt fbcode//thrift/lib/python/benchmark:memray -- stats /tmp/thrift_memray_out.bin
📏 Total allocations:
        65698

📦 Total memory allocated:
        6.381MB

📊 Histogram of allocation size:
        min: 8.000B
        --------------------------------------------
        < 18.000B  :    16 ▇
        < 43.000B  : 13358 ▇▇▇▇▇▇▇▇▇▇▇
        < 100.000B : 32676 ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇
        < 233.000B : 17503 ▇▇▇▇▇▇▇▇▇▇▇▇▇▇
        < 543.000B :  2109 ▇▇
        < 1.232KB  :     7 ▇
        < 2.866KB  :     7 ▇
        < 6.665KB  :     7 ▇
        < 15.495KB :     8 ▇
        <=36.023KB :     7 ▇
        --------------------------------------------
        max: 36.023KB
...

Please see https://bloomberg.github.io/memray/
"""


def _create_immutable() -> tuple[
    IncludedImmutable, StringBucketImmutable, MyStructImmutable
]:
    included = IncludedImmutable(vals=["1", "2", "3", "4", "5"])
    string_bucket = StringBucketImmutable(
        one="1",
        two="2",
        three="3",
        four="4",
        five="5",
        six="6",
        seven="7",
        eight="8",
        nine="9",
        ten="10",
    )
    my_struct = MyStructImmutable(
        val_bool=True,
        val_i32=12332,
        val_i64=12364,
        val_string="str",
        val_list=[1001, 1002, 1003],
        str_list=["1", "2", "3"],
        val_map={1100: "1", 2100: "2", 3100: "3"},
        str_map={"1": "1", "2": "2", "3": "3"},
        val_set={1123, 2123, 3123},
        val_map_structs={1231: included, 1232: included},
        val_struct=string_bucket,
        val_enum=MyEnum.FOO,
    )

    return (included, string_bucket, my_struct)


def _create_mutable() -> tuple[IncludedMutable, StringBucketMutable, MyStructMutable]:
    included = IncludedMutable(vals=to_thrift_list(["1", "2", "3", "4", "5"]))
    string_bucket = StringBucketMutable(
        one="1",
        two="2",
        three="3",
        four="4",
        five="5",
        six="6",
        seven="7",
        eight="8",
        nine="9",
        ten="10",
    )
    my_struct = MyStructMutable(
        val_bool=True,
        val_i32=12332,
        val_i64=12364,
        val_string="str",
        val_list=to_thrift_list([1001, 1002, 1003]),
        str_list=to_thrift_list(["1", "2", "3"]),
        val_map=to_thrift_map({1100: "1", 2100: "2", 3100: "3"}),
        str_map=to_thrift_map({"1": "1", "2": "2", "3": "3"}),
        val_set=to_thrift_set({1123, 2123, 3123}),
        val_map_structs=to_thrift_map({1231: included, 1232: included}),
        val_struct=string_bucket,
        val_enum=MyEnum.FOO,
    )

    return (included, string_bucket, my_struct)


@click.group()
def cli() -> None:
    pass


@cli.command()
def immutable_default_initialize() -> None:
    lst = []
    for _ in range(1000):
        lst.append(IncludedImmutable())
        lst.append(MyStructImmutable())
        lst.append(StringBucketImmutable())


@cli.command()
def immutable_initialize() -> None:
    lst = []
    for _ in range(1000):
        included, string_bucket, my_struct = _create_immutable()
        lst.append(my_struct)
        lst.append(string_bucket)
        lst.append(included)


@cli.command()
@click.option("--count", default=1000, help="Number of (struct triples) to build")
def immutable_initialize_tracemalloc(count: int) -> None:
    # Deterministic retained-memory measurement (no memray needed): the delta in
    # tracemalloc's "current" between before/after building `count` struct triples
    # isolates the per-instance internal-data footprint - in particular the isset
    # `bytes` object that the isset-disabled layout drops. Compare base vs diff.
    import tracemalloc

    gc.collect()
    tracemalloc.start()
    before = tracemalloc.get_traced_memory()[0]
    lst = []
    for _ in range(count):
        included, string_bucket, my_struct = _create_immutable()
        lst.append(my_struct)
        lst.append(string_bucket)
        lst.append(included)
    after = tracemalloc.get_traced_memory()[0]
    tracemalloc.stop()
    n = count * 3
    print(
        f"immutable retained: {(after - before) / 1024.0:.1f} KiB for {n} structs "
        f"({(after - before) / n:.1f} B/struct)"
    )


@cli.command()
@click.option("--count", default=50, help="Number of Root trees to build")
def immutable_unique_tree_tracemalloc(count: int) -> None:
    # Memory counterpart to `unique-nested-benchmark`: build the fanout-10,
    # depth-3 tree of 1111 UNIQUE types (1 Root + 10 T + 100 M + 1000 L) so the
    # isset savings are measured on the same workload as init/deserialize, rather
    # than the `_create_immutable` triple. Reports both per-struct (over all 1111
    # nodes) and per-root (the whole tree) so the savings can be read either way.
    import tracemalloc

    import thrift.benchmark.unique_struct.thrift_types as U

    def build_root() -> object:
        leaves = [getattr(U, f"L{i}")(val=i, str_val="x") for i in range(1000)]
        mids = [
            getattr(U, f"M{i}")(**{f"f{j + 1}": leaves[i * 10 + j] for j in range(10)})
            for i in range(100)
        ]
        tops = [
            getattr(U, f"T{i}")(**{f"f{j + 1}": mids[i * 10 + j] for j in range(10)})
            for i in range(10)
        ]
        return U.Root(**{f"f{j + 1}": tops[j] for j in range(10)})

    gc.collect()
    tracemalloc.start()
    before = tracemalloc.get_traced_memory()[0]
    roots = [build_root() for _ in range(count)]
    after = tracemalloc.get_traced_memory()[0]
    tracemalloc.stop()
    nodes = count * 1111
    delta = after - before
    print(
        f"unique-tree retained: {delta / 1024.0:.1f} KiB for {count} roots "
        f"({nodes} structs); {delta / nodes:.1f} B/struct, "
        f"{delta / count:.1f} B/root"
    )
    assert len(roots) == count


@cli.command()
def mutable_default_initialize() -> None:
    lst = []
    for _ in range(1000):
        lst.append(IncludedMutable())
        lst.append(MyStructMutable())
        lst.append(StringBucketMutable())


@cli.command()
def mutable_initialize() -> None:
    lst = []
    for _ in range(1000):
        included, string_bucket, my_struct = _create_mutable()
        lst.append(my_struct)
        lst.append(string_bucket)
        lst.append(included)


def all_protocols() -> Generator[Protocol, None, None]:
    yield Protocol.COMPACT
    yield Protocol.BINARY
    yield Protocol.JSON


def immutable_deserialize_impl(leak: bool) -> None:
    lst = []
    _, _, my_struct = _create_immutable()
    for protocol in all_protocols():
        buf: bytes = immutable_serializer.serialize(my_struct, protocol=protocol)
        for _ in range(10000):
            my_struct = immutable_serializer.deserialize(
                MyStructImmutable, buf, protocol=protocol
            )
            if not leak:
                lst.append(my_struct)


@cli.command()
def immutable_deserialize() -> None:
    immutable_deserialize_impl(leak=False)


@cli.command()
def immutable_deserialize_leak() -> None:
    immutable_deserialize_impl(leak=True)


def mutable_deserialize_impl(leak: bool) -> None:
    lst = []
    _, _, my_struct = _create_mutable()
    for protocol in all_protocols():
        buf: bytes = mutable_serializer.serialize(my_struct, protocol=protocol)
        for _ in range(10000):
            my_struct = mutable_serializer.deserialize(
                MyStructMutable,
                buf,
                protocol=protocol,
            )
            if not leak:
                lst.append(my_struct)

    time.sleep(1)


@cli.command()
def mutable_deserialize() -> None:
    mutable_deserialize_impl(leak=False)


@cli.command()
def mutable_deserialize_leak() -> None:
    mutable_deserialize_impl(leak=True)


def immutable_string_set_impl(size: int) -> None:
    """
    Test memory usage for string set.
    """
    lst = []
    # Create structs with large string sets
    for _ in range(size):
        my_struct = MyStructImmutable(str_set={f"permission_{i}" for i in range(1000)})
        _ = my_struct.str_set
        lst.append(my_struct)


@cli.command()
@click.option("--size", default=1000, help="Number of structs to create")
def immutable_string_set(size: int) -> None:
    immutable_string_set_impl(size)


def immutable_map_impl(leak: bool) -> None:
    lst = []
    for _ in range(10000):
        my_struct = MyStructImmutable(
            val_map={1000 + i: str(i) for i in range(1000)},
            str_map={str(i): str(i) for i in range(1000)},
            int_map={i: i + 1 for i in range(1000)},
        )
        if not leak:
            lst.append(my_struct)
        for protocol in all_protocols():
            buf: bytes = immutable_serializer.serialize(my_struct, protocol=protocol)
            for _ in range(10):
                my_struct_s = immutable_serializer.deserialize(
                    MyStructImmutable, buf, protocol=protocol
                )
                if not leak:
                    lst.append(my_struct_s)

    time.sleep(1)
    gc.collect()


@cli.command()
def immutable_map() -> None:
    immutable_map_impl(leak=False)


@cli.command()
def immutable_map_leak() -> None:
    immutable_map_impl(leak=True)


def main() -> None:
    trace_python_allocators = "TRACE_PYMALLOC" in os.environ
    with memray.Tracker(
        destination=memray.FileDestination(
            path="/tmp/thrift_memray_out.bin", overwrite=True
        ),
        trace_python_allocators=trace_python_allocators,
        native_traces=True,
        follow_fork=True,
    ):
        cli()


if __name__ == "__main__":
    main()
