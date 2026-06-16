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

import unittest

# Importing the fixture module makes its URIs discoverable by the registry.
import thrift.lib.python.schema.tests.type_system_bridge_test.thrift_types  # noqa: F401
from thrift.lib.python.schema import syntax_graph as _ast
from thrift.lib.python.schema._record import (
    BoolRecord,
    FieldSetRecord,
    Int32Record,
    ListRecord,
    TextRecord,
)
from thrift.lib.python.schema.schema_registry import SchemaRegistry
from thrift.lib.python.schema.type_system import (
    EnumNode,
    InvalidTypeError,
    ListTypeRef,
    OpaqueAliasNode,
    OpaqueAliasTypeRef,
    PresenceQualifier,
    Primitive,
    PrimitiveTypeRef,
    StructNode,
    StructTypeRef,
    TypeRef,
    UnionNode,
)
from thrift.lib.python.schema.type_system_bridge import SyntaxGraphBridge

_URI = "thrift.com/python/schema/ts_bridge"


def _field_type(node: StructNode | UnionNode, name: str) -> TypeRef:
    field = node.field_by_name(name)
    assert field is not None, f"missing field {name!r}"
    return field.type


class TypeSystemBridgeTest(unittest.TestCase):
    def setUp(self) -> None:
        SchemaRegistry._reset()
        self.registry = SchemaRegistry()

    def _struct(self, name: str) -> StructNode:
        node = self.registry.get_user_defined_type(f"{_URI}/{name}")
        assert isinstance(node, StructNode), f"{name} is not a StructNode: {node!r}"
        return node

    # -- Basic resolution ---------------------------------------------------

    def test_bridge_struct_with_resolved_fields(self) -> None:
        outer = self._struct("Outer")
        inner = self._struct("Inner")

        # Struct reference resolves to the (same, memoized) Inner node.
        inner_edge = _field_type(outer, "inner")
        assert isinstance(inner_edge, StructTypeRef)
        self.assertIs(inner_edge.node, inner)

        # Primitive field on Inner.
        value_edge = _field_type(inner, "value")
        self.assertEqual(value_edge, PrimitiveTypeRef(Primitive.I32))

        # Container of a struct reference.
        inners_edge = _field_type(outer, "inners")
        assert isinstance(inners_edge, ListTypeRef)
        element = inners_edge.element_type
        assert isinstance(element, StructTypeRef)
        self.assertEqual(element.node.uri, f"{_URI}/Inner")

    def test_get_known_uris_is_none(self) -> None:
        self.assertIsNone(self.registry.get_known_uris())

    def test_get_user_defined_type_or_throw(self) -> None:
        outer = self.registry.get_user_defined_type_or_throw(f"{_URI}/Outer")
        self.assertIsInstance(outer, StructNode)
        with self.assertRaises(InvalidTypeError):
            self.registry.get_user_defined_type_or_throw("does.not/Exist")

    def test_unknown_uri_returns_none(self) -> None:
        self.assertIsNone(self.registry.get_user_defined_type("does.not/Exist"))

    # -- Typedef erasure ----------------------------------------------------

    def test_typedef_to_primitive_is_erased(self) -> None:
        outer = self._struct("Outer")
        # `1: MyId id` where `typedef i64 MyId` -> bare I64 primitive.
        id_edge = _field_type(outer, "id")
        self.assertEqual(id_edge, PrimitiveTypeRef(Primitive.I64))

    def test_typedef_to_struct_is_erased(self) -> None:
        outer = self._struct("Outer")
        # `3: InnerAlias aliased` where `typedef Inner InnerAlias` -> Inner node.
        aliased_edge = _field_type(outer, "aliased")
        assert isinstance(aliased_edge, StructTypeRef)
        self.assertEqual(aliased_edge.node.uri, f"{_URI}/Inner")

    def test_typedef_is_not_surfaced_as_node(self) -> None:
        # Typedefs are not TypeSystem user-defined types (no OpaqueAliasNode).
        self.assertIsNone(self.registry.get_user_defined_type(f"{_URI}/MyId"))
        self.assertIsNone(self.registry.get_user_defined_type(f"{_URI}/InnerAlias"))
        outer = self._struct("Outer")
        self.assertNotIsInstance(_field_type(outer, "id"), OpaqueAliasTypeRef)

    def test_no_opaque_alias_nodes_from_bridge(self) -> None:
        # Nothing the bridge produces is an OpaqueAliasNode.
        for name in ("Outer", "Inner", "MyError", "MyUnion", "Presence"):
            node = self.registry.get_user_defined_type(f"{_URI}/{name}")
            self.assertNotIsInstance(node, OpaqueAliasNode)

    # -- Exception / union --------------------------------------------------

    def test_exception_bridges_to_struct(self) -> None:
        err = self.registry.get_user_defined_type(f"{_URI}/MyError")
        self.assertIsInstance(err, StructNode)
        assert isinstance(err, StructNode)
        self.assertEqual(_field_type(err, "code"), PrimitiveTypeRef(Primitive.I32))

    def test_union_forces_all_fields_optional(self) -> None:
        union = self.registry.get_user_defined_type(f"{_URI}/MyUnion")
        assert isinstance(union, UnionNode)
        self.assertEqual(len(union.fields), 2)
        for field in union.fields:
            self.assertEqual(field.presence, PresenceQualifier.OPTIONAL)

    # -- Presence (terse fidelity regression) -------------------------------

    def test_presence_mapping_including_terse(self) -> None:
        presence = self._struct("Presence")
        by_name = {f.identity.name: f.presence for f in presence.fields}
        self.assertEqual(by_name["plain"], PresenceQualifier.UNQUALIFIED)
        self.assertEqual(by_name["maybe"], PresenceQualifier.OPTIONAL)
        # Regression: terse must NOT collapse to UNQUALIFIED.
        self.assertEqual(by_name["terse"], PresenceQualifier.TERSE)

    # -- Cycles + memoization ----------------------------------------------

    def test_self_recursive_struct(self) -> None:
        node = self._struct("SelfRef")
        next_edge = _field_type(node, "next")
        assert isinstance(next_edge, StructTypeRef)
        self.assertIs(next_edge.node, node)

    def test_mutually_recursive_structs(self) -> None:
        m1 = self._struct("Mutual1")
        m2 = self._struct("Mutual2")

        other_edge = _field_type(m1, "other")
        assert isinstance(other_edge, StructTypeRef)
        self.assertIs(other_edge.node, m2)

        back_edge = _field_type(m2, "back")
        assert isinstance(back_edge, StructTypeRef)
        self.assertIs(back_edge.node, m1)

    def test_repeated_lookup_is_memoized(self) -> None:
        first = self.registry.get_user_defined_type(f"{_URI}/Outer")
        second = self.registry.get_user_defined_type(f"{_URI}/Outer")
        self.assertIs(first, second)

    # -- IDL `any` (resolved via the bundled omnibus schema) ----------------

    def test_any_field_bridges_to_any_primitive(self) -> None:
        # `1: any.Any any_field`. AnyStruct's definition isn't in the fixture's
        # own schema blob (core libs ship none) -- it resolves only because the
        # registry seeds the bundled omnibus schema.
        has_any = self._struct("HasAny")
        any_edge = _field_type(has_any, "any_field")
        # AnyStruct is abstracted into the ANY primitive, not a struct node.
        self.assertEqual(any_edge, PrimitiveTypeRef(Primitive.ANY))
        self.assertNotIsInstance(any_edge, StructTypeRef)

    # -- Enum ---------------------------------------------------------------

    def test_enum_bridges_with_values(self) -> None:
        node = self.registry.get_user_defined_type(f"{_URI}/Color")
        assert isinstance(node, EnumNode)
        by_name = {v.name: v.datum for v in node.values}
        self.assertEqual(by_name, {"RED": 0, "GREEN": 1, "BLUE": 2})

    def test_enum_value_annotations_populated(self) -> None:
        node = self.registry.get_user_defined_type(f"{_URI}/AnnotatedEnum")
        assert isinstance(node, EnumNode)
        by_name = {v.name: v for v in node.values}

        record = by_name["FIRST"].annotations.get(f"{_URI}/RecordAnno")
        assert isinstance(record, FieldSetRecord), (
            f"missing enum-value annotation: {by_name['FIRST'].annotations!r}"
        )
        # `count` (field id 1) and `label` (field id 2), re-resolved name -> id.
        self.assertEqual(record.fields[1], Int32Record(11))
        self.assertEqual(record.fields[2], TextRecord("first"))

        # A value with no annotation keeps an empty map.
        self.assertEqual(by_name["SECOND"].annotations, {})

    # -- Annotations + custom defaults ---------------------------------

    def test_node_annotations_populated(self) -> None:
        annotated = self._struct("Annotated")
        record = annotated.annotations.get(f"{_URI}/RecordAnno")
        assert isinstance(record, FieldSetRecord), (
            f"missing annotation: {annotated.annotations!r}"
        )
        # `count` (field id 1) and `label` (field id 2) re-resolved name -> id.
        self.assertEqual(record.fields[1], Int32Record(7))
        self.assertEqual(record.fields[2], TextRecord("outer"))

    def test_field_annotations_populated(self) -> None:
        annotated = self._struct("Annotated")
        field = annotated.field_by_name("value")
        assert field is not None
        record = field.annotations.get(f"{_URI}/RecordAnno")
        assert isinstance(record, FieldSetRecord), (
            f"missing annotation: {field.annotations!r}"
        )
        self.assertEqual(record.fields, {1: Int32Record(3), 2: TextRecord("field")})

    def test_field_without_annotation_has_empty_map(self) -> None:
        inner = self._struct("Inner")
        value = inner.field_by_name("value")
        assert value is not None
        self.assertEqual(value.annotations, {})

    def test_custom_defaults_converted(self) -> None:
        has_defaults = self._struct("HasDefaults")
        expected = {
            "num": Int32Record(42),
            "label": TextRecord("hello"),
            "nums": ListRecord([Int32Record(1), Int32Record(2), Int32Record(3)]),
            "flag": BoolRecord(True),
        }
        for name, want in expected.items():
            field = has_defaults.field_by_name(name)
            assert field is not None
            with self.subTest(field=name):
                self.assertEqual(field.custom_default, want)

    def test_field_without_default_is_none(self) -> None:
        inner = self._struct("Inner")
        value = inner.field_by_name("value")
        assert value is not None
        self.assertIsNone(value.custom_default)


