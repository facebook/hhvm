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

import pathlib
import unittest

from apache.thrift.protocol.detail.protocol_detail.thrift_types import Value
from thrift.lib.python.schema.syntax_graph import (
    Annotation,
    ConstantNode,
    Definition,
    EnumNode,
    EnumTypeRef,
    EnumValue,
    ExceptionNode,
    ExceptionTypeRef,
    FieldNode,
    FieldQualifier,
    FunctionNode,
    InteractionNode,
    ListTypeRef,
    MapTypeRef,
    Primitive,
    PrimitiveTypeRef,
    ProgramNode,
    ServiceNode,
    SetTypeRef,
    StructNode,
    StructTypeRef,
    StructuredDefinition,
    SyntaxGraph,
    TypedefNode,
    TypedefTypeRef,
    TypeRef,
    UnionNode,
    UnionTypeRef,
)


class SyntaxGraphTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        schema_bytes = (pathlib.Path(__file__).parent / "test_schema.ast").read_bytes()
        cls.graph = SyntaxGraph.from_serialized_schema(schema_bytes)

    # -- Programs ---------------------------------------------

    def test_program_count(self) -> None:
        self.assertEqual(len(self.graph.programs), 4)

    def test_find_program_by_name(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        self.assertEqual(prog.path, "thrift/lib/cpp2/schema/test/syntax_graph.thrift")
        self.assertIsInstance(prog, ProgramNode)

    def test_program_namespaces(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        self.assertIn("cpp2", prog.namespaces)
        self.assertEqual(prog.namespaces["cpp2"], "apache.thrift.syntax_graph.test")

    def test_program_includes(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        include_names = [p.name for p in prog.includes]
        self.assertIn("scope", include_names)
        self.assertIn("thrift", include_names)

    def test_program_not_found(self) -> None:
        with self.assertRaises(KeyError):
            self.graph.get_program_by_name("nonexistent")

    def test_definitions_count(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        self.assertEqual(len(prog.definitions), 19)

    # -- Struct, Enum, TypeRef --------------------------------

    def test_struct_basic(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["TestStruct"]
        self.assertIsInstance(defn, StructNode)
        self.assertEqual(defn.name, "TestStruct")
        self.assertEqual(defn.uri, "")

    def test_struct_fields(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)

        fields = struct.fields
        self.assertEqual(len(fields), 2)

        self.assertEqual(fields[0].id, 1)
        self.assertEqual(fields[0].name, "field1")
        self.assertIsInstance(fields[0].type, PrimitiveTypeRef)
        self.assertEqual(fields[0].type.primitive, Primitive.I32)
        self.assertEqual(fields[0].qualifier, FieldQualifier.Default)

        self.assertEqual(fields[1].id, 2)
        self.assertEqual(fields[1].name, "field2")
        self.assertEqual(fields[1].qualifier, FieldQualifier.Optional)

    def test_enum_basic(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["TestEnum"]
        self.assertIsInstance(defn, EnumNode)
        self.assertEqual(defn.name, "TestEnum")

    def test_enum_values(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        enum = prog["TestEnum"]
        self.assertIsInstance(enum, EnumNode)

        vals = enum.values
        self.assertEqual(len(vals), 3)

        self.assertEqual(vals[0].name, "UNSET")
        self.assertEqual(vals[0].value, 0)
        self.assertEqual(vals[1].name, "VALUE_1")
        self.assertEqual(vals[1].value, 1)
        self.assertEqual(vals[2].name, "VALUE_2")
        self.assertEqual(vals[2].value, 2)

    def test_typeref_equality_primitive(self) -> None:
        t1 = PrimitiveTypeRef(Primitive.I32)
        t2 = PrimitiveTypeRef(Primitive.I32)
        t3 = PrimitiveTypeRef(Primitive.STRING)
        self.assertEqual(t1, t2)
        self.assertNotEqual(t1, t3)

    def test_typeref_equality_struct(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)

        # field2 is of type TestEnum
        enum = prog["TestEnum"]
        self.assertIsInstance(enum, EnumNode)
        self.assertEqual(struct.fields[1].type, enum.as_type())

    def test_typeref_hash(self) -> None:
        t1 = PrimitiveTypeRef(Primitive.I32)
        t2 = PrimitiveTypeRef(Primitive.I32)
        self.assertEqual(hash(t1), hash(t2))
        # Can use in sets
        s = {t1, t2}
        self.assertEqual(len(s), 1)

    def test_enum_value_isinstance(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        enum = prog["TestEnum"]
        self.assertIsInstance(enum, EnumNode)
        for val in enum.values:
            self.assertIsInstance(val, EnumValue)

    def test_field_isinstance(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)
        for field in struct.fields:
            self.assertIsInstance(field, FieldNode)

    def test_isinstance_dispatch(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["TestStruct"]
        self.assertIsInstance(defn, StructNode)
        self.assertIsInstance(defn, StructuredDefinition)
        self.assertIsInstance(defn, Definition)
        self.assertNotIsInstance(defn, EnumNode)

    def test_program_contains(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        self.assertIn("TestStruct", prog)
        self.assertNotIn("Nonexistent", prog)

    def test_program_getitem_keyerror(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        with self.assertRaises(KeyError):
            prog["Nonexistent"]

    # -- Union, Exception, Typedef, Constant, Containers ------

    def test_union(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["TestUnion"]
        self.assertIsInstance(defn, UnionNode)
        self.assertEqual(defn.name, "TestUnion")

        fields = defn.fields
        self.assertEqual(len(fields), 2)

        self.assertEqual(fields[0].id, 1)
        self.assertEqual(fields[0].name, "s")
        self.assertIsInstance(fields[0].type, StructTypeRef)
        self.assertEqual(fields[0].type.node.name, "TestStruct")

        self.assertEqual(fields[1].id, 2)
        self.assertEqual(fields[1].name, "e")
        self.assertIsInstance(fields[1].type, EnumTypeRef)

    def test_union_as_type(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        union = prog["TestUnion"]
        self.assertIsInstance(union, UnionNode)

        ref = union.as_type()
        self.assertIsInstance(ref, UnionTypeRef)
        self.assertEqual(ref.node.name, "TestUnion")

    def test_union_typeref_field(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["StructWithCustomDefault"]
        self.assertIsInstance(struct, StructNode)

        # field8 is of type TestUnion
        field8 = struct.fields[7]
        self.assertEqual(field8.name, "field8")
        self.assertIsInstance(field8.type, UnionTypeRef)
        self.assertEqual(field8.type.node.name, "TestUnion")

    def test_exception(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["TestException"]
        self.assertIsInstance(defn, ExceptionNode)
        self.assertEqual(defn.uri, "meta.com/thrift_test/TestException")

        fields = defn.fields
        self.assertEqual(len(fields), 2)

        self.assertEqual(fields[0].id, 1)
        self.assertEqual(fields[0].name, "blob")
        self.assertIsInstance(fields[0].type, PrimitiveTypeRef)
        self.assertEqual(fields[0].type.primitive, Primitive.BINARY)

        self.assertEqual(fields[1].id, 2)
        self.assertEqual(fields[1].name, "s")
        self.assertIsInstance(fields[1].type, StructTypeRef)
        self.assertEqual(fields[1].type.node.name, "TestRecursiveStruct")

    def test_exception_as_type(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        exc = prog["TestException"]
        self.assertIsInstance(exc, ExceptionNode)

        ref = exc.as_type()
        self.assertIsInstance(ref, ExceptionTypeRef)
        self.assertEqual(ref.node.name, "TestException")

    def test_typedef(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["ListOfTestStruct"]
        self.assertIsInstance(defn, TypedefNode)
        self.assertEqual(defn.name, "ListOfTestStruct")

        target = defn.target_type
        self.assertIsInstance(target, ListTypeRef)

        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)
        self.assertEqual(target.element_type, struct.as_type())

    def test_typedef_chain_true_type(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        typedef1 = prog["ListOfTestStruct"]
        typedef2 = prog["TypedefToListOfTestStruct"]
        self.assertIsInstance(typedef1, TypedefNode)
        self.assertIsInstance(typedef2, TypedefNode)

        # TypedefToListOfTestStruct -> ListOfTestStruct -> list<TestStruct>
        ref2 = typedef2.as_type()
        self.assertIsInstance(ref2, TypedefTypeRef)
        true = ref2.true_type
        self.assertIsInstance(true, ListTypeRef)

        # Both typedefs resolve to the same underlying type
        self.assertEqual(ref2.true_type, typedef1.target_type)

    def test_constant(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["testConst"]
        self.assertIsInstance(defn, ConstantNode)
        self.assertEqual(defn.name, "testConst")

        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)
        self.assertEqual(defn.type, struct.as_type())

    def test_constant_value(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["testConst"]
        self.assertIsInstance(defn, ConstantNode)
        val = defn.value
        self.assertIsInstance(val, Value)
        self.assertIsNotNone(val.objectValue)

    def test_constant_nested_value(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["testNestedConst"]
        self.assertIsInstance(defn, ConstantNode)
        val = defn.value
        self.assertIsInstance(val, Value)
        self.assertIsNotNone(val.objectValue)

    def test_constant_as_type_raises(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["testConst"]
        self.assertIsInstance(defn, ConstantNode)
        with self.assertRaises(TypeError):
            defn.as_type()

    # -- Field Custom Defaults --------------------------------

    def test_field_default_primitives(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["StructWithCustomDefault"]
        self.assertIsInstance(struct, StructNode)

        field1 = struct.fields[0]
        self.assertEqual(field1.name, "field1")
        self.assertIsInstance(field1.default_value, Value)
        self.assertEqual(field1.default_value.boolValue, True)

        field2 = struct.fields[1]
        self.assertEqual(field2.name, "field2")
        self.assertIsInstance(field2.default_value, Value)
        self.assertEqual(field2.default_value.i32Value, 10)

        field3 = struct.fields[2]
        self.assertEqual(field3.name, "field3")
        self.assertIsInstance(field3.default_value, Value)
        self.assertEqual(field3.default_value.stringValue, b"foo")

    def test_field_default_containers(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["StructWithCustomDefault"]
        self.assertIsInstance(struct, StructNode)

        field5 = struct.fields[4]
        self.assertEqual(field5.name, "field5")
        self.assertIsInstance(field5.default_value, Value)
        self.assertEqual(len(field5.default_value.listValue), 1)

        field6 = struct.fields[5]
        self.assertEqual(field6.name, "field6")
        self.assertIsInstance(field6.default_value, Value)
        self.assertEqual(len(field6.default_value.setValue), 3)

    def test_field_default_map(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["StructWithCustomDefault"]
        self.assertIsInstance(struct, StructNode)

        field7 = struct.fields[6]
        self.assertEqual(field7.name, "field7")
        self.assertIsInstance(field7.default_value, Value)
        self.assertEqual(len(field7.default_value.mapValue), 1)

    def test_field_default_struct(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["StructWithCustomDefault"]
        self.assertIsInstance(struct, StructNode)

        field8 = struct.fields[7]
        self.assertEqual(field8.name, "field8")
        self.assertIsInstance(field8.default_value, Value)
        self.assertIsNotNone(field8.default_value.objectValue)

    def test_field_with_default_on_simple_struct(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)

        field1 = struct.fields[0]
        self.assertEqual(field1.name, "field1")
        self.assertIsInstance(field1.default_value, Value)
        self.assertEqual(field1.default_value.i32Value, 10)

    def test_field_no_default(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)

        field2 = struct.fields[1]
        self.assertEqual(field2.name, "field2")
        self.assertIsNone(field2.default_value)

    def test_container_typeref_list(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        typedef = prog["ListOfTestStruct"]
        self.assertIsInstance(typedef, TypedefNode)
        target = typedef.target_type
        self.assertIsInstance(target, ListTypeRef)

        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)
        expected = ListTypeRef(element_type=struct.as_type())
        self.assertEqual(target, expected)

    def test_container_typeref_equality(self) -> None:
        prim = PrimitiveTypeRef(Primitive.I32)
        l1 = ListTypeRef(element_type=prim)
        l2 = ListTypeRef(element_type=PrimitiveTypeRef(Primitive.I32))
        self.assertEqual(l1, l2)
        self.assertEqual(hash(l1), hash(l2))

        s1 = SetTypeRef(element_type=prim)
        s2 = SetTypeRef(element_type=prim)
        self.assertEqual(s1, s2)

        m1 = MapTypeRef(
            key_type=PrimitiveTypeRef(Primitive.STRING),
            value_type=prim,
        )
        m2 = MapTypeRef(
            key_type=PrimitiveTypeRef(Primitive.STRING),
            value_type=PrimitiveTypeRef(Primitive.I32),
        )
        self.assertEqual(m1, m2)

    # -- Cyclic References ------------------------------------

    def test_recursive_struct(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["TestRecursiveStruct"]
        self.assertIsInstance(defn, StructNode)

        self.assertEqual(len(defn.fields), 1)
        field = defn.fields[0]
        # The field type should reference back to the same struct
        self.assertEqual(field.type, defn.as_type())

    def test_field_parent(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)

        for field in struct.fields:
            self.assertIs(field.parent, struct)

    # -- Annotations ------------------------------------------

    def test_enum_annotation(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        enum = prog["TestEnum"]
        self.assertIsInstance(enum, EnumNode)

        # TestEnum has @TestStructuredAnnotation{field1 = 3}
        annot = self._find_annotation_by_type_name(
            enum.annotations, "TestStructuredAnnotation"
        )
        self.assertIsNotNone(annot)
        self.assertEqual(annot.value["field1"], 3)

    def test_enum_value_annotation(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        enum = prog["TestEnum"]
        self.assertIsInstance(enum, EnumNode)

        unset = enum.values[0]
        self.assertEqual(unset.name, "UNSET")
        annot = self._find_annotation_by_type_name(
            unset.annotations, "TestStructuredAnnotation"
        )
        self.assertIsNotNone(annot)
        self.assertEqual(annot.value["field1"], 4)

    def test_union_annotations_count(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        union = prog["TestUnion"]
        self.assertIsInstance(union, UnionNode)
        self.assertEqual(len(union.annotations), 3)

    def test_nested_annotation_value(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        union = prog["TestUnion"]
        self.assertIsInstance(union, UnionNode)

        annot = self._find_annotation_by_type_name(
            union.annotations, "TestStructuredAnnotation"
        )
        self.assertIsNotNone(annot)
        self.assertEqual(annot.value["field1"], 3)
        # Nested struct value
        self.assertIsInstance(annot.value["field2"], dict)
        self.assertEqual(annot.value["field2"]["field1"], 4)

    def test_field_annotation(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        exc = prog["TestException"]
        self.assertIsInstance(exc, ExceptionNode)

        blob_field = exc.fields[0]
        self.assertEqual(blob_field.name, "blob")
        self.assertEqual(len(blob_field.annotations), 1)

    def test_annotation_type_resolution(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        enum = prog["TestEnum"]
        self.assertIsInstance(enum, EnumNode)

        for annot in enum.annotations:
            # The annotation type should resolve to a TypeRef
            self.assertIsInstance(annot.type, TypeRef)
            # And following true_type should give a struct
            true = annot.type.true_type
            self.assertIsInstance(true, StructTypeRef)

    # -- Services & Interactions ------------------------------

    def test_service_basic(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)
        self.assertIsNone(svc.base_service)

    def test_service_functions_count(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)
        self.assertEqual(len(svc.functions), 6)

    def test_service_foo_function(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)

        foo = svc.functions[0]
        self.assertEqual(foo.name, "foo")

        # Return type is TestStruct
        struct = prog["TestStruct"]
        self.assertIsInstance(struct, StructNode)
        self.assertEqual(foo.response.type, struct.as_type())

        # No stream/sink/interaction
        self.assertIsNone(foo.response.sink)
        self.assertIsNone(foo.response.stream)

        # One parameter
        self.assertEqual(len(foo.params), 1)
        self.assertEqual(foo.params[0].id, 1)
        self.assertEqual(foo.params[0].type, PrimitiveTypeRef(Primitive.I32))

    def test_service_no_return(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)

        no_return = svc.functions[5]
        self.assertEqual(no_return.name, "noReturn")
        self.assertIsNone(no_return.response.type)

    def test_service_stream(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)

        create_stream = svc.functions[2]
        self.assertEqual(create_stream.name, "createStream")
        self.assertEqual(create_stream.response.type, PrimitiveTypeRef(Primitive.I32))

        stream = create_stream.response.stream
        self.assertIsNotNone(stream)
        self.assertEqual(stream.payload_type, PrimitiveTypeRef(Primitive.I32))

    def test_service_interaction(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)

        create_interaction = svc.functions[1]
        self.assertEqual(create_interaction.name, "createInteraction")
        self.assertIsNone(create_interaction.response.type)

    def test_service_inheritance(self) -> None:
        prog2 = self.graph.get_program_by_name("syntax_graph_2")
        svc2 = prog2["TestService2"]
        self.assertIsInstance(svc2, ServiceNode)

        base = svc2.base_service
        self.assertIsNotNone(base)
        self.assertEqual(base.name, "TestService")

    def test_interaction(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["TestInteraction"]
        self.assertIsInstance(defn, InteractionNode)

        funcs = defn.functions
        self.assertEqual(len(funcs), 1)
        self.assertEqual(funcs[0].name, "foo")
        self.assertEqual(funcs[0].response.type, PrimitiveTypeRef(Primitive.I32))

        # Param type is TestRecursiveStruct
        self.assertEqual(len(funcs[0].params), 1)
        self.assertEqual(funcs[0].params[0].name, "input")
        recursive = prog["TestRecursiveStruct"]
        self.assertIsInstance(recursive, StructNode)
        self.assertEqual(funcs[0].params[0].type, recursive.as_type())

    def test_function_parent(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)

        for func in svc.functions:
            self.assertIsInstance(func.parent, ServiceNode)
            self.assertIs(func.parent, svc)

    def test_function_isinstance(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)
        for func in svc.functions:
            self.assertIsInstance(func, FunctionNode)

    def test_function_exception_typeref(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["TestService"]
        self.assertIsInstance(svc, ServiceNode)

        foo = svc.functions[0]
        self.assertEqual(foo.name, "foo")
        self.assertEqual(len(foo.exceptions), 1)
        self.assertIsInstance(foo.exceptions[0].type, ExceptionTypeRef)
        self.assertEqual(foo.exceptions[0].type.node.name, "TestException")

    # -- Doc Blocks & repr ------------------------------------

    def test_doc_block_struct(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["StructWithDocBlock"]
        self.assertIsInstance(defn, StructNode)

        self.assertIsNotNone(defn.doc_block)
        self.assertIn("This is a docblock for testing purposes", defn.doc_block)
        self.assertIn("It spans multiple lines", defn.doc_block)

    def test_doc_block_field(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        defn = prog["StructWithDocBlock"]
        self.assertIsInstance(defn, StructNode)

        field = defn.fields[0]
        self.assertIsNotNone(field.doc_block)
        self.assertIn("Field documentation", field.doc_block)

    def test_doc_block_service(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["ServiceWithDocBlock"]
        self.assertIsInstance(svc, ServiceNode)

        self.assertIsNotNone(svc.doc_block)
        self.assertIn("A dummy service for testing docblock exposure", svc.doc_block)

    def test_doc_block_function(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        svc = prog["ServiceWithDocBlock"]
        self.assertIsInstance(svc, ServiceNode)

        ping = svc.functions[0]
        self.assertEqual(ping.name, "ping")
        self.assertIsNotNone(ping.doc_block)
        self.assertIn("Performs a ping operation", ping.doc_block)

    def test_repr(self) -> None:
        prog = self.graph.get_program_by_name("syntax_graph")
        struct = prog["TestStruct"]
        self.assertIn("TestStruct", repr(struct))
        self.assertIn("StructNode", repr(struct))

    # -- Helpers -----------------------------------------------------------

    def _find_annotation_by_type_name(
        self, annotations: list[Annotation], type_name: str
    ) -> Annotation | None:
        for annot in annotations:
            ref = annot.type.true_type
            if isinstance(ref, StructTypeRef) and ref.node.name == type_name:
                return annot
        return None


if __name__ == "__main__":
    unittest.main()
