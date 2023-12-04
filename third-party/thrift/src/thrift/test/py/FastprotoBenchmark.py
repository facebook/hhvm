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

from __future__ import absolute_import, division, print_function, unicode_literals

import gc
import os
import timeit
from multiprocessing import Process, Queue

import psutil
from thrift.protocol import fastproto, TBinaryProtocol, TCompactProtocol
from thrift.transport import TTransport

try:
    from guppy import hpy
except ImportError:
    hpy = None

from FastProto.ttypes import AStruct, OneOfEach

ooe = OneOfEach()
ooe.aBool = True
ooe.aByte = 1
ooe.anInteger16 = 234
ooe.anInteger32 = 2345678
ooe.anInteger64 = 23456789012345
ooe.aString = "This is my rifle" * 100
ooe.aDouble = 2.3456789012
ooe.aFloat = 12345.678
ooe.aList = [12, 34, 56, 78, 90, 100, 123, 456, 789]
ooe.aSet = set(["This", "is", "my", "rifle"])
ooe.aMap = {"What": 4, "a": 1, "wonderful": 9, "day": 3, "!": 1}
ooe.aStruct = AStruct(aString="isn't it?", anInteger=999)

trans = TTransport.TMemoryBuffer()
proto = TBinaryProtocol.TBinaryProtocol(trans)
ooe.write(proto)
binary_buf = trans.getvalue()

trans = TTransport.TMemoryBuffer()
proto = TCompactProtocol.TCompactProtocol(trans)
ooe.write(proto)
compact_buf = trans.getvalue()


class TDevNullTransport(TTransport.TTransportBase):
    def __init__(self):
        pass

    def isOpen(self):
        return True


iters = 1000000


def benchmark_fastproto():
    setup_write = """
from __main__ import ooe, TDevNullTransport
from FastProto.ttypes import OneOfEach
from thrift.protocol import fastproto

trans = TDevNullTransport()
def doWrite():
    buf = fastproto.encode(ooe, [OneOfEach, OneOfEach.thrift_spec, False],
        utf8strings=0, protoid={0})
    trans.write(buf)
"""
    print(
        "Fastproto binary write = {}".format(
            timeit.Timer("doWrite()", setup_write.format(0)).timeit(number=iters)
        )
    )
    print(
        "Fastproto compact write = {}".format(
            timeit.Timer("doWrite()", setup_write.format(2)).timeit(number=iters)
        )
    )

    setup_read = """
from __main__ import binary_buf, compact_buf
from FastProto.ttypes import OneOfEach
from thrift.protocol import fastproto
from thrift.transport import TTransport

def doReadBinary():
    trans = TTransport.TMemoryBuffer(binary_buf)
    ooe = OneOfEach()
    fastproto.decode(ooe, trans, [OneOfEach, OneOfEach.thrift_spec, False],
        utf8strings=0, protoid=0)

def doReadCompact():
    trans = TTransport.TMemoryBuffer(compact_buf)
    ooe = OneOfEach()
    fastproto.decode(ooe, trans, [OneOfEach, OneOfEach.thrift_spec, False],
        utf8strings=0, protoid=2)
"""
    print(
        "Fastproto binary read = {}".format(
            timeit.Timer("doReadBinary()", setup_read).timeit(number=iters)
        )
    )
    print(
        "Fastproto compact read = {}".format(
            timeit.Timer("doReadCompact()", setup_read).timeit(number=iters)
        )
    )


def fastproto_encode(q, protoid):
    hp = hpy()
    trans = TDevNullTransport()
    p = psutil.Process(os.getpid())

    global ooe
    before = hp.heap()
    for i in range(iters):
        buf = fastproto.encode(
            ooe,
            [OneOfEach, OneOfEach.thrift_spec, False],
            utf8strings=0,
            protoid=protoid,
        )
        trans.write(buf)
        if (i + 1) % 100000 == 0:
            q.put((i + 1, p.memory_info()))

    gc.collect()
    after = hp.heap()
    leftover = after - before
    q.put("Memory leftover in Python after {} times: {}".format(iters, leftover))


def fastproto_decode(q, protoid):
    hp = hpy()
    p = psutil.Process(os.getpid())

    before = hp.heap()
    for i in range(iters):
        trans = TTransport.TMemoryBuffer(binary_buf if protoid == 0 else compact_buf)
        ooe_local = OneOfEach()
        fastproto.decode(
            ooe_local,
            trans,
            [OneOfEach, OneOfEach.thrift_spec, False],
            utf8strings=0,
            protoid=protoid,
        )
        if (i + 1) % 100000 == 0:
            q.put((i + 1, p.memory_info()))

    gc.collect()
    after = hp.heap()
    leftover = after - before
    q.put("Memory leftover in Python after {} times: {}".format(iters, leftover))


def memory_usage_fastproto():
    q = Queue()
    for method in (fastproto_encode, fastproto_decode):
        print("Memory usage with {}:".format(method.__name__))
        for protoid in (0, 2):
            print("Binary" if protoid == 0 else "Compact")
            p = Process(target=method, args=(q, protoid))
            p.start()
            while True:
                ret = q.get()
                if isinstance(ret, tuple):
                    print("Memory info after {} times: {}".format(ret[0], ret[1]))
                else:
                    print(ret)
                    p.join()
                    break


def main() -> None:
    print("Starting Benchmarks")
    benchmark_fastproto()
    if hpy is not None:
        memory_usage_fastproto()


if __name__ == "__main__":
    main()  # pragma: no cover
