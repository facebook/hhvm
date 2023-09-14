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

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from contextlib import ExitStack
from importlib import resources

file_manager = ExitStack()
thrift = str(
    file_manager.enter_context(resources.path(__package__, "thrift")).absolute()
)


def write_file(path, content):
    if d := os.path.dirname(path):
        os.makedirs(d)
    with open(path, "w") as f:
        f.write(content)


class CompilerFailureTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def run_thrift(self, *args, gen="mstch_cpp2"):
        # Annotation files are copied from `thrift/annotation`
        argsx = [
            thrift,
            "--gen",
            gen,
            "-I",
            resources.path(__package__, "implicit_includes"),
        ]
        argsx.extend(args)
        pipe = subprocess.PIPE
        p = subprocess.Popen(argsx, stdout=pipe, stderr=pipe)
        out, err = p.communicate()
        out = out.decode(sys.getdefaultencoding())
        err = err.decode(sys.getdefaultencoding())
        err = err.replace("{}/".format(self.tmp), "")
        err = err.replace("\r\n", "\n")
        return p.returncode, out, err

    def test_duplicate_method_name_base_base(self):
        # tests overriding a method of the parent and the grandparent services
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                service MySBB {
                    void lol();
                }
                """
            ),
        )
        write_file(
            "bar.thrift",
            textwrap.dedent(
                """\
                include "foo.thrift"
                service MySB extends foo.MySBB {
                    void meh();
                }
                """
            ),
        )
        write_file(
            "baz.thrift",
            textwrap.dedent(
                """\
                include "bar.thrift"
                service MyS extends bar.MySB {
                    void lol();
                    void meh();
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("baz.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:baz.thrift:3] Function `MyS.lol` redefines `foo.MySBB.lol`.\n"
            "[ERROR:baz.thrift:4] Function `MyS.meh` redefines `bar.MySB.meh`.\n",
        )

    def test_circular_include_dependencies(self):
        # tests overriding a method of the parent and the grandparent services
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "bar.thrift"
                service MySBB {
                    void lol();
                }
                """
            ),
        )
        write_file(
            "bar.thrift",
            textwrap.dedent(
                """\
                include "foo.thrift"
                service MySB extends foo.MySBB {
                    void meh();
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:bar.thrift:1] Circular dependency found: "
            "file `foo.thrift` is already parsed.\n",
        )

    def test_struct_optional_refs(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A {
                    1: A rec (cpp.ref);
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 0)
        self.assertEqual(
            err,
            "[WARNING:foo.thrift:2] `cpp.ref` field `rec` must be optional if it is recursive.\n"
            "[WARNING:foo.thrift:2] cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `rec`.\n",
        )

    def test_structured_ref(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"

                struct Foo {
                    1: optional Foo field1 (cpp.ref);

                    @cpp.Ref{type = cpp.RefType.Unique}
                    2: optional Foo field2;

                    @cpp.Ref{type = cpp.RefType.Unique}
                    3: optional Foo field3 (cpp.ref);

                    @cpp.Ref{type = cpp.RefType.Unique}
                    @cpp.Ref{type = cpp.RefType.Unique}
                    4: optional Foo field4;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            "\n" + err,
            textwrap.dedent(
                """
                [WARNING:foo.thrift:4] cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field1`.
                [WARNING:foo.thrift:6] @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `field2`.
                [ERROR:foo.thrift:9] The @cpp.Ref annotation cannot be combined with the `cpp.ref` or `cpp.ref_type` annotations. Remove one of the annotations from `field3`.
                [WARNING:foo.thrift:9] cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field3`.
                [WARNING:foo.thrift:9] @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `field3`.
                [WARNING:foo.thrift:12] @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `field4`.
                [ERROR:foo.thrift:13] Structured annotation `Ref` is already defined for `field4`.
            """
            ),
        )

    def test_adapter(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"
                include "thrift/annotation/hack.thrift"

                typedef i64 MyI64 (cpp.adapter="MyAdapter", hack.adapter="MyAdapter")

                struct MyStruct {
                    @cpp.Adapter{name="MyAdapter"}
                    @hack.Adapter{name="MyAdapter"}
                    1: MyI64 my_field;
                    @cpp.Adapter{}
                    @hack.Adapter{name="MyAdapter"}
                    2: i64 my_field2;
                    @cpp.Adapter{name="MyAdapter"}
                    @hack.Adapter{name="MyAdapter"}
                    3: optional i64 my_field3 (cpp.ref);
                    @cpp.Adapter{name="MyAdapter"}
                    @hack.Adapter{name="MyAdapter"}
                    4: optional i64 my_field4 (cpp.ref_type = "unique");
                    @cpp.Adapter{name="MyAdapter"}
                    @hack.Adapter{name="MyAdapter"}
                    @cpp.Ref{type = cpp.RefType.Unique}
                    5: optional i64 my_field5;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:7] `@cpp.Adapter` cannot be combined with "
            "`cpp.adapter` in `my_field`.\n"
            "[ERROR:foo.thrift:7] `@hack.Adapter` cannot be combined with "
            "`hack.adapter` in `my_field`.\n"
            "[ERROR:foo.thrift:10] key `name` not found.\n"
            "[ERROR:foo.thrift:13] cpp.ref, cpp2.ref are deprecated. "
            "Please use @thrift.Box annotation instead in `my_field3` with @cpp.Adapter.\n"
            "[ERROR:foo.thrift:16] cpp.ref_type = `unique`, cpp2.ref_type = `unique` "
            "are deprecated. Please use @thrift.Box annotation instead in `my_field4` with @cpp.Adapter.\n"
            "[ERROR:foo.thrift:19] @cpp.Ref{type = cpp.RefType.Unique} is deprecated. "
            "Please use @thrift.Box annotation instead in `my_field5` with @cpp.Adapter.\n",
        )

    def test_typedef_adapter(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"
                include "thrift/annotation/hack.thrift"

                @cpp.Adapter{}
                @hack.Adapter{}
                typedef i32 MyI32

                @cpp.Adapter{name="MyAdapter"}
                @hack.Adapter{name="MyAdapter"}
                typedef i64 MyI64

                @cpp.Adapter{name="MyAdapter"}
                @hack.Adapter{name="MyAdapter"}
                typedef MyI64 DoubleMyI64

                struct MyStruct {
                    1: MyI64 my_field;
                    2: MyI64 (cpp.adapter="MyAdapter", hack.adapter="MyAdapter") my_field1;
                    3: MyI64 my_field2 (cpp.adapter="MyAdapter", hack.adapter="MyAdapter");
                }

                @cpp.Adapter{name="MyAdapter"}
                struct Adapted {}

                @cpp.Adapter{name="MyAdapter"}
                typedef Adapted DoubleAdapted
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:18] `@cpp.Adapter` cannot be combined with "
            "`cpp.adapter` in `my_field1`.\n"
            "[ERROR:foo.thrift:18] `@hack.Adapter` cannot be combined with "
            "`hack.adapter` in `my_field1`.\n"
            "[ERROR:foo.thrift:4] key `name` not found.\n"
            "[ERROR:foo.thrift:4] key `name` not found.\n"
            "[ERROR:foo.thrift:12] The `@cpp.Adapter` annotation cannot be annotated more "
            "than once in all typedef levels in `DoubleMyI64`.\n"
            "[ERROR:foo.thrift:12] The `@hack.Adapter` annotation cannot be annotated more "
            "than once in all typedef levels in `DoubleMyI64`.\n"
            "[ERROR:foo.thrift:25] The `@cpp.Adapter` annotation cannot be annotated more "
            "than once in all typedef levels in `DoubleAdapted`.\n",
        )

    def test_hack_wrapper(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/hack.thrift"

                @hack.Adapter{name = "\MyAdapter1"}
                typedef i64 i64Adapted

                @hack.Wrapper{name = "\MyTypeIntWrapper"}
                typedef i64 i64WithWrapper

                @hack.Wrapper{name = "\MyTypeIntWrapper"}
                @hack.Adapter{name = "\MyAdapter1"}
                typedef i64 i64Wrapped_andAdapted

                @hack.Wrapper{name = "\MyTypeIntWrapper"}
                typedef i64Adapted i64Wrapped_andAdapted_2

                typedef list<i64Wrapped_andAdapted> list_of_i64Wrapped_andAdapted

                @hack.Adapter{name = "\MyAdapter1"}
                typedef list<i64Wrapped_andAdapted> adapted_list_of_i64Wrapped_andAdapted

                typedef StructWithWrapper structWithWrapper_typedf

                @hack.Adapter{name = "\MyAdapter1"}
                typedef StructWithWrapper adapted_structWithWrapper_typedf

                @hack.Wrapper{name = "\MyStructWrapper"}
                struct StructWithWrapper {
                1: i64 int_field;
                }

                @hack.Wrapper{name = "\MyStructWrapper"}
                struct MyNestedStruct {
                @hack.FieldWrapper{name = "\MyFieldWrapper"}
                1: i64WithWrapper double_wrapped_field;
                @hack.FieldWrapper{name = "\MyFieldWrapper"}
                @hack.Adapter{name = "\MyFieldAdapter"}
                2: i64WithWrapper double_wrapped_and_adapted_field;
                @hack.FieldWrapper{name = "\MyFieldWrapper"}
                3: StructWithWrapper double_wrapped_struct;
                @hack.FieldWrapper{name = "\MyFieldWrapper"}
                4: map<string, StructWithWrapper> wrapped_map_of_string_to_StructWithWrapper;
                @hack.Adapter{name = "\MyFieldAdapter"}
                5: map<string, StructWithWrapper> adapted_map_of_string_to_StructWithWrapper;
                @hack.FieldWrapper{name = "\MyFieldWrapper"}
                @hack.Adapter{name = "\MyFieldAdapter"}
                6: StructWithWrapper adapted_double_wrapped_struct;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:35] `@hack.Adapter` on `double_wrapped_and_adapted_field` "
            "cannot be combined with `@hack.Wrapper` on `i64WithWrapper`.\n"
            "[ERROR:foo.thrift:42] `@hack.Adapter` on `adapted_map_of_string_to_StructWithWrapper` "
            "cannot be combined with `@hack.Wrapper` on `StructWithWrapper`.\n"
            "[ERROR:foo.thrift:44] `@hack.Adapter` on `adapted_double_wrapped_struct` "
            "cannot be combined with `@hack.Wrapper` on `StructWithWrapper`.\n"
            "[ERROR:foo.thrift:18] `@hack.Adapter` on `adapted_list_of_i64Wrapped_andAdapted` "
            "cannot be combined with `@hack.Wrapper` on `i64Wrapped_andAdapted`.\n"
            "[ERROR:foo.thrift:23] `@hack.Adapter` on `adapted_structWithWrapper_typedf` "
            "cannot be combined with `@hack.Wrapper` on `StructWithWrapper`.\n",
        )

    def test_mixin_nonstruct_members(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A {
                    1: i32 i (cpp.mixin);
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:2] Mixin field `i` type must be a struct or union. Found `i32`.\n",
        )

    def test_mixin_in_union(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A { 1: i32 i }
                union B {
                    1: A a (cpp.mixin);
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err, "[ERROR:foo.thrift:3] Union `B` cannot contain mixin field `a`.\n"
        )

    def test_mixin_with_cpp_ref(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A { 1: i32 i }
                struct B {
                    1: A a (cpp.ref = "true", cpp.mixin);
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                """\
                [WARNING:foo.thrift:3] `cpp.ref` field `a` must be optional if it is recursive.
                [WARNING:foo.thrift:3] cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `a`.
                [ERROR:foo.thrift:3] Mixin field `a` can not be a ref in cpp.
                """
            ),
        )

    def test_bitpack_with_tablebased_seriliazation(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"
                struct A { 1: i32 i }
                @cpp.PackIsset
                struct D { 1: i32 i }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift", gen="mstch_cpp2:json,tablebased")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                """\
                [ERROR:foo.thrift:3] Tablebased serialization is incompatible with isset bitpacking for struct `D`
                """
            ),
        )

    def test_optional_mixin_field(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A { 1: i32 i }
                struct B {
                    1: optional A a (cpp.mixin);
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                "[ERROR:foo.thrift:3] Mixin field `a` cannot be optional.\n"
            ),
        )

    def test_structured_annotations_uniqueness(self):
        write_file(
            "other/foo.thrift",
            textwrap.dedent(
                """\
                struct Foo {}
                """
            ),
        )
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "other/foo.thrift"
                struct Foo {
                    1: i32 count;
                }

                @foo.Foo
                @Foo{count=1}
                @Foo{count=2}
                typedef i32 Annotated
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            # TODO(afuller): Fix t_scope to not include the locally defined Foo as
            # `foo.Foo`, which override the included foo.Foo definition.
            "[ERROR:foo.thrift:7] Structured annotation `Foo` is already defined for `Annotated`.\n"
            "[ERROR:foo.thrift:8] Structured annotation `Foo` is already defined for `Annotated`.\n",
        )

    def test_structured_annotations_type_resolved(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct Annotation {
                    1: i32 count;
                    2: TooForward forward;
                }

                @Annotation{count=1, forward=TooForward{name="abc"}}
                struct Annotated {
                    1: string name;
                }

                struct TooForward {
                    1: string name;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:6] The type 'TooForward' "
            "is not defined yet. Types must be defined before the usage in "
            "constant values.\n",
        )

    def test_too_many_splits(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct Foo { 1: i32 field }
                struct Bar { 1: i32 field }
                exception Baz { 1: i32 field }
                enum E { f = 0 }
                service SumService {
                    i32 sum(1: i32 num1, 2: i32 num2);
                }
                """
            ),
        )

        ret, out, err = self.run_thrift(
            "foo.thrift", gen="mstch_cpp2:types_cpp_splits=4"
        )
        self.assertEqual(ret, 0)

        ret, out, err = self.run_thrift(
            "foo.thrift", gen="mstch_cpp2:types_cpp_splits=5"
        )

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] `types_cpp_splits=5` is misconfigured: "
            "it can not be greater than the number of objects, which is 4.\n",
        )

        ret, out, err = self.run_thrift(
            "foo.thrift", gen="mstch_cpp2:types_cpp_splits=3a"
        )

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] Invalid types_cpp_splits value: `3a`\n",
        )

    def test_invalid_splits(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct Foo { 1: i32 field }
                """
            ),
        )

        ret, out, err = self.run_thrift(
            "foo.thrift", gen="mstch_cpp2:types_cpp_splits=1a"
        )

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] Invalid types_cpp_splits value: `1a`\n",
        )

    def test_too_many_client_splits(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                service MyService1 {
                    i32 func1(1: i32 num);
                    i32 func2(1: i32 num);
                    i32 func3(1: i32 num);
                }
                service MyService2 {
                    i32 func1(1: i32 num);
                    i32 func2(1: i32 num);
                }
                """
            ),
        )

        ret, out, err = self.run_thrift(
            "foo.thrift", gen="mstch_cpp2:client_cpp_splits={MyService1:3,MyService2:2}"
        )
        self.assertEqual(ret, 0)

        ret, out, err = self.run_thrift(
            "foo.thrift", gen="mstch_cpp2:client_cpp_splits={MyService1:3,MyService2:3}"
        )
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:6] `client_cpp_splits=3` (For service MyService2) is misconfigured: it can not be greater than the number of functions, which is 2.\n",
        )

    def test_invalid_client_splits(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                service MyService1 {
                    i32 func1(1: i32 num);
                    i32 func2(1: i32 num);
                    i32 func3(1: i32 num);
                }
                service MyService2 {
                    i32 func1(1: i32 num);
                    i32 func2(1: i32 num);
                }
                """
            ),
        )

        ret, out, err = self.run_thrift(
            "foo.thrift",
            gen="mstch_cpp2:client_cpp_splits={MyService1:3:1,MyService2:2}",
        )
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] Invalid pair `MyService1:3:1` in client_cpp_splits value: `MyService1:3:1,MyService2:2`\n",
        )

        ret, out, err = self.run_thrift(
            "foo.thrift", gen="mstch_cpp2:client_cpp_splits={MyService3:1}"
        )
        self.assertEqual(ret, 0)

    def test_lazy_field(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A {
                    1: i32 field (cpp.experimental.lazy)
                }
                typedef double FP
                struct B {
                    1: FP field (cpp.experimental.lazy)
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                "[ERROR:foo.thrift:2] Integral field `field` can not be"
                " marked as lazy, since doing so won't bring any benefit.\n"
                "[ERROR:foo.thrift:6] Floating point field `field` can not be"
                " marked as lazy, since doing so won't bring any benefit.\n"
            ),
        )

    def test_bad_throws(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A {}

                service B {
                    void foo() throws (1: A ex)
                    stream<i32 throws (1: A ex)> bar()
                    sink<i32 throws (1: A ex),
                         i32 throws (1: A ex)> baz()
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                "[ERROR:foo.thrift:4] Non-exception type, `A`, in throws.\n"
                "[ERROR:foo.thrift:5] Non-exception type, `A`, in throws.\n"
                "[ERROR:foo.thrift:6] Non-exception type, `A`, in throws.\n"
                "[ERROR:foo.thrift:7] Non-exception type, `A`, in throws.\n"
            ),
        )

    def test_boxed_ref(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"
                include "thrift/annotation/thrift.thrift"

                @thrift.Experimental
                package "apache.org/thrift/test"

                struct MyStruct {
                    1: i32 field1 = 1;
                }

                typedef MyStruct MyStruct2

                struct A {
                    1: optional i64 field (cpp.ref, thrift.box);
                    @thrift.Box
                    @thrift.InternBox
                    2: optional i64 field2;
                    @thrift.InternBox
                    3: MyStruct field3 = {"field1" : 1};
                    @cpp.Ref
                    @thrift.Box
                    @thrift.InternBox
                    @thrift.TerseWrite
                    4: MyStruct field4;
                    @thrift.InternBox
                    @thrift.TerseWrite
                    5: MyStruct field5;
                    @thrift.InternBox
                    @thrift.TerseWrite
                    6: MyStruct2 field6;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                "[WARNING:foo.thrift:14] cpp.box and thrift.box are deprecated. Please use @thrift.Box annotation instead in `field`.\n"
                "[ERROR:foo.thrift:14] The `@thrift.Box` annotation cannot be combined with the other reference annotations."
                " Only annotate a single reference annotations from `field`.\n"
                "[ERROR:foo.thrift:15] The `@thrift.InternBox` annotation cannot be combined with the other reference annotations."
                " Only annotate a single reference annotations from `field2`.\n"
                "[ERROR:foo.thrift:15] The `@thrift.InternBox` annotation can only be used with a struct field.\n"
                "[ERROR:foo.thrift:15] The `@thrift.InternBox` annotation can only be used with unqualified or terse fields."
                " Make sure `field2` is unqualified or annotated with `@thrift.TerseWrite`.\n"
                "[ERROR:foo.thrift:18] The `@thrift.InternBox` annotation currently does not support a field with custom default.\n"
                "[ERROR:foo.thrift:20] The `@thrift.InternBox` annotation cannot be combined with the other reference annotations."
                " Only annotate a single reference annotations from `field4`.\n"
                "[ERROR:foo.thrift:20] The `thrift.box` annotation can only be used with optional fields. Make sure `field4` is optional.\n"
            ),
        )

    def test_boxed_optional(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"

                struct A {
                    1: i64 field (cpp.box)
                    @thrift.Box
                    2: i64 field2
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                "[WARNING:foo.thrift:4] cpp.box and thrift.box are deprecated. Please use @thrift.Box annotation instead in `field`.\n"
                "[ERROR:foo.thrift:4] The `thrift.box` annotation can only be used with optional fields. Make sure `field` is optional.\n"
                "[ERROR:foo.thrift:5] The `thrift.box` annotation can only be used with optional fields. Make sure `field2` is optional.\n"
            ),
        )

    def test_nonexist_field_name(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct Foo {}
                typedef list<Foo> List
                const List l = [{"foo": "bar"}];
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                "[ERROR:foo.thrift:3] type error: `Foo` has no field `foo`.\n"
            ),
        )

    def test_annotation_scopes(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/scope.thrift"

                struct NotAnAnnot {}

                @scope.Struct
                struct StructAnnot{}
                @scope.Field
                struct FieldAnnot{}

                @scope.Struct
                @scope.Field
                struct StructOrFieldAnnot {}

                @scope.Enum
                struct EnumAnnot {}

                @NotAnAnnot
                @StructAnnot
                @FieldAnnot
                @StructOrFieldAnnot
                @EnumAnnot
                struct TestStruct {
                    @FieldAnnot
                    @StructAnnot
                    @StructOrFieldAnnot
                    1: bool test_field;
                }

                @EnumAnnot
                enum TestEnum { Foo = 0, Bar = 1 }

                typedef StructAnnot AliasedAnnot

                @AliasedAnnot
                struct AlsoStruct {}
                """
            ),
        )

        ret, out, err = self.run_thrift("--legacy-strict", "foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            "\n" + err,
            textwrap.dedent(
                """
                [WARNING:foo.thrift:17] Using `NotAnAnnot` as an annotation, even though it has not been enabled for any annotation scope.
                [ERROR:foo.thrift:19] `FieldAnnot` cannot annotate `TestStruct`
                [ERROR:foo.thrift:21] `EnumAnnot` cannot annotate `TestStruct`
                [ERROR:foo.thrift:24] `StructAnnot` cannot annotate `test_field`
                [WARNING:foo.thrift:34] Using `AliasedAnnot` as an annotation, even though it has not been enabled for any annotation scope.
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            "\n" + err,
            textwrap.dedent(
                """
                [ERROR:foo.thrift:19] `FieldAnnot` cannot annotate `TestStruct`
                [ERROR:foo.thrift:21] `EnumAnnot` cannot annotate `TestStruct`
                [ERROR:foo.thrift:24] `StructAnnot` cannot annotate `test_field`
                """
            ),
        )

    def test_lazy_struct_compatibility(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct Foo {
                    1: list<i32> field (cpp.experimental.lazy)
                } (cpp.methods = "")
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                "[ERROR:foo.thrift:1] cpp.methods is incompatible with lazy deserialization in struct `Foo`\n"
            ),
        )

    def test_duplicate_field_id(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A {
                    1: i64 field1;
                    1: i64 field2;
                }

                struct B {
                    1: i64 field1;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            textwrap.dedent(
                "[ERROR:foo.thrift:3] Field id 1 for `field2` has already been used.\n"
            ),
        )

    def test_thrift_uri_uniqueness(self):
        write_file(
            "file1.thrift",
            textwrap.dedent(
                """
                struct Foo1 {
                } (thrift.uri = "facebook.com/thrift/annotation/Foo")
                """
            ),
        )

        write_file(
            "file2.thrift",
            textwrap.dedent(
                """
                struct Foo2 {
                } (thrift.uri = "facebook.com/thrift/annotation/Foo")
                struct Bar1 {
                } (thrift.uri = "facebook.com/thrift/annotation/Bar")
                """
            ),
        )

        write_file(
            "main.thrift",
            textwrap.dedent(
                """\
                include "file1.thrift"
                include "file2.thrift"
                struct Bar2 {
                } (thrift.uri = "facebook.com/thrift/annotation/Bar")
                struct Baz1 {
                } (thrift.uri = "facebook.com/thrift/annotation/Baz")
                struct Baz2 {
                } (thrift.uri = "facebook.com/thrift/annotation/Baz")
                """
            ),
        )

        ret, out, err = self.run_thrift("main.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            "\n" + err,
            textwrap.dedent(
                """
                [ERROR:file2.thrift:2] Thrift URI `facebook.com/thrift/annotation/Foo` is already defined for `Foo2`.
                [ERROR:main.thrift:3] Thrift URI `facebook.com/thrift/annotation/Bar` is already defined for `Bar2`.
                [ERROR:main.thrift:7] Thrift URI `facebook.com/thrift/annotation/Baz` is already defined for `Baz2`.
                """
            ),
        )

    def test_unique_ref(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"

                struct Bar {
                    1: optional Foo field1 (cpp.ref);

                    2: optional Foo field2 (cpp2.ref);

                    @cpp.Ref{type = cpp.RefType.Unique}
                    3: optional Foo field3;

                    @cpp.Ref{type = cpp.RefType.Shared}
                    4: optional Foo field4;

                    @cpp.Ref{type = cpp.RefType.SharedMutable}
                    5: optional Foo field5;

                    6: optional Foo field6 (cpp.ref_type = "unique");

                    7: optional Foo field7 (cpp2.ref_type = "unique");

                    8: optional Foo field8 (cpp.ref_type = "shared");

                    9: optional Foo field9 (cpp2.ref_type = "shared");

                    10: optional Foo field10 (cpp.ref = "true");

                    11: optional Foo field11 (cpp2.ref = "true");

                    12: optional Foo field12;

                    13: optional Foo field13;

                }

                struct Foo {}
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 0)
        self.assertEqual(
            "\n" + err,
            textwrap.dedent(
                """
                [WARNING:foo.thrift:4] cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field1`.
                [WARNING:foo.thrift:6] cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field2`.
                [WARNING:foo.thrift:8] @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `field3`.
                [WARNING:foo.thrift:17] cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. Please use @thrift.Box annotation instead in `field6`.
                [WARNING:foo.thrift:19] cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. Please use @thrift.Box annotation instead in `field7`.
                [WARNING:foo.thrift:25] cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field10`.
                [WARNING:foo.thrift:27] cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field11`.
                """
            ),
        )

    def test_boxed(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A {
                    1: optional i64 field (cpp.box)
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 0)
        self.assertEqual(
            "\n" + err,
            textwrap.dedent(
                """
                [WARNING:foo.thrift:2] cpp.box and thrift.box are deprecated. Please use @thrift.Box annotation instead in `field`.
                """
            ),
        )

    def test_terse_write_annotation(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"

                @thrift.Experimental
                package "apache.org/thrift/test"

                struct TerseFields {
                    @thrift.TerseWrite
                    1: i64 field1;
                    @thrift.TerseWrite
                    2: optional i64 field2;
                    @thrift.TerseWrite
                    3: required i64 field3;
                }

                union TerseUnion {
                    @thrift.TerseWrite
                    1: i64 field1;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:9] `@thrift.TerseWrite` cannot be used with qualified fields\n"
            "[ERROR:foo.thrift:11] `@thrift.TerseWrite` cannot be used with qualified fields\n"
            "[WARNING:foo.thrift:11] Required field is deprecated: `field3`.\n"
            "[ERROR:foo.thrift:16] `@thrift.TerseWrite` cannot be applied to union fields (in `TerseUnion`).\n",
        )

    def test_interaction_oneway_factory(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                interaction I {
                    void foo();
                }

                service S {
                    oneway I foo();
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:6] Oneway methods must have void return type: foo\n",
        )

    # Time complexity of for_each_transitive_field should be O(1)
    def test_time_complexity_of_for_each_transitive_field(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct S_01 { 1: i32 i; }
                struct S_02 { 1: optional S_01 a (thrift.box); 2: optional S_01 b (thrift.box); }
                struct S_03 { 1: optional S_02 a (thrift.box); 2: optional S_02 b (thrift.box); }
                struct S_04 { 1: optional S_03 a (thrift.box); 2: optional S_03 b (thrift.box); }
                struct S_05 { 1: optional S_04 a (thrift.box); 2: optional S_04 b (thrift.box); }
                struct S_06 { 1: optional S_05 a (thrift.box); 2: optional S_05 b (thrift.box); }
                struct S_07 { 1: optional S_06 a (thrift.box); 2: optional S_06 b (thrift.box); }
                struct S_08 { 1: optional S_07 a (thrift.box); 2: optional S_07 b (thrift.box); }
                struct S_09 { 1: optional S_08 a (thrift.box); 2: optional S_08 b (thrift.box); }
                struct S_10 { 1: optional S_09 a (thrift.box); 2: optional S_09 b (thrift.box); }
                struct S_11 { 1: optional S_10 a (thrift.box); 2: optional S_10 b (thrift.box); }
                struct S_12 { 1: optional S_11 a (thrift.box); 2: optional S_11 b (thrift.box); }
                struct S_13 { 1: optional S_12 a (thrift.box); 2: optional S_12 b (thrift.box); }
                struct S_14 { 1: optional S_13 a (thrift.box); 2: optional S_13 b (thrift.box); }
                struct S_15 { 1: optional S_14 a (thrift.box); 2: optional S_14 b (thrift.box); }
                struct S_16 { 1: optional S_15 a (thrift.box); 2: optional S_15 b (thrift.box); }
                struct S_17 { 1: optional S_16 a (thrift.box); 2: optional S_16 b (thrift.box); }
                struct S_18 { 1: optional S_17 a (thrift.box); 2: optional S_17 b (thrift.box); }
                struct S_19 { 1: optional S_18 a (thrift.box); 2: optional S_18 b (thrift.box); }
                struct S_20 { 1: optional S_19 a (thrift.box); 2: optional S_19 b (thrift.box); }
                struct S_21 { 1: optional S_20 a (thrift.box); 2: optional S_20 b (thrift.box); }
                struct S_22 { 1: optional S_21 a (thrift.box); 2: optional S_21 b (thrift.box); }
                struct S_23 { 1: optional S_22 a (thrift.box); 2: optional S_22 b (thrift.box); }
                struct S_24 { 1: optional S_23 a (thrift.box); 2: optional S_23 b (thrift.box); }
                struct S_25 { 1: optional S_24 a (thrift.box); 2: optional S_24 b (thrift.box); }
                struct S_26 { 1: optional S_25 a (thrift.box); 2: optional S_25 b (thrift.box); }
                struct S_27 { 1: optional S_26 a (thrift.box); 2: optional S_26 b (thrift.box); }
                struct S_28 { 1: optional S_27 a (thrift.box); 2: optional S_27 b (thrift.box); }
                struct S_29 { 1: optional S_28 a (thrift.box); 2: optional S_28 b (thrift.box); }
                struct S_30 { 1: optional S_29 a (thrift.box); 2: optional S_29 b (thrift.box); }
                struct S_31 { 1: optional S_30 a (thrift.box); 2: optional S_30 b (thrift.box); }
                struct S_32 { 1: optional S_31 a (thrift.box); 2: optional S_31 b (thrift.box); }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 0)

    def test_inject_metadata_fields_annotation(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct Fields {
                    1: i64 field1;
                    2: optional i64 field2;
                    3: required i64 field3;
                }
                """
            ),
        )

        write_file(
            "bar.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/internal.thrift"

                typedef i64 MyI64

                union UnionFields {
                    1: i64 field1;
                    2: i64 field2;
                }

                struct Fields {
                    1: i64 field1;
                    2: optional i64 field2;
                    3: required i64 field3;
                }

                @internal.InjectMetadataFields{type="foo.Fields"}
                struct Injected1 {}

                @internal.InjectMetadataFields
                struct Injected2 {}

                @internal.InjectMetadataFields{type="UnionFields"}
                struct Injected3 {}

                @internal.InjectMetadataFields{type="MyI64"}
                struct Injected4 {}

                @internal.InjectMetadataFields{type="Fields"}
                struct Injected5 {
                    1: i64 field1;
                }

                // If a field is explicitly assigned with field id 0,
                // the field id gets implicitly converted -1.
                struct BoundaryFields {
                    -1: i64 underflow;
                    1: i64 lower_boundary;
                    999: i64 upper_boundary;
                    1000: i64 overflow;
                }

                @internal.InjectMetadataFields{type="BoundaryFields"}
                struct Injected6 {}
                """
            ),
        )

        ret, out, err = self.run_thrift("bar.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[WARNING:bar.thrift:36] Nonpositive value (-1) not allowed as a field id.\n"
            "[ERROR:bar.thrift:16] Can not find expected type `foo.Fields` specified"
            " in `@internal.InjectMetadataFields` in the current scope. Please check the include.\n"
            "[ERROR:bar.thrift:19] key `type` not found.\n"
            "[ERROR:bar.thrift:22] `bar.UnionFields` is not a struct type. "
            "`@internal.InjectMetadataFields` can be only used with a struct type.\n"
            "[ERROR:bar.thrift:25] `bar.MyI64` is not a struct type. "
            "`@internal.InjectMetadataFields` can be only used with a struct type.\n"
            "[ERROR:bar.thrift:42] Field id `-1` does not mapped to valid internal id.\n"
            "[ERROR:bar.thrift:42] Field id `1000` does not mapped to valid internal id.\n"
            "[WARNING:bar.thrift:13] Required field is deprecated: `field3`.\n"
            "[ERROR:bar.thrift:11] Field `field1` is already defined for `Injected5`.\n"
            "[WARNING:bar.thrift:13] Required field is deprecated: `field3`.\n",
        )

    def test_set_invalid_elem_type(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct S {
                    1: i32 field;
                    2: set<float> set_of_float;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift", gen="hack")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] InvalidKeyType: Hack only supports integers and strings as key for map and set - https://fburl.com/wiki/pgzirbu8, field: set<float> set_of_float.\n",
        )

    def test_map_invalid_key_type(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct S {
                    1: i32 field;
                    2: map<float, i32> map_of_float_to_int;
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("foo.thrift", gen="hack")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] InvalidKeyType: Hack only supports integers and strings as key for map and set - https://fburl.com/wiki/pgzirbu8, field: map<float, i32> map_of_float_to_int.\n",
        )

    def test_rpc_return_invalid_key_type(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                service Foo {
                    set<float> invalid_rpc_return();
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("foo.thrift", gen="hack")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] InvalidKeyType: Hack only supports integers and strings as key for map and set - https://fburl.com/wiki/pgzirbu8, function invalid_rpc_return has invalid return type with type: set<float>.\n",
        )

    def test_undefined_type(self):
        write_file("header.thrift", "")
        write_file(
            "main.thrift",
            textwrap.dedent(
                """\
                include "header.thrift"
                service Foo {
                  header.Bar func();
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("main.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:main.thrift:3] Failed to resolve return type of `func`.\n"
            "[ERROR:main.thrift:3] Type `header.Bar` not defined.\n",
        )

    def test_adapting_variable(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"
                include "thrift/annotation/thrift.thrift"

                package "facebook.com/thrift/test"

                @cpp.Adapter{name="MyAdapter"}
                @scope.Transitive
                struct Config { 1: string path; }

                @Config{path = "to/my/service"}
                const i32 Foo = 10;

                @Config{path = "to/my/service"}
                const string Bar = "20";

                struct MyStruct { 1: i32 field; }

                @Config{path = "to/my/service"}
                const MyStruct Baz = MyStruct{field=30};

                @cpp.Adapter{name="MyAdapter"}
                const i32 Foo2 = 10;

                @cpp.Adapter{name="MyAdapter"}
                const string Bar2 = "20";

                @cpp.Adapter{name="MyAdapter"}
                const MyStruct Baz2 = MyStruct{field=30};
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:10] Using adapters on const `Foo` is only allowed in the experimental mode.\n"
            "[ERROR:foo.thrift:13] Using adapters on const `Bar` is only allowed in the experimental mode.\n"
            "[ERROR:foo.thrift:18] Using adapters on const `Baz` is only allowed in the experimental mode.\n"
            "[ERROR:foo.thrift:21] Using adapters on const `Foo2` is only allowed in the experimental mode.\n"
            "[ERROR:foo.thrift:24] Using adapters on const `Bar2` is only allowed in the experimental mode.\n"
            "[ERROR:foo.thrift:27] Using adapters on const `Baz2` is only allowed in the experimental mode.\n",
        )

        write_file(
            "bar.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"
                include "thrift/annotation/thrift.thrift"
                include "thrift/annotation/scope.thrift"

                @thrift.Experimental
                package "facebook.com/thrift/test"

                @cpp.Adapter{name="MyAdapter"}
                @scope.Transitive
                @thrift.Experimental
                struct Config { 1: string path; }

                @Config{path = "to/my/service"}
                const i32 Foo = 10;

                @Config{path = "to/my/service"}
                const string Bar = "20";

                struct MyStruct { 1: i32 field; }

                @Config{path = "to/my/service"}
                const MyStruct Baz = MyStruct{field=30};

                @cpp.Adapter{name="MyAdapter"}
                const i32 Foo2 = 10;

                @cpp.Adapter{name="MyAdapter"}
                const string Bar2 = "20";

                @cpp.Adapter{name="MyAdapter"}
                const MyStruct Baz2 = MyStruct{field=30};
                """
            ),
        )

        ret, out, err = self.run_thrift("bar.thrift")
        self.assertEqual(ret, 0)

    def test_rpc_param_invalid_key_type(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                service Foo {
                    void invalid_rpc_param(set<float> arg1);
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("foo.thrift", gen="hack")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] InvalidKeyType: Hack only supports integers and strings as key for map and set - https://fburl.com/wiki/pgzirbu8, function invalid_rpc_param has invalid param arg1 with type: set<float>.\n",
        )

    def test_reserved_ids(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"

                @thrift.ReserveIds{ids = [3, 8]}
                struct IdList {
                  1: i64 a;
                  3: string bad_field;
                }

                @thrift.ReserveIds{ids = [2], id_ranges = {5: 10, 15: 20}}
                struct IdRanges {
                  1: i64 a;
                  9: string bad_field;
                }

                @thrift.ReserveIds{ids = [3, 8]}
                enum EnumWithBadId {
                  A = 0,
                  B = 3,
                }

                @thrift.ReserveIds{ids = [3, 8]}
                union UnionWithBadId {
                  1: i64 a;
                  3: string bad_field;
                }

                @thrift.ReserveIds{ids = [3, 8]}
                safe exception ExceptionWithBadId {
                  1: i64 a;
                  3: string bad_field;
                }

                @thrift.ReserveIds{id_ranges = {5: 3}}
                struct InvalidIdRange {
                  1: i64 a;
                  2: string bad_field;
                }

                @thrift.ReserveIds{id_ranges = {5: 10}}
                struct OkStruct {
                  1: i64 a;
                  10: string b;
                }

                @thrift.ReserveIds{ids = [-40000, 40000], id_ranges = {-50001: -50000, 50000: 50001}}
                struct Message {
                  1: string msg;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")

        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:3] Fields in IdList cannot use reserved ids: 3\n"
            "[ERROR:foo.thrift:9] Fields in IdRanges cannot use reserved ids: 9\n"
            "[ERROR:foo.thrift:21] Fields in UnionWithBadId cannot use reserved ids: 3\n"
            "[ERROR:foo.thrift:33] For each (start: end) in id_ranges, we must have start < end. Got (5: 3), annotated on InvalidIdRange\n"
            "[ERROR:foo.thrift:45] Struct `Message` cannot have reserved id that is out of range: -50001\n"
            "[ERROR:foo.thrift:45] Struct `Message` cannot have reserved id that is out of range: -40000\n"
            "[ERROR:foo.thrift:45] Struct `Message` cannot have reserved id that is out of range: 40000\n"
            "[ERROR:foo.thrift:45] Struct `Message` cannot have reserved id that is out of range: 50000\n"
            "[ERROR:foo.thrift:27] Fields in ExceptionWithBadId cannot use reserved ids: 3\n"
            "[ERROR:foo.thrift:15] Enum values in EnumWithBadId cannot use reserved ids: 3\n",
        )

    def test_required_key_specified_in_structured_annotation(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"

                struct Foo {
                    @cpp.FieldInterceptor{name = "MyFieldInterceptor"}
                    1: i32 field1;
                    @cpp.FieldInterceptor
                    2: i32 field2;
                }

                @cpp.EnumType
                enum MyEnum1 {
                    ZERO = 0,
                }

                @cpp.EnumType{type = cpp.EnumUnderlyingType.I16}
                enum MyEnum2 {
                    ZERO = 0,
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:6] `@cpp.FieldInterceptor` cannot be used without `name` specified in `field2`.\n"
            "[ERROR:foo.thrift:10] `@cpp.EnumType` cannot be used without `type` specified in `MyEnum1`.\n",
        )

    def test_cpp_adapter_compatibility(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"

                @cpp.Adapter{name="Adapter"}
                typedef i32 Bar1 (cpp.type = "std::uint32_t")

                @cpp.Adapter{name="Adapter"}
                struct A {
                    1: i32 field;
                } (cpp.type = "CustomA")

                struct B {
                    @cpp.Adapter{name="Adapter"}
                    1: i32 field (cpp.type = "std::uint32_t");
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:6] Definition `A` cannot have both cpp.type/cpp.template and @cpp.Adapter annotations\n"
            "[ERROR:foo.thrift:12] Definition `field` cannot have both cpp.type/cpp.template and @cpp.Adapter annotations\n"
            "[ERROR:foo.thrift:3] Definition `Bar1` cannot have both cpp.type/cpp.template and @cpp.Adapter annotations\n",
        )

    def test_nonexist_type_in_variable(self):
        write_file("foo.thrift", 'const map<i8, string> foo = {1: "str"}')
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(err, "[ERROR:foo.thrift:1] Type `foo.i8` not defined.\n")

    def test_terse_write_outside_experimental_mode(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"

                package "apache.org/thrift/test"

                struct MyStruct {
                    @thrift.TerseWrite
                    1: i32 field1 = 1;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:6] Using @thrift.TerseWrite on field `field1` is only allowed in the experimental mode.\n",
        )

    def test_cycle(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct A {
                    1: B field;
                }

                struct B {
                    1: A field;
                }
                """
            ),
        )
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err,
            "[ERROR:foo.thrift:1] Cyclic dependency: A -> B -> A\n",
        )

    def test_invalid_hex_escape(self):
        write_file("foo.thrift", 'const string s = "\\x";')
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(err, "[ERROR:foo.thrift:1] invalid `\\x` escape sequence\n")

    def test_invalid_unicode_escape(self):
        write_file("foo.thrift", 'const string s = "\\u";')
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(err, "[ERROR:foo.thrift:1] invalid `\\u` escape sequence\n")

    def test_invalid_escape(self):
        write_file("foo.thrift", 'const string s = "\\*";')
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(err, "[ERROR:foo.thrift:1] invalid escape sequence `\\*`\n")

    def test_surrogate_in_unicode_escape(self):
        write_file("foo.thrift", 'const string s = "\\ud800";')
        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err, "[ERROR:foo.thrift:1] surrogate in `\\u` escape sequence\n"
        )
