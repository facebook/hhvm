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

import copy
import pickle
import unittest
from typing import TypeVar

from testing.thrift_types import HardError
from thrift.lib.python.test.testing_utils import run_concurrently
from thrift.python.exceptions import GeneratedError
from thrift.python.serializer import deserialize, serialize


T = TypeVar("T", bound=GeneratedError)


def serialization_roundtrip(exception: T) -> T:
    data = serialize(exception)
    return deserialize(type(exception), data)


def pickle_roundtrip(exception: T) -> T:
    data = pickle.dumps(exception)
    return pickle.loads(data)


class FreeThreading_ExceptionTests(unittest.TestCase):
    def test_exception_init_and_serde(self) -> None:
        def worker() -> None:
            e1 = HardError(errortext="err", code=2)
            e2 = serialization_roundtrip(e1)
            self.assertEqual(e1, e2)

        run_concurrently(worker_func=worker, nthreads=10)

    def test_serde_same_exception_from_multiple_threads(self) -> None:
        def worker(e1: HardError) -> None:
            e2 = serialization_roundtrip(e1)
            self.assertEqual(e1, e2)

        e1 = HardError(errortext="err", code=2)
        run_concurrently(worker_func=worker, nthreads=10, args=(e1,))

    def test_copy_deepcopy(self) -> None:
        def worker(e1: HardError) -> None:
            e2 = copy.copy(e1)
            self.assertEqual(e1, e2)
            e3 = copy.deepcopy(e1)
            self.assertEqual(e1, e3)

        e1 = HardError(errortext="err", code=2)
        run_concurrently(worker_func=worker, nthreads=10, args=(e1,))

    def test_pickle_same_exception_from_multiple_threads(self) -> None:
        def worker(e1: HardError) -> None:
            e2 = pickle_roundtrip(e1)
            self.assertEqual(e1, e2)

        e1 = HardError(errortext="err", code=2)
        run_concurrently(worker_func=worker, nthreads=10, args=(e1,))
