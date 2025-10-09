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

from testing.thrift_types import ComplexUnion, Digits, Integers
from thrift.lib.python.test.testing_utils import run_concurrently

from thrift.python.serializer import deserialize, serialize
from thrift.python.types import StructOrUnion


T = TypeVar("T", bound=StructOrUnion)


def serialization_roundtrip(union: T) -> T:
    data = serialize(union)
    return deserialize(type(union), data)


def pickle_roundtrip(union: T) -> T:
    data = pickle.dumps(union)
    return pickle.loads(data)


class FreeThreading_UnionTests(unittest.TestCase):
    def test_union_init_and_serde(self) -> None:
        def worker() -> None:
            u1 = ComplexUnion(tiny=1)
            u2 = serialization_roundtrip(u1)
            self.assertEqual(u1, u2)

        run_concurrently(worker_func=worker, nthreads=10)

    def test_serde_same_union_from_multiple_threads(self) -> None:
        def worker(u1: Digits) -> None:
            u2 = serialization_roundtrip(u1)
            self.assertEqual(u1, u2)

        u1 = Digits(data=[Integers(tiny=1), Integers(unbounded="123")])
        run_concurrently(worker_func=worker, nthreads=10, args=(u1,))

    def test_copy_deepcopy(self) -> None:
        def worker(u1: Digits) -> None:
            u2 = copy.copy(u1)
            self.assertEqual(u1, u2)
            u3 = copy.deepcopy(u1)
            self.assertEqual(u1, u3)

        u1 = Digits(data=[Integers(tiny=1), Integers(unbounded="123")])
        run_concurrently(worker_func=worker, nthreads=10, args=(u1,))

    def test_pickle_same_union_from_multiple_threads(self) -> None:
        def worker(u1: Digits) -> None:
            u2 = pickle_roundtrip(u1)
            self.assertEqual(u1, u2)

        u1 = Digits(data=[Integers(tiny=1), Integers(unbounded="123")])
        run_concurrently(worker_func=worker, nthreads=10, args=(u1,))