class _StubResolver:
    """Minimal ``SyntaxResolver``: serves pre-built AST definitions by URI."""

    def __init__(self, by_uri: dict[str, object]) -> None:
        self._by_uri = by_uri

    def definition_by_uri(self, uri: str) -> object | None:
        return self._by_uri.get(uri)


def _build_ast_struct(
    resolver: _ast._Resolver,
    *,
    uri: str,
    name: str,
    key: bytes,
    fields: list[_ast.FieldNode] | None = None,
) -> _ast.StructNode:
    """Construct and register a minimal AST ``StructNode`` for bridge tests."""
    node = _ast.StructNode(
        uri=uri,
        fields=fields or [],
        name=name,
        definition_key=key,
        resolver=resolver,
        doc_block=None,
        annotations=[],
    )
    resolver.register(key, node)
    return node


def _struct_field(name: str, type_ref: _ast.TypeRef) -> _ast.FieldNode:
    return _ast.FieldNode(
        id=1,
        name=name,
        type=type_ref,
        qualifier=_ast.FieldQualifier.Default,
        doc_block=None,
        annotations=[],
    )


class UriLessBridgeRejectionTest(unittest.TestCase):
    """A URI-less type reached transitively as a field type must fail loudly at
    bridge time."""

    def test_field_referencing_uri_less_struct_fails_at_bridge_time(self) -> None:
        resolver = _ast._Resolver()
        uri_less = _build_ast_struct(resolver, uri="", name="UriLess", key=b"k-uriless")
        referencer = _build_ast_struct(
            resolver,
            uri="test.com/Referencer",
            name="Referencer",
            key=b"k-referencer",
            fields=[_struct_field("bad", uri_less.as_type())],
        )
        bridge = SyntaxGraphBridge(_StubResolver({"test.com/Referencer": referencer}))

        with self.assertRaises(InvalidTypeError) as ctx:
            bridge.get_user_defined_type("test.com/Referencer")
        message = str(ctx.exception)
        self.assertIn("Referencer.bad", message)  # names the referencing field
        self.assertIn("URI-less", message)  # flags the cause

        # A retry must re-raise, not return the half-built (field-less)
        # placeholder cached during the failed first attempt.
        with self.assertRaises(InvalidTypeError):
            bridge.get_user_defined_type("test.com/Referencer")

    def test_uri_less_struct_reached_through_container_field(self) -> None:
        # The referencing-field context is threaded through container edges too.
        resolver = _ast._Resolver()
        hidden = _build_ast_struct(resolver, uri="", name="Hidden", key=b"k-hidden")
        holder = _build_ast_struct(
            resolver,
            uri="test.com/Holder",
            name="Holder",
            key=b"k-holder",
            fields=[_struct_field("items", _ast.ListTypeRef(hidden.as_type()))],
        )
        bridge = SyntaxGraphBridge(_StubResolver({"test.com/Holder": holder}))

        with self.assertRaises(InvalidTypeError) as ctx:
            bridge.get_user_defined_type("test.com/Holder")
        self.assertIn("Holder.items", str(ctx.exception))

    def test_failed_build_rolls_back_cyclic_sibling(self) -> None:
        resolver = _ast._Resolver()
        uri_less = _build_ast_struct(resolver, uri="", name="Bad", key=b"k-bad")
        b = _build_ast_struct(
            resolver,
            uri="test.com/B",
            name="B",
            key=b"k-b",
            # Reference A by key: A does not exist yet (the lazy ref breaks the
            # construction cycle and resolves once A is registered below).
            fields=[
                _struct_field("a", _ast.StructTypeRef(_ast._Lazy(resolver, b"k-a")))
            ],
        )
        a = _build_ast_struct(
            resolver,
            uri="test.com/A",
            name="A",
            key=b"k-a",
            fields=[
                _struct_field("b", b.as_type()),
                _struct_field("bad", uri_less.as_type()),
            ],
        )
        bridge = SyntaxGraphBridge(_StubResolver({"test.com/A": a, "test.com/B": b}))

        # Bridging A fails on its URI-less field.
        with self.assertRaises(InvalidTypeError):
            bridge.get_user_defined_type("test.com/A")

        # B transitively depends on A, so looking B up must ALSO raise -- not
        # return a cached B whose `a` edge points at a field-less A placeholder
        # left behind by the failed build.
        with self.assertRaises(InvalidTypeError):
            bridge.get_user_defined_type("test.com/B")


