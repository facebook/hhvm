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

import thrift.lib.python.schema.tests.schema_registry_dep.thrift_types as DepModule
import thrift.lib.python.schema.tests.schema_registry_test.thrift_types as TestModule
from thrift.lib.python.schema.schema_registry import SchemaRegistry
from thrift.lib.python.schema.syntax_graph import (
    EnumNode,
    ExceptionNode,
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
