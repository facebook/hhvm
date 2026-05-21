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
import threading
import unittest
from typing import TypeVar

from test_thrift.thrift_mutable_types import easy, Integers, Nested1, Nested2, Nested3
from thrift.lib.python.test.testing_utils import run_concurrently
from thrift.python.mutable_serializer import deserialize, serialize
from thrift.python.mutable_types import MutableStructOrUnion


T = TypeVar("T", bound=MutableStructOrUnion)


def serialization_roundtrip(struct: T) -> T:
    data = serialize(struct)
    return deserialize(type(struct), data)


def pickle_roundtrip(struct: T) -> T:
    data = pickle.dumps(struct)
    return pickle.loads(data)


class FreeThreading_MutableStructTests(unittest.TestCase):
    def test_struct_init_and_serde(self) -> None:
        def worker() -> None:
            s1 = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
            s2 = serialization_roundtrip(s1)
            self.assertEqual(s1, s2)

        run_concurrently(worker_func=worker, nthreads=10)

    def test_serde_same_struct_from_multiple_threads(self) -> None:
        def worker(s1: Nested1) -> None:
            s2 = serialization_roundtrip(s1)
            self.assertEqual(s1, s2)

        s1 = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        run_concurrently(worker_func=worker, nthreads=10, args=(s1,))

    def test_copy_deepcopy(self) -> None:
        def worker(s1: Nested1) -> None:
            s2 = copy.copy(s1)
            self.assertEqual(s1, s2)
            s3 = copy.deepcopy(s1)
            self.assertEqual(s1, s3)

        s1 = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        run_concurrently(worker_func=worker, nthreads=10, args=(s1,))

    def test_call_replace(self) -> None:
        def worker(s1: easy) -> None:
            tid = threading.get_ident() % 1000000000
            s2 = s1(val=tid, name=f"{tid}")
            self.assertEqual(s2.val, tid)
            self.assertEqual(s2.name, f"{tid}")

        s1 = easy(val=1, an_int=Integers(small=300), name="foo")
        run_concurrently(worker_func=worker, nthreads=10, args=(s1,))

    def test_locked_scalar_field_writes(self) -> None:
        # GIVEN
        nthreads = 10
        expected_value = 10
        s1 = easy(val=0, name="initial")
        lock: threading.Lock = threading.Lock()
        assigned_value = 0

        def worker(s1: easy) -> None:
            nonlocal assigned_value
            with lock:
                assigned_value += 1
                s1.val = assigned_value

        # WHEN
        run_concurrently(worker_func=worker, nthreads=nthreads, args=(s1,))

        # THEN
        self.assertEqual(expected_value, s1.val)

    def test_locked_scalar_field_write_read_observes_lock_acquisition_order(
        self,
    ) -> None:
        # GIVEN
        nthreads = 10
        expected_snapshots = [
            (1, "name-1"),
            (2, "name-2"),
            (3, "name-3"),
            (4, "name-4"),
            (5, "name-5"),
            (6, "name-6"),
            (7, "name-7"),
            (8, "name-8"),
            (9, "name-9"),
            (10, "name-10"),
        ]
        expected_final_snapshot = (10, "name-10")
        observed_snapshots: list[tuple[int, str]] = []
        s1 = easy(val=0, name="initial")
        lock: threading.Lock = threading.Lock()
        assigned_value = 0

        def worker(s1: easy) -> None:
            nonlocal assigned_value
            with lock:
                assigned_value += 1
                s1.val = assigned_value
                s1.name = f"name-{assigned_value}"
                observed_name = s1.name
                if observed_name is None:
                    self.fail("Expected name to be set while lock is held")
                observed_snapshots.append((s1.val, observed_name))

        # WHEN
        run_concurrently(worker_func=worker, nthreads=nthreads, args=(s1,))

        # THEN
        actual_final_name = s1.name
        if actual_final_name is None:
            self.fail("Expected final name to be set")
        actual_final_snapshot = (s1.val, actual_final_name)
        self.assertEqual(expected_snapshots, observed_snapshots)
        self.assertEqual(expected_final_snapshot, actual_final_snapshot)

    def test_pickle_same_struct_from_multiple_threads(self) -> None:
        def worker(s1: Nested1) -> None:
            s2 = pickle_roundtrip(s1)
            self.assertEqual(s1, s2)

        s1 = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        run_concurrently(worker_func=worker, nthreads=10, args=(s1,))
