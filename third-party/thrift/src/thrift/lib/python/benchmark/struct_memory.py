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

import click
import memray
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


@click.group()
def cli() -> None:
    pass


@click.command()
def immutable_default_initialize() -> None:
    lst = []
    for _ in range(1000):
        lst.append(IncludedImmutable())
        lst.append(MyStructImmutable())
        lst.append(StringBucketImmutable())


@click.command()
def immutable_initialize() -> None:
    lst = []
    for _ in range(1000):
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
        lst.append(
            MyStructImmutable(
                val_bool=True,
                val_i32=32,
                val_i64=64,
                val_string="str",
                val_list=[1, 2, 3],
                str_list=["1", "2", "3"],
                val_map={1: "1", 2: "2", 3: "3"},
                str_map={"1": "1", "2": "2", "3": "3"},
                val_set={1, 2, 3},
                val_map_structs={1: included, 2: included},
                val_struct=string_bucket,
                val_enum=MyEnum.FOO,
            )
        )
        lst.append(string_bucket)
        lst.append(included)


@click.command()
def mutable_default_initialize() -> None:
    lst = []
    for _ in range(1000):
        lst.append(IncludedMutable())
        lst.append(MyStructMutable())
        lst.append(StringBucketMutable())


@click.command()
def mutable_initialize() -> None:
    lst = []
    for _ in range(1000):
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
        lst.append(
            MyStructMutable(
                val_bool=True,
                val_i32=32,
                val_i64=64,
                val_string="str",
                val_list=to_thrift_list([1, 2, 3]),
                str_list=to_thrift_list(["1", "2", "3"]),
                val_map=to_thrift_map({1: "1", 2: "2", 3: "3"}),
                str_map=to_thrift_map({"1": "1", "2": "2", "3": "3"}),
                val_set=to_thrift_set({1, 2, 3}),
                val_map_structs=to_thrift_map({1: included, 2: included}),
                val_struct=string_bucket,
                val_enum=MyEnum.FOO,
            )
        )
        lst.append(string_bucket)
        lst.append(included)


def main() -> None:
    with memray.Tracker(
        destination=memray.FileDestination(
            path="/tmp/thrift_memray_out.bin", overwrite=True
        ),
        trace_python_allocators=True,
        native_traces=True,
        follow_fork=True,
    ):
        cli.add_command(immutable_default_initialize)
        cli.add_command(immutable_initialize)
        cli.add_command(mutable_default_initialize)
        cli.add_command(mutable_initialize)
        cli()


if __name__ == "__main__":
    main()
