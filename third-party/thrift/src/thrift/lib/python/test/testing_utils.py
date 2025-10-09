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

from collections.abc import Sequence, Set
from contextlib import contextmanager
from typing import Callable


# This is a proxy for np.array. It's an allowable input type
# for a container, but it raises on `if Untruthy():`
class Untruthy(Set, Sequence):
    def __init__(self, limit: int) -> None:
        self.i: int = 0
        self.limit: int = limit

    def __len__(self) -> int:
        return self.limit

    # pyre-ignore[14]
    def __iter__(self) -> "Untruthy":
        self.i: int = 0
        return self

    def __contains__(self, x: object) -> bool:
        if not isinstance(x, int):
            return False
        return x >= 0 and x < self.limit

    def __getitem__(self, item: int) -> int:
        if item in self:
            return item
        raise IndexError

    def __next__(self) -> int:
        if self.i >= self.limit:
            raise StopIteration
        ret = self.i
        self.i += 1
        return ret

    def __bool__(self) -> bool:
        raise ValueError("Do not dare question my truth")


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
