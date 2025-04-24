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