class BridgeSourceInfoTest(unittest.TestCase):
    """The bridge threads provenance from the AST: each bridged node's
    ``source_info`` is ``{"file://" + program.path, definition.name}``, or ``None``
     when the AST carries no program."""

    def setUp(self) -> None:
        SchemaRegistry._reset()
        self.registry = SchemaRegistry()

    def _expected_locator(self, name: str) -> str:
        ast_def = self.registry.get_definition_by_uri(f"{_URI}/{name}")
        return "file://" + ast_def.program.path

    def test_struct_source_info(self) -> None:
        node = self.registry.get_user_defined_type(f"{_URI}/Outer")
        assert node is not None
        info = node.source_info
        assert info is not None, "expected source_info on a bridged struct"
        self.assertEqual(info.locator, self._expected_locator("Outer"))
        self.assertEqual(info.name, "Outer")

    def test_union_source_info(self) -> None:
        node = self.registry.get_user_defined_type(f"{_URI}/MyUnion")
        assert node is not None
        info = node.source_info
        assert info is not None, "expected source_info on a bridged union"
        self.assertEqual(info.locator, self._expected_locator("MyUnion"))
        self.assertEqual(info.name, "MyUnion")

    def test_enum_source_info(self) -> None:
        node = self.registry.get_user_defined_type(f"{_URI}/Color")
        assert node is not None
        info = node.source_info
        assert info is not None, "expected source_info on a bridged enum"
        self.assertEqual(info.locator, self._expected_locator("Color"))
        self.assertEqual(info.name, "Color")

    def test_locator_uses_file_scheme_and_real_path(self) -> None:
        # Guards the exact "file://" + program.path shape against a non-empty
        # fixture path.
        node = self.registry.get_user_defined_type(f"{_URI}/Inner")
        assert node is not None
        info = node.source_info
        assert info is not None
        self.assertTrue(info.locator.startswith("file://"))
        self.assertTrue(info.locator.endswith("type_system_bridge_test.thrift"))

    def test_source_info_none_when_ast_has_no_program(self) -> None:
        # A bridged node whose AST definition has no linked program (e.g. a
        # programmatically built AST node, as the _StubResolver tests use)
        # carries no source_info -- and must not crash on the asserting
        # `Definition.program` accessor.
        resolver = _ast._Resolver()
        no_program = _build_ast_struct(
            resolver, uri="test.com/NoProgram", name="NoProgram", key=b"k-noprog"
        )
        bridge = SyntaxGraphBridge(_StubResolver({"test.com/NoProgram": no_program}))
        node = bridge.get_user_defined_type("test.com/NoProgram")
        assert node is not None
        self.assertIsNone(node.source_info)
