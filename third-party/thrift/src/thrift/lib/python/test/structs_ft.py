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

import threading
import unittest

from typing import Callable, TypeVar

from testing.thrift_types import easy, Nested1, Nested2, Nested3

from thrift.python.serializer import deserialize, serialize
from thrift.python.types import StructOrUnion


T = TypeVar("T", bound=StructOrUnion)


def run_concurrently(
    worker_func: Callable[..., None],
    nthreads: int,
    # pyre-ignore[2]:
    args=(),
    # pyre-ignore[2]:
    kwargs={},  # noqa
) -> None:
    """
    Run the `worker_func` concurrently
    """
    barrier: threading.Barrier = threading.Barrier(nthreads)

    # pyre-ignore[2]: Parameters ...
    def wrapper_func(*args, **kwargs) -> None:
        # Wait for all threads to reach this point.
        barrier.wait()
        worker_func(*args, **kwargs)

    workers = [
        threading.Thread(target=wrapper_func, args=args, kwargs=kwargs)
        for _ in range(nthreads)
    ]

    for worker in workers:
        worker.start()

    for worker in workers:
        worker.join()


def serialization_roundtrip(struct: T) -> T:
    data = serialize(struct)
    return deserialize(type(struct), data)


class FreeThreading_StructTests(unittest.TestCase):
    def test_struct_init_and_serde(self) -> None:
        def worker() -> None:
            s1 = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
            s2 = serialization_roundtrip(s1)
            self.assertEqual(s1, s2)

        run_concurrently(worker_func=worker, nthreads=10)
