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

import sys
import unittest

import thrift.lib.python.schema.tests.schema_registry_any.thrift_types as AnyModule
import thrift.lib.python.schema.tests.schema_registry_dep.thrift_types as DepModule
import thrift.lib.python.schema.tests.schema_registry_legacy_uri.thrift_clients as LegacyClients
import thrift.lib.python.schema.tests.schema_registry_legacy_uri.thrift_services as LegacyServices
import thrift.lib.python.schema.tests.schema_registry_legacy_uri.thrift_types as LegacyModule
import thrift.lib.python.schema.tests.schema_registry_test.thrift_types as TestModule
from thrift.lib.python.schema.schema_registry import SchemaRegistry
from thrift.lib.python.schema.syntax_graph import (
    EnumNode,
    ExceptionNode,
    InteractionNode,
    ServiceNode,
    StructNode,
    StructTypeRef,
    UnionNode,
)


class SchemaRegistryTest(unittest.TestCase):
    def setUp(self) -> None:
        SchemaRegistry._reset()

    # -- Single-module get_node ---------------------------------

    def test_get_node_struct(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(TestModule.MyStruct)
        assert isinstance(defn, StructNode)
        self.assertEqual(defn.name, "MyStruct")
        self.assertEqual(defn.uri, "thrift.com/python/schema/registry_test/MyStruct")

    def test_get_node_enum(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(TestModule.MyEnum)
        assert isinstance(defn, EnumNode)
        self.assertEqual(defn.name, "MyEnum")
        self.assertEqual(defn.uri, "thrift.com/python/schema/registry_test/MyEnum")

    def test_get_node_union(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(TestModule.MyUnion)
        assert isinstance(defn, UnionNode)
        self.assertEqual(defn.name, "MyUnion")
        self.assertEqual(defn.uri, "thrift.com/python/schema/registry_test/MyUnion")

    def test_get_node_exception(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(TestModule.MyException)
        assert isinstance(defn, ExceptionNode)
        self.assertEqual(defn.name, "MyException")
        self.assertEqual(defn.uri, "thrift.com/python/schema/registry_test/MyException")

    def test_get_node_caches_result(self) -> None:
        registry = SchemaRegistry()
        defn1 = registry.get_node(TestModule.MyStruct)
        defn2 = registry.get_node(TestModule.MyStruct)
        self.assertIs(defn1, defn2)

    # -- URI lookup ---------------------------------------------

    def test_get_definition_by_uri_enum(self) -> None:
        registry = SchemaRegistry()
        uri = "thrift.com/python/schema/registry_test/MyEnum"
        defn = registry.get_definition_by_uri(uri)
        self.assertIsInstance(defn, EnumNode)
        self.assertEqual(defn.name, "MyEnum")

    def test_get_definition_by_uri_struct(self) -> None:
        registry = SchemaRegistry()
        uri = "thrift.com/python/schema/registry_test/MyStruct"
        defn = registry.get_definition_by_uri(uri)
        self.assertIsInstance(defn, StructNode)
        self.assertEqual(defn.name, "MyStruct")

    def test_get_definition_by_uri_unknown(self) -> None:
        registry = SchemaRegistry()
        with self.assertRaises(KeyError):
            registry.get_definition_by_uri("unknown.com/does/not/Exist")

    # -- Multi-module merge -------------------------------------

    def test_multi_module_merge(self) -> None:
        registry = SchemaRegistry()
        # Register types from both modules
        defn_main = registry.get_node(TestModule.MyStruct)
        defn_dep = registry.get_node(DepModule.DepStruct)
        self.assertEqual(defn_main.name, "MyStruct")
        self.assertEqual(defn_dep.name, "DepStruct")
        # Both should be reachable from the same syntax graph as the same objects
        graph = registry.syntax_graph
        main_prog = graph.get_program_by_name("schema_registry_test")
        dep_prog = graph.get_program_by_name("schema_registry_dep")
        self.assertIs(main_prog["MyStruct"], defn_main)
        self.assertIs(dep_prog["DepStruct"], defn_dep)

    def test_cross_module_field_type(self) -> None:
        registry = SchemaRegistry()
        # Register both modules so cross-module references resolve
        registry.get_node(DepModule.DepStruct)
        struct_defn = registry.get_node(TestModule.MyStruct)
        assert isinstance(struct_defn, StructNode)
        # Field 'dep' should reference DepStruct
        dep_field = struct_defn.fields[2]
        self.assertEqual(dep_field.name, "dep")
        assert isinstance(dep_field.type, StructTypeRef)
        resolved = dep_field.type.node
        self.assertIsInstance(resolved, StructNode)
        self.assertEqual(resolved.name, "DepStruct")

    # -- Singleton ----------------------------------------------

    def test_singleton(self) -> None:
        r1 = SchemaRegistry.get()
        r2 = SchemaRegistry.get()
        self.assertIs(r1, r2)

    def test_reset_clears_singleton(self) -> None:
        r1 = SchemaRegistry.get()
        SchemaRegistry._reset()
        r2 = SchemaRegistry.get()
        self.assertIsNot(r1, r2)

    # -- Error handling -----------------------------------------

    def test_type_without_uri(self) -> None:
        registry = SchemaRegistry()
        # A plain Python class has no __get_thrift_uri__
        with self.assertRaises(KeyError):
            registry.get_node(int)

    def test_syntax_graph_empty_initially(self) -> None:
        registry = SchemaRegistry()
        self.assertEqual(len(registry.syntax_graph.programs), 0)

    def test_syntax_graph_available_after_registration(self) -> None:
        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)
        self.assertGreater(len(registry.syntax_graph.programs), 0)

    # -- Node stability across registrations -----------------------

    def test_node_identity_survives_new_registration(self) -> None:
        registry = SchemaRegistry()
        node_before = registry.get_node(TestModule.MyStruct)
        registry.get_node(DepModule.DepStruct)
        node_after = registry.get_node(TestModule.MyStruct)
        self.assertIs(node_before, node_after)

    def test_node_belongs_to_current_graph(self) -> None:
        registry = SchemaRegistry()
        node_before = registry.get_node(TestModule.MyStruct)
        registry.get_node(DepModule.DepStruct)
        program = registry.syntax_graph.get_program_by_name("schema_registry_test")
        self.assertIs(node_before, program["MyStruct"])

    def test_cross_module_navigation_after_dependency_registered(self) -> None:
        registry = SchemaRegistry()
        struct_defn = registry.get_node(TestModule.MyStruct)
        assert isinstance(struct_defn, StructNode)
        registry.get_node(DepModule.DepStruct)
        dep_field = struct_defn.fields[2]
        self.assertEqual(dep_field.name, "dep")
        assert isinstance(dep_field.type, StructTypeRef)
        resolved = dep_field.type.node
        self.assertIsInstance(resolved, StructNode)
        self.assertEqual(resolved.name, "DepStruct")
        self.assertIs(resolved, registry.get_node(DepModule.DepStruct))

    def test_program_include_across_registrations(self) -> None:
        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)
        registry.get_node(DepModule.DepStruct)
        test_program = registry.syntax_graph.get_program_by_name("schema_registry_test")
        dep_program = registry.syntax_graph.get_program_by_name("schema_registry_dep")
        self.assertIn(dep_program, test_program.includes)

    def test_syntax_graph_identity_stable(self) -> None:
        registry = SchemaRegistry()
        graph1 = registry.syntax_graph
        registry.get_node(TestModule.MyStruct)
        graph2 = registry.syntax_graph
        registry.get_node(DepModule.DepStruct)
        graph3 = registry.syntax_graph
        self.assertIs(graph1, graph2)
        self.assertIs(graph2, graph3)

    # -- Transitive dependency registration -------------------------

    def test_transitive_dep_registered_automatically(self) -> None:
        """Registering TestModule (which includes dep) should transitively
        register DepModule, making DepStruct available without explicit
        registration."""
        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)
        dep_defn = registry.get_definition_by_uri(
            "thrift.com/python/schema/registry_dep/DepStruct"
        )
        self.assertIsInstance(dep_defn, StructNode)
        self.assertEqual(dep_defn.name, "DepStruct")

    def test_transitive_dep_program_has_definitions(self) -> None:
        """After registering only TestModule, the dep program should have
        its definitions populated (not empty)."""
        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)
        dep_program = registry.syntax_graph.get_program_by_name("schema_registry_dep")
        dep_names = [d.name for d in dep_program.definitions]
        self.assertIn("DepStruct", dep_names)

    def test_cross_module_navigation_without_explicit_dep_registration(self) -> None:
        """Navigating from MyStruct to its DepStruct field should work
        after only registering TestModule (dep is transitively registered)."""
        registry = SchemaRegistry()
        struct_defn = registry.get_node(TestModule.MyStruct)
        assert isinstance(struct_defn, StructNode)
        dep_field = struct_defn.fields[2]
        self.assertEqual(dep_field.name, "dep")
        assert isinstance(dep_field.type, StructTypeRef)
        resolved = dep_field.type.node
        self.assertIsInstance(resolved, StructNode)
        self.assertEqual(resolved.name, "DepStruct")

    # -- uri_to_module_map for not-yet-imported modules -------------

    def test_get_definition_by_uri_via_uri_to_module_map(self) -> None:
        lazy_mod_name = (
            "thrift.lib.python.schema.tests.schema_registry_lazy.thrift_types"
        )
        # Ensure the module is not already in sys.modules so the
        # uri_to_module_map resolution path is exercised.
        sys.modules.pop(lazy_mod_name, None)

        registry = SchemaRegistry()
        defn = registry.get_definition_by_uri(
            "thrift.com/python/schema/registry_lazy/LazyStruct"
        )
        self.assertIsInstance(defn, StructNode)
        self.assertEqual(defn.name, "LazyStruct")

    # -- URI-less types ---------------------------------------------

    def test_get_node_struct_without_uri(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(LegacyModule.LegacyStruct)
        assert isinstance(defn, StructNode)
        self.assertEqual(defn.name, "LegacyStruct")
        self.assertEqual(defn.uri, "")

    def test_get_node_union_without_uri(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(LegacyModule.LegacyUnion)
        assert isinstance(defn, UnionNode)
        self.assertEqual(defn.name, "LegacyUnion")
        self.assertEqual(defn.uri, "")

    def test_get_node_exception_without_uri(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(LegacyModule.LegacyException)
        assert isinstance(defn, ExceptionNode)
        self.assertEqual(defn.name, "LegacyException")
        self.assertEqual(defn.uri, "")

    def test_get_node_enum_without_uri(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(LegacyModule.LegacyEnum)
        assert isinstance(defn, EnumNode)
        self.assertEqual(defn.name, "LegacyEnum")
        self.assertEqual(defn.uri, "")

    def test_get_node_service_handler_without_uri(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(LegacyServices.LegacyServiceInterface)
        assert isinstance(defn, ServiceNode)
        self.assertEqual(defn.name, "LegacyService")
        self.assertEqual(defn.uri, "")

    def test_get_node_service_client_without_uri(self) -> None:
        registry = SchemaRegistry()
        handler_defn = registry.get_node(LegacyServices.LegacyServiceInterface)
        client_defn = registry.get_node(LegacyClients.LegacyService)
        # Handler and client should resolve to the same ServiceNode.
        self.assertIs(handler_defn, client_defn)

    def test_get_node_interaction_without_uri(self) -> None:
        registry = SchemaRegistry()
        defn = registry.get_node(LegacyClients.LegacyService_LegacyInteraction)
        assert isinstance(defn, InteractionNode)
        self.assertEqual(defn.name, "LegacyInteraction")
        self.assertEqual(defn.uri, "")

    def test_get_node_caches_uri_less_type(self) -> None:
        registry = SchemaRegistry()
        defn1 = registry.get_node(LegacyModule.LegacyStruct)
        defn2 = registry.get_node(LegacyModule.LegacyStruct)
        self.assertIs(defn1, defn2)

    def test_uri_less_field_resolution(self) -> None:
        registry = SchemaRegistry()
        struct_defn = registry.get_node(LegacyModule.LegacyStruct)
        assert isinstance(struct_defn, StructNode)
        # Field 'kind' references LegacyEnum, also URI-less. Resolution should
        # not depend on URI at any point.
        kind_field = struct_defn.fields[2]
        self.assertEqual(kind_field.name, "kind")
        resolved = kind_field.type.true_type
        self.assertIsInstance(resolved.node, EnumNode)  # type: ignore[attr-defined]
        self.assertEqual(resolved.node.name, "LegacyEnum")  # type: ignore[attr-defined]

    def test_get_definition_by_uri_empty_string_fails(self) -> None:
        registry = SchemaRegistry()
        # Even after registering URI-less definitions, the empty-string URI
        # must not auto-discover them.
        registry.get_node(LegacyModule.LegacyStruct)
        with self.assertRaises(KeyError):
            registry.get_definition_by_uri("")

    # -- definition_key lookup --------------------------------------

    def test_get_definition_by_definition_key(self) -> None:
        registry = SchemaRegistry()
        struct_defn = registry.get_node(TestModule.MyStruct)
        key = struct_defn.definition_key
        self.assertIsInstance(key, bytes)
        self.assertNotEqual(key, b"")
        looked_up = registry._builder.get_definition_by_definition_key(key)
        self.assertIs(looked_up, struct_defn)

    def test_get_node_falls_back_to_uri_when_definition_key_method_absent(
        self,
    ) -> None:
        registry = SchemaRegistry()
        cls = TestModule.MyStruct
        original = getattr(cls, "__get_thrift_definition_key__", None)
        self.assertIsNotNone(original)
        try:
            delattr(cls, "__get_thrift_definition_key__")
            defn = registry.get_node(cls)
            self.assertIsInstance(defn, StructNode)
            self.assertEqual(defn.name, "MyStruct")
        finally:
            setattr(cls, "__get_thrift_definition_key__", original)

    def test_get_node_invalid_definition_key_raises(self) -> None:
        registry = SchemaRegistry()
        cls = TestModule.MyStruct
        original = getattr(cls, "__get_thrift_definition_key__", None)
        self.assertIsNotNone(original)
        try:
            setattr(cls, "__get_thrift_definition_key__", staticmethod(lambda: b""))
            with self.assertRaises(RuntimeError):
                registry.get_node(cls)

            setattr(
                cls,
                "__get_thrift_definition_key__",
                staticmethod(lambda: "not-bytes"),
            )
            with self.assertRaises(RuntimeError):
                registry.get_node(cls)

            # Present but unknown bytes — should raise KeyError, not silently
            # fall back to URI.
            SchemaRegistry._reset()
            registry2 = SchemaRegistry()
            setattr(
                cls,
                "__get_thrift_definition_key__",
                staticmethod(lambda: b"\xde\xad\xbe\xef"),
            )
            with self.assertRaises(KeyError):
                registry2.get_node(cls)
        finally:
            setattr(cls, "__get_thrift_definition_key__", original)

    def test_merge_schema_idempotent(self) -> None:
        """Merging the same schema bytes twice is a no-op, not a conflict."""
        from thrift.lib.python.schema.schema_registry import _extract_schema_bytes

        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)
        # Re-merge the same schema bytes — must not raise.
        data = _extract_schema_bytes(TestModule)
        schema = registry._deserialize_schema(data)
        registry._builder.merge_schema(schema)

    def test_merge_schema_duplicate_key_with_different_payload_raises(self) -> None:
        from thrift.lib.python.schema.schema_registry import _extract_schema_bytes

        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)
        schema = registry._deserialize_schema(_extract_schema_bytes(TestModule))
        defns_map = dict(schema.definitionsMap or {})
        # Pick two distinct keys whose definitions differ, then point key_a at
        # defn_b. Re-merging this tampered schema must trip the conflict guard.
        keys = list(defns_map.keys())
        key_a, key_b = next(
            (a, b)
            for a in keys
            for b in keys
            if a != b and defns_map[a] != defns_map[b]
        )
        defns_map[key_a] = defns_map[key_b]
        tampered_schema = schema(definitionsMap=defns_map)
        with self.assertRaises(RuntimeError):
            registry._builder.merge_schema(tampered_schema)

    def test_get_definition_by_definition_key_unknown_returns_none(self) -> None:
        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)
        self.assertIsNone(
            registry._builder.get_definition_by_definition_key(b"\xde\xad\xbe\xef")
        )

    def test_dynamic_import_transitively_registers_deps(self) -> None:
        """When a module is dynamically imported via uri_to_module_map,
        its transitive dependencies should also be registered."""
        lazy_mod_name = (
            "thrift.lib.python.schema.tests.schema_registry_lazy_with_dep.thrift_types"
        )
        dep_mod_name = "thrift.lib.python.schema.tests.schema_registry_dep.thrift_types"
        sys.modules.pop(dep_mod_name, None)
        sys.modules.pop(lazy_mod_name, None)

        registry = SchemaRegistry()
        defn = registry.get_definition_by_uri(
            "thrift.com/python/schema/registry_lazy_with_dep/LazyWithDepStruct"
        )
        self.assertIsInstance(defn, StructNode)
        self.assertEqual(defn.name, "LazyWithDepStruct")

        # Navigate to the dependency type through the field — this proves
        # transitive registration happened during the single
        # get_definition_by_uri call above, without a separate lookup.
        assert isinstance(defn, StructNode)
        dep_field = defn.fields[1]
        self.assertEqual(dep_field.name, "dep")
        assert isinstance(dep_field.type, StructTypeRef)
        resolved = dep_field.type.node
        self.assertIsInstance(resolved, StructNode)
        self.assertEqual(resolved.name, "DepStruct")

    # -- Omnibus schema seeding for core/well-known types -----------

    _ANY_URI = "facebook.com/thrift/type/Any"

    def test_core_type_resolvable_after_seeding(self) -> None:
        # Registering any user module seeds the omnibus, so a core type like
        # `Any` (whose `any` module ships no bundled schema) becomes resolvable
        # by URI even though TestModule never references it.
        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)
        defn = registry.get_definition_by_uri(self._ANY_URI)
        self.assertIsInstance(defn, StructNode)
        self.assertEqual(defn.name, "AnyStruct")

    def test_any_field_resolves_to_omnibus_struct(self) -> None:
        # A field typed with `any.Any` resolves through the typedef
        # to AnyStruct, whose definition comes from the seeded omnibus.
        registry = SchemaRegistry()
        struct_defn = registry.get_node(AnyModule.StructWithAny)
        assert isinstance(struct_defn, StructNode)
        payload_field = struct_defn.fields[1]
        self.assertEqual(payload_field.name, "payload")
        resolved = payload_field.type.true_type
        assert isinstance(resolved, StructTypeRef)
        self.assertEqual(resolved.node.name, "AnyStruct")
        self.assertEqual(resolved.node.uri, self._ANY_URI)

    def test_source_identifier_lookups_unsupported(self) -> None:
        # The lazy registry cannot enumerate to build a source index, so (like
        # ``get_known_uris() -> None``) the source-identifier lookups report
        # "nothing known": ``None`` and an empty mapping.
        registry = SchemaRegistry()
        registry.get_node(TestModule.MyStruct)  # populate some definitions
        self.assertIsNone(
            registry.get_user_defined_type_by_source_identifier(
                "file://anything.thrift", "MyStruct"
            )
        )
        self.assertEqual(
            dict(registry.get_user_defined_types_at_location("file://anything.thrift")),
            {},
        )
