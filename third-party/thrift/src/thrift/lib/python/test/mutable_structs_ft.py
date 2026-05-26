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

from test_thrift.thrift_mutable_types import (
    easy,
    Integers,
    Nested1,
    Nested2,
    Nested3,
    OptionalColorGroups,
)
from thrift.lib.python.test.testing_utils import run_concurrently
from thrift.python.mutable_serializer import deserialize, serialize
from thrift.python.mutable_types import (
    MutableStructOrUnion,
    to_thrift_list,
    to_thrift_map,
    to_thrift_set,
)


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

    def test_locked_container_field_assignment_observes_assigned_values_in_lock_order(
        self,
    ) -> None:
        # GIVEN
        expected_snapshots = [
            ([1, 101], {1, 101}, {1: 101}),
            ([2, 102], {2, 102}, {2: 102}),
            ([3, 103], {3, 103}, {3: 103}),
            ([4, 104], {4, 104}, {4: 104}),
            ([5, 105], {5, 105}, {5: 105}),
            ([6, 106], {6, 106}, {6: 106}),
            ([7, 107], {7, 107}, {7: 107}),
            ([8, 108], {8, 108}, {8: 108}),
            ([9, 109], {9, 109}, {9: 109}),
            ([10, 110], {10, 110}, {10: 110}),
        ]
        expected_final_state = ([10, 110], {10, 110}, {10: 110})
        observed_snapshots: list[tuple[list[int], set[int], dict[int, int]]] = []
        color_groups = OptionalColorGroups(
            color_list=to_thrift_list([0]),
            color_set=to_thrift_set({0}),
            color_map=to_thrift_map({0: 0}),
        )
        lock: threading.Lock = threading.Lock()
        assigned_value = 0

        def worker(color_groups: OptionalColorGroups) -> None:
            nonlocal assigned_value
            with lock:
                assigned_value += 1
                value = assigned_value
                color_groups.color_list = to_thrift_list([value, value + 100])
                color_groups.color_set = to_thrift_set({value, value + 100})
                color_groups.color_map = to_thrift_map({value: value + 100})

                observed_list = color_groups.color_list
                observed_set = color_groups.color_set
                observed_map = color_groups.color_map
                if observed_list is None:
                    self.fail("Expected color_list to be set while lock is held")
                if observed_set is None:
                    self.fail("Expected color_set to be set while lock is held")
                if observed_map is None:
                    self.fail("Expected color_map to be set while lock is held")

                observed_snapshots.append(
                    (
                        list(observed_list),
                        set(observed_set),
                        dict(observed_map),
                    )
                )

        # WHEN
        run_concurrently(worker_func=worker, nthreads=10, args=(color_groups,))

        # THEN
        self.assertEqual(expected_snapshots, observed_snapshots)
        actual_final_state = (
            list(color_groups.color_list)
            if color_groups.color_list is not None
            else [],
            set(color_groups.color_set)
            if color_groups.color_set is not None
            else set(),
            dict(color_groups.color_map) if color_groups.color_map is not None else {},
        )
        self.assertEqual(expected_final_state, actual_final_state)

    def test_locked_container_field_failed_assignment_preserves_previous_values(
        self,
    ) -> None:
        # GIVEN
        expected_state = ([1, 2], {1, 2}, {1: 2})
        expected_exception_types = [TypeError, TypeError, TypeError] * 10
        color_groups = OptionalColorGroups(
            color_list=to_thrift_list([1, 2]),
            color_set=to_thrift_set({1, 2}),
            color_map=to_thrift_map({1: 2}),
        )
        lock: threading.Lock = threading.Lock()
        actual_exception_types: list[type[Exception]] = []

        def worker(color_groups: OptionalColorGroups) -> None:
            with lock:
                try:
                    color_groups.color_list = to_thrift_list([3, "Not an integer", 4])
                except TypeError as e:
                    actual_exception_types.append(type(e))
                try:
                    color_groups.color_set = to_thrift_set({3, "Not an integer", 4})
                except TypeError as e:
                    actual_exception_types.append(type(e))
                try:
                    color_groups.color_map = to_thrift_map({3: 4, 5: "Not an integer"})
                except TypeError as e:
                    actual_exception_types.append(type(e))

        # WHEN
        run_concurrently(worker_func=worker, nthreads=10, args=(color_groups,))

        # THEN
        self.assertEqual(expected_exception_types, actual_exception_types)
        actual_state = (
            list(color_groups.color_list)
            if color_groups.color_list is not None
            else [],
            set(color_groups.color_set)
            if color_groups.color_set is not None
            else set(),
            dict(color_groups.color_map) if color_groups.color_map is not None else {},
        )
        self.assertEqual(expected_state, actual_state)

    def test_pickle_same_struct_from_multiple_threads(self) -> None:
        def worker(s1: Nested1) -> None:
            s2 = pickle_roundtrip(s1)
            self.assertEqual(s1, s2)

        s1 = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        run_concurrently(worker_func=worker, nthreads=10, args=(s1,))
