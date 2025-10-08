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
from contextlib import contextmanager

from typing import Callable, TypeVar

from testing.thrift_types import easy, Nested1, Nested2, Nested3

from thrift.python.serializer import deserialize, serialize
from thrift.python.types import StructOrUnion


T = TypeVar("T", bound=StructOrUnion)


@contextmanager
# pyre-ignore[2, 3]:
def catch_thread_exception(*args, **kwds):
    """
    Context manager for threading.excepthook().
    See the threading.excepthook() documentation for details.
    """

    class ExceptionContext:
        def __init__(self):
            self.exc_type = None
            self.exc_value = None
            self.exc_traceback = None
            self.thread = None

    # pyre-ignore[2]:
    def scoped_excepthook(args, /) -> None:
        context.exc_type = args.exc_type
        context.exc_value = args.exc_value
        context.exc_traceback = args.exc_traceback
        context.thread = args.thread

    old_hook = threading.excepthook
    context: ExceptionContext = ExceptionContext()
    threading.excepthook = scoped_excepthook
    try:
        yield context
    finally:
        threading.excepthook = old_hook


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

    with catch_thread_exception() as cm:
        workers = [
            threading.Thread(target=wrapper_func, args=args, kwargs=kwargs)
            for _ in range(nthreads)
        ]

        for worker in workers:
            worker.start()

        for worker in workers:
            worker.join()

        if cm.exc_value is not None:
            raise cm.exc_value


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
