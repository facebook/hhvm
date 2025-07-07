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

# pyre-unsafe

import unittest
from pickle import loads

import pickle_thrift.pickle_thrift.thrift_types as python_types
import pickle_thrift.pickle_thrift.types as py3_types
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import is_auto_migrated


class ThriftPicklingTest(unittest.TestCase):
    python_pickled_data = b"\x80\x04\x95v\x00\x00\x00\x00\x00\x00\x00\x8c\x13thrift.python.types\x94\x8c\x10_unpickle_struct\x94\x93\x94\x8c(pickle_thrift.pickle_thrift.thrift_types\x94\x8c\x0beasy_pickle\x94\x93\x94C\x08\x15T(\x03foo\x00\x94\x86\x94R\x94."
    py3_pickled_data = b"\x80\x04\x95m\x00\x00\x00\x00\x00\x00\x00\x8c\x15thrift.py3.serializer\x94\x8c\x0bdeserialize\x94\x93\x94\x8c!pickle_thrift.pickle_thrift.types\x94\x8c\x0beasy_pickle\x94\x93\x94C\t8\x03foo\x05\x02T\x00\x94\x86\x94R\x94."

    def test_pickled_python_struct_load(self) -> None:
        loaded = loads(self.python_pickled_data)
        expected_ending = ".thrift_types"
        self.assertTrue(
            loaded.__class__.__module__.endswith(expected_ending),
            f"This code is auto_migrated={is_auto_migrated()}, the data was python but the module is {loaded.__class__.__module__}",
        )

    def test_pickled_py3_struct_load(self) -> None:
        loaded = loads(self.py3_pickled_data)
        expected_ending = ".thrift_types" if is_auto_migrated() else ".types"
        self.assertTrue(
            loaded.__class__.__module__.endswith(expected_ending),
            f"This code is auto_migrated={is_auto_migrated()}, the data was py3 but the module is {loaded.__class__.__module__}",
        )

    def test_pickled_py3_struct_load_and_nest_in_py3(self) -> None:
        loaded = loads(self.py3_pickled_data)
        py3_types.nested_pickle(c=loaded)

    def test_pickled_python_struct_load_and_nest_in_py3_struct(self) -> None:
        # pickled data is python, and loading it into a python struct
        loaded = loads(self.python_pickled_data)

        # Now testing __init__ function of py3 struct by passing the python struct
        py3_struct1 = py3_types.nested_pickle(c=loaded)

        # Now testing __call__ function of py3 struct by passing the python struct
        py3_struct2 = py3_struct1(c=loaded)
        self.assertEqual(py3_struct2.c.val, 42)

    def test_pickled_python_struct_load_and_nest_in_py3_union(self) -> None:
        # pickled data is python, and loading it into a python struct
        loaded = loads(self.python_pickled_data)

        # Now testing __init__ function of py3 union by passing the python struct
        py3_union1 = py3_types.nested_pickle_union(c=loaded)

        self.assertEqual(py3_union1.type, py3_union1.Type.c)
        self.assertEqual(py3_union1.c.val, 42)

        # Now testing struct containers of py3 struct by passing the python struct
        py3_struct3 = py3_types.struct_container(
            list_easy=[loaded], map_easy_easy={loaded: loaded}, set_easy={loaded}
        )
        self.assertEqual(py3_struct3.list_easy[0].val, 42)
        self.assertEqual(len(py3_struct3.set_easy), 1)
        self.assertEqual(py3_struct3.map_easy_easy[loaded._to_py3()].val, 42)

        # Now testing union containers of py3 struct by passing the python struct
        py3_union1 = py3_types.union_container(list_easy=[loaded])
        self.assertEqual(py3_union1.type, py3_union1.Type.list_easy)
        self.assertEqual(py3_union1.list_easy[0].val, 42)

        py3_union2 = py3_types.union_container(map_easy_easy={loaded: loaded})
        self.assertEqual(py3_union2.type, py3_union1.Type.map_easy_easy)
        self.assertEqual(py3_union2.map_easy_easy[loaded._to_py3()].val, 42)

        py3_union3 = py3_types.union_container(set_easy={loaded})
        self.assertEqual(py3_union3.type, py3_union1.Type.set_easy)
        self.assertEqual(len(py3_union3.set_easy), 1)

        # Now testing exception containers of py3 struct by passing the python struct
        py3_exception1 = py3_types.exception_container(
            list_easy=[loaded], map_easy_easy={loaded: loaded}, set_easy={loaded}
        )
        self.assertEqual(py3_exception1.list_easy[0].val, 42)
        self.assertEqual(len(py3_exception1.set_easy), 1)
        self.assertEqual(py3_exception1.map_easy_easy[loaded._to_py3()].val, 42)

    def test_pickled_py3_struct_load_and_nest_in_python(self) -> None:
        loaded = loads(self.py3_pickled_data)
        python_types.nested_pickle(c=loaded)
        if is_auto_migrated():
            self.assertIsInstance(loaded, python_types.easy_pickle)
        else:
            self.assertNotIsInstance(loaded, python_types.easy_pickle)

    def test_pickled_python_struct_load_and_nest_in_python(self) -> None:
        loaded = loads(self.python_pickled_data)
        python_types.nested_pickle(c=loaded)

    def test_struct_container(self) -> None:
        py3_struct = py3_types.struct_container(
            # pyre-ignore[6]:expected `Optional[Sequence[py3_types.easy_pickle]]`
            # but got `List[Union[py3_types.easy_pickle, python_types.easy_pickle]]`
            list_easy=[py3_types.easy_pickle(val=18), python_types.easy_pickle(val=19)],
            # pyre-ignore[6]:expected `Optional[AbstractSet[py3_types.easy_pickle]]`
            # but got `Set[Union[py3_types.easy_pickle, python_types.easy_pickle]]`
            set_easy={py3_types.easy_pickle(), python_types.easy_pickle()},
            # pyre-ignore[6]:expected `Optional[Mapping[py3_types.easy_pickle, py3_types.easy_pickle]]`
            # but got `Dict[Union[py3_types.easy_pickle, python_types.easy_pickle], Union[py3_types.easy_pickle, python_types.easy_pickle]]`
            map_easy_easy={
                py3_types.easy_pickle(): python_types.easy_pickle(),
                python_types.easy_pickle(): py3_types.easy_pickle(),
            },
            # pyre-ignore[6]:expected `Optional[Sequence[Sequence[py3_types.easy_pickle]]]`
            # but got `List[Union[List[py3_types.easy_pickle], List[python_types.easy_pickle]]]`
            list_list_easy=[
                [py3_types.easy_pickle(val=1)],
                [python_types.easy_pickle(val=2)],
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[AbstractSet[py3_types.easy_pickle]]]`
            # but got `List[Union[Set[py3_types.easy_pickle], Set[python_types.easy_pickle]]]`
            list_set_easy=[
                {py3_types.easy_pickle(val=1)},
                {python_types.easy_pickle(val=2)},
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[Mapping[py3_types.easy_pickle, py3_types.easy_pickle]]]`
            # but got `List[Union[Dict[py3_types.easy_pickle, python_types.easy_pickle], Dict[python_types.easy_pickle, py3_types.easy_pickle]]]`
            list_map_easy_easy=[
                {py3_types.easy_pickle(val=1): python_types.easy_pickle(val=2)},
                {python_types.easy_pickle(val=2): py3_types.easy_pickle(val=1)},
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[Mapping[str, py3_types.easy_pickle]]]`
            # but got `List[Union[Dict[str, py3_types.easy_pickle], Dict[str, python_types.easy_pickle]]]`
            list_map_str_easy=[
                {"1": python_types.easy_pickle(val=2)},
                {"2": py3_types.easy_pickle(val=1)},
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[Mapping[py3_types.easy_pickle, str]]]`
            # but got `List[Union[Dict[py3_types.easy_pickle, str], Dict[python_types.easy_pickle, str]]]`
            list_map_easy_str=[
                {py3_types.easy_pickle(val=1): "1"},
                {python_types.easy_pickle(val=2): "2"},
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[Mapping[py3_types.easy_pickle, Sequence[str]]]]`
            # but got `List[Union[Dict[py3_types.easy_pickle, List[str]], Dict[python_types.easy_pickle, List[str]]]]
            list_map_easy_list_str=[
                {py3_types.easy_pickle(val=1): ["1", "2"]},
                {python_types.easy_pickle(val=2): ["2", "1"]},
            ],
        )

        python_struct = python_types.struct_container(
            # pyre-ignore[6]:expected `Optional[Sequence[python_types.easy_pickle]]`
            # but got `List[Union[py3_types.easy_pickle, python_types.easy_pickle]]`
            list_easy=[py3_types.easy_pickle(val=18), python_types.easy_pickle(val=19)],
            # pyre-ignore[6]:expected `Optional[AbstractSet[python_types.easy_pickle]]`
            # but got `Set[Union[py3_types.easy_pickle, python_types.easy_pickle]]`
            set_easy={py3_types.easy_pickle(), python_types.easy_pickle()},
            # pyre-ignore[6]:expected `Optional[Mapping[python_types.easy_pickle, python_types.easy_pickle]]`
            # but got `Dict[Union[py3_types.easy_pickle, python_types.easy_pickle], Union[py3_types.easy_pickle, python_types.easy_pickle]]`
            map_easy_easy={
                py3_types.easy_pickle(): python_types.easy_pickle(),
                python_types.easy_pickle(): py3_types.easy_pickle(),
            },
            # pyre-ignore[6]:expected `Optional[Sequence[Sequence[python_types.easy_pickle]]]`
            # but got `List[Union[List[py3_types.easy_pickle], List[python_types.easy_pickle]]]`
            list_list_easy=[
                [py3_types.easy_pickle(val=1)],
                [python_types.easy_pickle(val=2)],
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[AbstractSet[python_types.easy_pickle]]]`
            # but got `List[Union[Set[py3_types.easy_pickle], Set[python_types.easy_pickle]]]`
            list_set_easy=[
                {py3_types.easy_pickle(val=1)},
                {python_types.easy_pickle(val=2)},
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[Mapping[python_types.easy_pickle, python_types.easy_pickle]]]`
            # but got `List[Union[Dict[py3_types.easy_pickle, python_types.easy_pickle], Dict[python_types.easy_pickle, py3_types.easy_pickle]]]`
            list_map_easy_easy=[
                {py3_types.easy_pickle(val=1): python_types.easy_pickle(val=2)},
                {python_types.easy_pickle(val=2): py3_types.easy_pickle(val=1)},
            ],
            # pyre-ignore[6]: expected `Optional[Sequence[Mapping[str, python_types.easy_pickle]]]`
            # but got `List[Union[Dict[str, py3_types.easy_pickle], Dict[str, python_types.easy_pickle]]]`
            list_map_str_easy=[
                {"1": python_types.easy_pickle(val=2)},
                {"2": py3_types.easy_pickle(val=1)},
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[Mapping[python_types.easy_pickle, str]]]`
            # but got `List[Union[Dict[py3_types.easy_pickle, str], Dict[python_types.easy_pickle, str]]]`
            list_map_easy_str=[
                {py3_types.easy_pickle(val=1): "1"},
                {python_types.easy_pickle(val=2): "2"},
            ],
            # pyre-ignore[6]:expected `Optional[Sequence[Mapping[python_types.easy_pickle, Sequence[str]]]]`
            # but got `List[Union[Dict[py3_types.easy_pickle, List[str]], Dict[python_types.easy_pickle, List[str]]]]`
            list_map_easy_list_str=[
                {py3_types.easy_pickle(val=1): ["1", "2"]},
                {python_types.easy_pickle(val=2): ["2", "1"]},
            ],
        )
        self.assertIsInstance(py3_struct.list_easy[0], py3_types.easy_pickle)
        for key, value in py3_struct.map_easy_easy.items():
            self.assertIsInstance(key, py3_types.easy_pickle)
            self.assertIsInstance(value, py3_types.easy_pickle)
        self.assertTrue(
            all(isinstance(ele, py3_types.easy_pickle) for ele in py3_struct.set_easy)
        )
        self.assertEqual(py3_struct.list_easy[0].val, python_struct.list_easy[0].val)
        self.assertEqual(py3_struct.list_easy[1].val, python_struct.list_easy[1].val)
        self.assertEqual(py3_struct, python_struct._to_py3())
        self.assertEqual(py3_struct._to_python(), python_struct)
