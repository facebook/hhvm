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
import enum
import math
import sys
import types
import unittest

from testing.dependency.types import IncludedStruct
from testing.sub_dependency.types import Basic, IncludedColour
from testing.types import (
    Color,
    easy,
    FANCY_CONST,
    File,
    hard,
    Integers,
    Kind,
    mixed,
    Nested1,
    Nested2,
    Nested3,
    NonCopyable,
    numerical,
    OptionalFile,
    Optionals,
    PrivateCppRefField,
    Reserved,
    Runtime,
    SlowCompare,
    StringBucket,
    UnusedError,
)
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import (
    brokenInAutoMigrate,
    is_auto_migrated,
)
from thrift.py3.common import Protocol
from thrift.py3.serializer import deserialize, serialize
from thrift.py3.types import Struct

try:
    from enum import Enum, StrEnum
except ImportError:

    class StrEnum(str, enum.Enum):
        pass


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

    def test_property(self) -> None:
        r = Runtime(property="foo", int_list_val=[2, 3, 4])
        self.assertEqual(r.property, "foo")
        self.assertEqual(r.int_list_val, [2, 3, 4])

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
        self.assertIs(x, copy.copy(x))

        cpp_noncopyable = NonCopyable(num=123)
        self.assertIs(cpp_noncopyable, copy.copy(cpp_noncopyable))
        self.assertIs(cpp_noncopyable, copy.deepcopy(cpp_noncopyable))

    @brokenInAutoMigrate()
    def test_noncopyable(self) -> None:
        x = NonCopyable(num=123)
        # when thrift-py3 decoupled from cpp2, call operator will work
        with self.assertRaises(TypeError):
            x(num=1234)

    def test_hashability(self) -> None:
        hash(easy())

    @brokenInAutoMigrate()
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

    def test_no_dict(self) -> None:
        # Struct
        with self.assertRaises(AttributeError):
            easy().__dict__

        # Union
        with self.assertRaises(AttributeError):
            Integers().__dict__

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

    def test_struct_module_name(self) -> None:
        variant_prefix = "thrift_" if is_auto_migrated() else ""
        expected = f"testing.{variant_prefix}types"

        self.assertEqual(easy.__module__, expected)
        self.assertEqual(easy().__class__.__module__, expected)
        self.assertEqual(File.__module__, expected)
        self.assertEqual(File().__class__.__module__, expected)
        self.assertEqual(hard.__module__, expected)
        self.assertEqual(hard().__class__.__module__, expected)

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

        def auto_migrate_mangle(name: str) -> str:
            return f"_Reserved{name}" if is_auto_migrated() else name

        mangled_str_name = auto_migrate_mangle("__mangled_str")
        mangled_int_name = auto_migrate_mangle("__mangled_int")
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
                (mangled_str_name, ""),
                (mangled_int_name, 0),
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
                mangled_str_name,
                mangled_int_name,
            ],
        )

    def test_dir(self) -> None:
        if not is_auto_migrated():
            expected = ["__iter__", "an_int", "name", "val", "val_list"]
        else:
            expected = ["__iter__", "an_int", "name", "py3_hidden", "val", "val_list"]
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

    def test_compare_optional(self) -> None:
        x = StringBucket()
        y = StringBucket()

        # Both are default so they are equal and neither are greater
        self.assertFalse(x < y)
        self.assertFalse(x > y)
        self.assertTrue(x <= y)
        self.assertTrue(x >= y)

        x = StringBucket(one="one")

        # x has a field set so it's greater
        self.assertFalse(x < y)
        self.assertTrue(x > y)
        self.assertFalse(x <= y)
        self.assertTrue(x >= y)

        # x has an optional field set so even though it's empty string, "" > None
        x = StringBucket(two="")
        self.assertFalse(x < y)
        self.assertTrue(x > y)
        self.assertFalse(x <= y)
        self.assertTrue(x >= y)

        # comparisons happen in field order so because y.one > x.one, y > x
        y = StringBucket(one="one")
        self.assertTrue(x < y)
        self.assertFalse(x > y)
        self.assertTrue(x <= y)
        self.assertFalse(x >= y)

        z = easy()
        with self.assertRaises(TypeError):
            # pyre-fixme[58]: Test to make sure that invalid comparison errors out
            z < y  # noqa: B015

    def test_included_struct_const(self) -> None:
        self.assertIsInstance(FANCY_CONST, IncludedStruct)
        self.assertEqual(FANCY_CONST.val, Basic(nom="fancy", val=47, bin=b"01010101"))
        self.assertEqual(FANCY_CONST.color, IncludedColour.red)
        self.assertEqual(
            FANCY_CONST.color_list, [IncludedColour.red, IncludedColour.blue]
        )
        self.assertEqual(
            FANCY_CONST.color_set, {IncludedColour.red, IncludedColour.blue}
        )
        self.assertEqual(
            FANCY_CONST.color_map,
            {IncludedColour.red: Basic(), IncludedColour.blue: Basic(nom="b")},
        )

    def test_instance_base_class(self) -> None:
        self.assertIsInstance(Nested1(), Struct)
        self.assertIsInstance(Nested1(), Nested1)
        self.assertNotIsInstance(Nested1(), Nested2)
        self.assertNotIsInstance(3, Nested1)
        self.assertNotIsInstance(3, Struct)
        self.assertTrue(issubclass(Nested1, Struct))
        self.assertFalse(issubclass(int, Struct))
        self.assertFalse(issubclass(int, Nested1))
        self.assertFalse(issubclass(Struct, Nested1))
        self.assertFalse(issubclass(Nested1, Nested2))

    def test_subclass_not_allow_inheritance(self) -> None:
        thrift_python_err = r"Inheritance from generated thrift struct .+ is deprecated. Please use composition."
        cython_err = (
            r"type '.+' is not an acceptable base type"
            if not hasattr(File, "_FBTHRIFT__PYTHON_CLASS")
            else r"Inheritance of thrift-generated .+ from TestSubclass is deprecated."
        )
        err_regex = thrift_python_err if is_auto_migrated() else cython_err
        with self.assertRaisesRegex(TypeError, err_regex):
            types.new_class("TestSubclass", bases=(File,))

    def test_subclass_allow_inheritance(self) -> None:
        c = TestSubclass(name="bob")
        self.assertIsInstance(c, OptionalFile)
        self.assertIsInstance(c, Struct)
        self.assertIsInstance(OptionalFile(), Struct)
        self.assertEqual(c.name, "bob")

    def test_subclass_allow_inheritance_ancestor(self) -> None:
        c = TestSubSubclass(name="bob")
        self.assertIsInstance(c, TestSubclass)
        self.assertIsInstance(c, OptionalFile)
        self.assertIsInstance(c, Struct)
        self.assertEqual(c.name, "bob")

    def test_defaulted_optional_field(self) -> None:
        def assert_mixed(m: mixed) -> None:
            if is_auto_migrated():
                self.assertIsNone(m.opt_field)
                self.assertIsNone(m.opt_float)
                self.assertIsNone(m.opt_int)
                self.assertIsNone(m.opt_enum)
                self.assertIsNone(m.opt_pointless_default_str)
                self.assertIsNone(m.opt_pointless_default_int)
            else:
                self.assertEqual(m.opt_field, "optional")
                self.assertEqual(m.opt_float, 1.0)
                self.assertEqual(m.opt_int, 1)
                self.assertEqual(m.opt_enum, Color.red)
                self.assertEqual(m.opt_pointless_default_str, "")
                self.assertEqual(m.opt_pointless_default_int, 0)

        def assert_isset(m: mixed) -> None:
            # pyre-fixme[6]: the pyre typing for this is broken in thrift-py3
            isset = Struct.isset(m)

            for fld_name, _ in mixed:
                if not fld_name.startswith("opt_") or "ref" in fld_name:
                    continue

                self.assertFalse(getattr(isset, fld_name), fld_name)

        # constructor
        m = mixed()
        assert_mixed(m)
        assert_isset(m)

        # call operator
        m = m(some_field_="don't care")
        assert_mixed(m)
        assert_isset(m)

        # serialization round-trip
        m = deserialize(mixed, serialize(m))
        assert_mixed(m)
        assert_isset(m)

        ### Now with explicit `None` set
        # in py3, even setting the field explicitly to None:
        #   - the field value is still the default (non-None)
        #   - the issset value is still False
        #  This is deeply regrettable.
        m = mixed(opt_field=None)
        assert_mixed(m)
        assert_isset(m)

        m = m(opt_field=None)
        assert_mixed(m)
        assert_isset(m)

        m = deserialize(mixed, serialize(m))
        assert_mixed(m)
        assert_isset(m)

        # basic sanity check for normal set behavior
        non_opt = mixed(
            opt_field="foo",
            opt_float=2.0,
            opt_int=3,
            opt_enum=Color.blue,
            opt_pointless_default_str="bar",
            opt_pointless_default_int=4,
        )
        self.assertEqual(non_opt.opt_field, "foo")
        self.assertEqual(non_opt.opt_float, 2.0)
        self.assertEqual(non_opt.opt_int, 3)
        self.assertEqual(non_opt.opt_enum, Color.blue)
        self.assertEqual(non_opt.opt_pointless_default_str, "bar")
        self.assertEqual(non_opt.opt_pointless_default_int, 4)
        # pyre-fixme[6]: the pyre typing for this is broken in thrift-py3
        non_opt_isset = Struct.isset(non_opt)
        for field, field_value in non_opt:
            if not field.startswith("opt_"):
                continue
            self.assertEqual(
                getattr(non_opt_isset, field, False), field_value is not None, field
            )

    def roundtrip(self, x: numerical) -> numerical:
        return deserialize(numerical, serialize(x))

    def test_permissive_init_int_with_enum(self) -> None:
        n = numerical(int_val=Kind.LINK)

        def assert_strict(n: numerical) -> None:
            self.assertIs(type(n.int_val), int)
            self.assertEqual(n.int_val, Kind.LINK.value)

        assert_strict(n)

        rt = self.roundtrip(n)
        assert_strict(rt)

    def test_permissive_init_float_with_enum(self) -> None:
        n = numerical(float_val=Kind.LINK)

        def assert_strict(n: numerical) -> None:
            self.assertIs(type(n.float_val), float)
            self.assertEqual(n.float_val, Kind.LINK.value)

        assert_strict(n)

        rt = self.roundtrip(n)
        assert_strict(rt)

    def test_permissive_init_float_with_bool(self) -> None:
        n = numerical(float_val=True)

        def assert_strict(n: numerical) -> None:
            self.assertIs(type(n.float_val), float)
            self.assertEqual(n.float_val, float(True))

        assert_strict(n)

        rt = self.roundtrip(n)
        assert_strict(rt)

    def test_permissive_init_float_with_int(self) -> None:
        n = numerical(float_val=888)

        def assert_strict(n: numerical) -> None:
            self.assertIs(type(n.float_val), float)
            self.assertEqual(n.float_val, 888.0)

        assert_strict(n)

        rt = self.roundtrip(n)
        assert_strict(rt)

    def test_bad_kwarg(self) -> None:
        if is_auto_migrated():
            with self.assertRaises(TypeError):
                # pyre-ignore[28]: simulating customer clowntown
                numerical(float_val=1.0, bad_kwarg=None, int_val=2)
        else:
            # pyre-ignore[28]: simulating customer clowntown
            n = numerical(float_val=1.0, bad_kwarg=None, int_val=2)
            # thrift-py3 ignores
            self.assertIsInstance(n, numerical)
            self.assertEqual(n.float_val, 1.0)
            self.assertEqual(n.int_val, 2)

    def test_string_enum(self) -> None:
        bucket = StringBucket(one=MyStrEnum.A, two=MyStrEnumBad.B)
        self.assertEqual(bucket.one, MyStrEnum.A.value)
        self.assertEqual(bucket.one, MyStrEnum.A)
        if not is_auto_migrated() or sys.version_info < (3, 11):
            self.assertEqual(bucket.two, MyStrEnumBad.B.value)
            self.assertEqual(bucket.two, MyStrEnumBad.B)
        else:  # auto-migration for Python 3.11+
            # thrift-python gets the "bad" behavior due to usage of format
            # in StringTypeInfo.to_internal_data
            self.assertEqual(bucket.two, "MyStrEnumBad.B")
            self.assertNotEqual(bucket.two, MyStrEnumBad.B)


class TestSubclass(OptionalFile):
    pass


class TestSubSubclass(TestSubclass):
    pass


class MyStrEnum(StrEnum):
    A = "a"
    B = "b"


class MyStrEnumBad(str, Enum):
    A = "a"
    B = "b"
