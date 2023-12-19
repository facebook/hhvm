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

import copy
import math
import unittest

from testing.types import (
    Color,
    easy,
    File,
    hard,
    Integers,
    Kind,
    Nested1,
    Nested2,
    Nested3,
    NonCopyable,
    numerical,
    Optionals,
    PrivateCppRefField,
    Reserved,
    Runtime,
    SlowCompare,
    UnusedError,
)
from thrift.py3.common import Protocol
from thrift.py3.serializer import deserialize
from thrift.py3.types import Struct


class StructTests(unittest.TestCase):
    def test_isset_Struct(self) -> None:
        serialized = b'{"name":"/dev/null","type":8}'
        file = deserialize(File, serialized, protocol=Protocol.JSON)
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for 1st
        #  param but got `File`.
        self.assertTrue(Struct.isset(file).type)
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for 1st
        #  param but got `File`.
        self.assertFalse(Struct.isset(file).permissions)
        # required fields are always set
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for 1st
        #  param but got `File`.
        self.assertTrue(Struct.isset(file).name)

        serialized = b'{"name":"/dev/null"}'
        file = deserialize(File, serialized, protocol=Protocol.JSON)
        self.assertEqual(file.type, Kind.REGULAR)
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for 1st
        #  param but got `File`.
        self.assertFalse(Struct.isset(file).type)

    def test_isset_repr(self) -> None:
        serialized = b'{"name":"/dev/null","type":8}'
        file = deserialize(File, serialized, protocol=Protocol.JSON)
        self.assertEqual(
            "Struct.isset(<File>, name=True, permissions=False, type=True)",
            # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for
            #  1st param but got `File`.
            repr(Struct.isset(file)),
        )
        self.assertEqual(
            "Struct.isset(<File>, name=True, permissions=False, type=True)",
            # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for
            #  1st param but got `File`.
            str(Struct.isset(file)),
        )

    def test_isset_Union(self) -> None:
        i = Integers(large=2)
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for
            #  1st param but got `Integers`.
            Struct.isset(i).large

    def test_isset_Error(self) -> None:
        e = UnusedError()
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for 1st
        #  param but got `UnusedError`.
        self.assertFalse(Struct.isset(e).message)

        e = UnusedError(message="ACK")
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.types._T]]` for 1st
        #  param but got `UnusedError`.
        self.assertTrue(Struct.isset(e).message)

    def test_copy(self) -> None:
        x = easy(val=1, an_int=Integers(small=300), name="foo", val_list=[1, 2, 3, 4])
        dif_list = copy.copy(x.val_list)
        self.assertEqual(x.val_list, dif_list)
        dif_int = copy.copy(x.an_int)
        self.assertEqual(x.an_int, dif_int)

    def test_hashability(self) -> None:
        hash(easy())

    def test_str(self) -> None:
        self.assertEqual(
            "easy(name=None, val=0, val_list=i[], an_int=Integers(type=EMPTY, value=None))",
            str(easy()),
        )
        self.assertEqual(
            "easy(name=None, val=0, val_list=i[], an_int=Integers(type=EMPTY, value=None))",
            repr(easy()),
        )

        x = easy(val=1, an_int=Integers(small=300), name="foo", val_list=[1, 2, 3, 4])
        self.assertEqual(
            "easy(name='foo', val=1, val_list=i[1, 2, 3, 4], an_int=Integers(type=small, value=300))",
            str(x),
        )
        self.assertEqual(
            "easy(name='foo', val=1, val_list=i[1, 2, 3, 4], an_int=Integers(type=small, value=300))",
            repr(x),
        )

    def test_optional_struct_creation(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-fixme[19]: Expected 0 positional arguments.
            easy(1, [1, 1], "test", Integers(tiny=1))
        easy(val=1, an_int=Integers(small=500))
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[str]` for 1st param but got `bytes`.
            easy(name=b"binary")
        # Only Required Fields don't accept None
        easy(val=5, an_int=None)

    def test_required_fields_not_enforced(self) -> None:
        # None is not acceptable as a string
        hard(val=1, val_list=[1, 2], name=None, an_int=Integers(small=1))

        hard(val=1, val_list=[1, 2])

    def test_call_replace(self) -> None:
        x = easy(val=1, an_int=Integers(small=300), name="foo")
        y = x(name="bar")
        self.assertNotEqual(x.name, y.name)
        z = y(an_int=None, val=4)
        self.assertNotEqual(x.an_int, z.an_int)
        self.assertNotEqual(x.val, z.val)
        self.assertIsNone(z.an_int.value)
        self.assertEqual(y.val, x.val)
        self.assertEqual(y.an_int, x.an_int)
        x = easy()
        self.assertIsNotNone(x.val)
        self.assertIsNotNone(x.val_list)
        self.assertIsNone(x.name)
        self.assertIsNotNone(x.an_int)

    def test_call_replace_container(self) -> None:
        x = Optionals(values=["a", "b", "c"])
        z = x(values=["b", "c"])
        self.assertEqual(z.values, ["b", "c"])
        y = z(values=None)
        self.assertIsNone(y.values)

    def test_runtime_checks(self) -> None:
        x = Runtime()
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: For 1st param expected `Union[None, bool, __NotSet]`
            #  but got `int`.
            x(bool_val=5)

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[bool]` for 1st param but got `int`.
            Runtime(bool_val=5)

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: For 1st param expected `Union[None, Color, __NotSet]`
            #  but got `int`.
            x(enum_val=2)

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[Color]` for 1st param but got `int`.
            Runtime(enum_val=2)

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: For 1st param expected `Union[None, Sequence[int],
            #  __NotSet]` but got `List[str]`.
            x(int_list_val=["foo", "bar", "baz"])

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[typing.Sequence[int]]` for 1st param
            #  but got `List[str]`.
            Runtime(int_list_val=["foo", "bar", "baz"])

    def test_reserved(self) -> None:
        x = Reserved(
            from_="hello",
            nonlocal_=3,
            ok="bye",
            is_cpdef=True,
            move="Qh4xe1",
            inst="foo",
            changes="bar",
        )
        self.assertEqual(x.from_, "hello")
        self.assertEqual(x.nonlocal_, 3)
        self.assertEqual(x.ok, "bye")
        self.assertEqual(x.is_cpdef, True)
        self.assertEqual(x.move, "Qh4xe1")
        self.assertEqual(x.inst, "foo")
        self.assertEqual(x.changes, "bar")

    def test_ordering(self) -> None:
        x = Runtime(bool_val=False, enum_val=Color.red, int_list_val=[64, 128])
        y = x(bool_val=True)
        self.assertLess(x, y)
        self.assertLessEqual(x, y)
        self.assertGreater(y, x)
        self.assertGreaterEqual(y, x)
        self.assertEqual([x, y], sorted([y, x]))

    def test_noncomparable(self) -> None:
        x = SlowCompare(field1="text", field2=10, field3=Color.red)
        y = x(field3=Color.blue)
        x2 = SlowCompare(field1="text", field2=10, field3=Color.red)

        self.assertEqual(x, x2)
        self.assertNotEqual(x, y)

    def test_noncopyable(self) -> None:
        x = NonCopyable(num=123)
        with self.assertRaises(TypeError):
            x(num=1234)
        with self.assertRaises(TypeError):
            copy.copy(x)
        with self.assertRaises(TypeError):
            copy.deepcopy(x)

    def test_init_with_invalid_field(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[28]: intentionally used a wrong field name "val_lists" for test
            easy(val=1, an_int=Integers(small=300), name="foo", val_lists=[1, 2, 3, 4])

    def test_iterate(self) -> None:
        x = Reserved(
            from_="hello",
            nonlocal_=3,
            ok="bye",
            is_cpdef=True,
            move="Qh4xe1",
            inst="foo",
            changes="bar",
        )
        self.assertEqual(
            list(x),
            [
                ("from_", "hello"),
                ("nonlocal_", 3),
                ("ok", "bye"),
                ("is_cpdef", True),
                ("move", "Qh4xe1"),
                ("inst", "foo"),
                ("changes", "bar"),
                ("__mangled_str", ""),
                ("__mangled_int", 0),
            ],
        )
        self.assertEqual(
            [k for k, _ in Reserved],
            [
                "from_",
                "nonlocal_",
                "ok",
                "is_cpdef",
                "move",
                "inst",
                "changes",
                "__mangled_str",
                "__mangled_int",
            ],
        )

    def test_dir(self) -> None:
        expected = ["__iter__", "an_int", "name", "val", "val_list"]
        self.assertEqual(expected, dir(easy()))
        self.assertEqual(expected, dir(easy))
        self.assertEqual(["__iter__"], dir(Struct))

    def test_update_nested_fields(self) -> None:
        n = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        n = Struct.update_nested_field(n, {"a.b.c": easy(val=128)})
        self.assertEqual(n.a.b.c.val, 128)

    def test_update_multiple_nested_fields(self) -> None:
        n = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        n = Struct.update_nested_field(
            n,
            {
                "a.b.c.name": "bar",
                "a.b.c.val": 256,
            },
        )
        self.assertEqual(n.a.b.c.name, "bar")
        self.assertEqual(n.a.b.c.val, 256)

    def test_update_invalid_nested_fields(self) -> None:
        n = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        with self.assertRaises(ValueError):
            Struct.update_nested_field(n, {"": 0})
        with self.assertRaises(ValueError):
            Struct.update_nested_field(n, {"e": 0})
        with self.assertRaises(ValueError):
            Struct.update_nested_field(n, {"a.b.e": 0})
        with self.assertRaises(ValueError):
            Struct.update_nested_field(n, {"a.e.f": 0})

    def test_update_conflicting_nested_fields(self) -> None:
        n = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        with self.assertRaises(ValueError):
            n = Struct.update_nested_field(
                n,
                {
                    "a.b.c": easy(val=128),
                    "a.b.c.val": 256,
                },
            )

    def test_to_py3(self) -> None:
        e = easy()
        self.assertEqual(e, e._to_py3())


class NumericalConversionsTests(unittest.TestCase):
    def test_overflow(self) -> None:
        with self.assertRaises(OverflowError):
            numerical(float_val=5, int_val=2**63 - 1)

        with self.assertRaises(OverflowError):
            numerical(float_val=5, int_val=2, int_list=[5, 2**32])

    def test_int_to_float(self) -> None:
        x = numerical(int_val=5, float_val=5, float_list=[1, 5, 6])
        x(float_val=10)
        x(float_val=10)
        x(float_list=[6, 7, 8])

    def test_int_to_i64(self) -> None:
        large = 2**63 - 1
        numerical(int_val=5, float_val=5, i64_val=int(large))
        too_large = 2**65 - 1
        with self.assertRaises(OverflowError):
            numerical(int_val=5, float_val=5, i64_val=int(too_large))

    def test_float_to_int_required_field(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[int]` for 1st param but got `float`.
            numerical(int_val=math.pi, float_val=math.pi)

    def test_float_to_int_unqualified_field(self) -> None:
        with self.assertRaises(TypeError):
            numerical(
                float_val=math.pi,
                # pyre-fixme[6]: Expected `Optional[int]` for 3rd param but got `float`.
                int_val=math.pi,
            )

    def test_float_to_int_list(self) -> None:
        with self.assertRaises(TypeError):
            numerical(
                int_val=5,
                float_val=math.pi,
                # pyre-fixme[6]: Expected `Optional[typing.Sequence[int]]` for 3rd
                #  param but got `List[float]`.
                int_list=[math.pi, math.e],
            )

    def test_private_cpp_ref_field(self) -> None:
        x = PrivateCppRefField(
            field1=easy(val=1, name="11"),
            field2=easy(val=2, name="22"),
            field3=easy(val=3, name="33"),
        )
        field1 = x.field1
        field2 = x.field2
        field3 = x.field3
        if field1:
            self.assertEqual(field1.val, 1)
            self.assertEqual(field1.name, "11")
        if field2:
            self.assertEqual(field2.val, 2)
            self.assertEqual(field2.name, "22")
        if field3:
            self.assertEqual(field3.val, 3)
            self.assertEqual(field3.name, "33")
