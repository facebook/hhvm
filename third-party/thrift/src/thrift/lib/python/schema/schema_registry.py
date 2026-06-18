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

"""
Pure Python SchemaRegistry -- lazy runtime schema lookup.

Sits on top of SyntaxGraph and provides Definition lookup by Python thrift
type or by Thrift URI.
"""

from __future__ import annotations

import functools
import importlib
import sys
import types
from collections.abc import Callable, Mapping
from importlib import resources
from typing import Any

import zstandard  # @manual=fbsource//third-party/pypi/zstandard:zstandard
from apache.thrift.type.schema import thrift_types as _schema_types
from thrift.lib.python.schema.syntax_graph import Definition, SyntaxGraph
from thrift.lib.python.schema.type_system import DefinitionNode, TypeSystem
from thrift.lib.python.schema.type_system_bridge import SyntaxGraphBridge
from thrift.python.serializer import deserialize, Protocol


def _has_bundled_schema(module: types.ModuleType) -> bool:
    """Whether a module carries a bundled ``_fbthrift_schema_*`` blob."""
    return any(k.startswith("_fbthrift_schema_") for k in vars(module))


def _extract_schema_bytes(module: types.ModuleType) -> bytes:
    """Find the _fbthrift_schema_* attribute on a generated thrift module."""
    schema_attr = None
    for attr in vars(module):
        if attr.startswith("_fbthrift_schema_"):
            if schema_attr is not None:
                raise RuntimeError(
                    f"Multiple bundled schemas found in module {module.__name__}."
                )
            schema_attr = attr

    if schema_attr is None:
        raise RuntimeError(f"Bundled schema not found in module {module.__name__}.")

    data = getattr(module, schema_attr)
    if not isinstance(data, bytes) or len(data) == 0:
        raise RuntimeError(
            f"Invalid or empty bundled schema in module {module.__name__}."
        )
    return data


_OMNIBUS_SCHEMA_RESOURCE = "omnibus_schema.zst"


@functools.cache
def _load_omnibus_schema() -> _schema_types.Schema | None:
    """Load the bundled omnibus schema of the well-known/core thrift types.

    Core types (``any``, ``patch``, ``type_system``, ...) live in modules that
    are intentionally built without ``with_schema`` (a thrift2ast bootstrap
    constraint), so their generated modules carry no ``_fbthrift_schema_`` blob.
    Seeding this omnibus into the registry lets fields typed with them resolve.

    Returns ``None`` when the resource isn't bundled; the registry then degrades
    to its prior behavior (such types remain unresolvable)."""
    try:
        data = (resources.files(__package__) / _OMNIBUS_SCHEMA_RESOURCE).read_bytes()
    except (OSError, ModuleNotFoundError):
        return None
    schema = SchemaRegistry._deserialize_schema(data)
    # Drop the program list: the omnibus combines many root files and can carry
    # duplicate program names, which the SyntaxGraph's name-unique program
    # index rejects. Only the definitions are needed here to resolve fields that
    # reference these core types.
    return schema(programs=[])


def _get_uri(thrift_type: type[Any]) -> str:
    """Extract URI from a Python thrift type, raising KeyError if unavailable."""
    get_uri = getattr(thrift_type, "__get_thrift_uri__", None)
    if get_uri is None:
        raise KeyError(f"Type {thrift_type.__name__} does not have __get_thrift_uri__")
    uri = get_uri()
    # Base classes return NotImplementedError() as sentinel
    if uri is None or isinstance(uri, NotImplementedError):
        raise KeyError(f"Type {thrift_type.__name__} does not have a Thrift URI")
    return uri


def _get_definition_key(thrift_type: type[Any]) -> bytes | None:
    """Extract the DefinitionKey bytes from a Python thrift type, if available.

    Returns None when the codegen predates `__get_thrift_definition_key__`. Raises
    `RuntimeError` when the method is present but returns invalid data — that signals
    a codegen bug and must not silently fall back to URI lookup.
    """
    get_key = getattr(thrift_type, "__get_thrift_definition_key__", None)
    if get_key is None:
        return None
    key = get_key()
    if not isinstance(key, bytes):
        raise RuntimeError(
            f"Type {thrift_type.__name__}.__get_thrift_definition_key__ "
            f"returned non-bytes: {type(key).__name__}"
        )
    if not key:
        raise RuntimeError(
            f"Type {thrift_type.__name__}.__get_thrift_definition_key__ "
            "returned empty bytes"
        )
    return key


class SchemaRegistry(TypeSystem):
    """Pure Python registry for looking up SyntaxGraph Definitions by type or URI.

    The registry is also the unbounded ``TypeSystem`` view: it bridges SyntaxGraph
    definitions into runtime ``DefinitionNode``s on demand, memoized.
    ``get_known_uris()`` returns ``None`` because the registry is
    lazy/module-discovery based and cannot enumerate all URIs up front."""

    _instance: SchemaRegistry | None = None

    @classmethod
    def get(cls) -> SchemaRegistry:
        """Global singleton access."""
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    @classmethod
    def _reset(cls) -> None:
        """Reset the singleton (for testing only)."""
        cls._instance = None

    def __init__(self) -> None:
        from thrift.lib.python.schema.syntax_graph_builder import (
            IncrementalGraphBuilder,
        )

        self._registered_modules: set[str] = set()
        self._type_to_definition: dict[type[Any], Definition] = {}
        self._uri_to_module: dict[str, types.ModuleType] = {}
        self._builder: IncrementalGraphBuilder = IncrementalGraphBuilder()
        self._module_resolver: Callable[[str], types.ModuleType | None] | None = None
        self._uri_module_map: dict[str, str] | None = None
        self._ts_bridge: SyntaxGraphBridge = SyntaxGraphBridge(self)
        self._omnibus_seeded: bool = False

    @property
    def syntax_graph(self) -> SyntaxGraph:
        """The underlying SyntaxGraph (empty until modules are registered)."""
        return self._builder.syntax_graph

    def set_module_resolver(
        self,
        resolver: Callable[[str], types.ModuleType | None],
    ) -> None:
        """Register a callback that resolves a Thrift URI to its Python module.

        Called as a last resort by get_definition_by_uri when the URI is not
        found in already-imported modules. The resolver receives a URI string
        and should return the thrift_types module that defines it, or None.
        """
        self._module_resolver = resolver

    def get_node(self, thrift_type: type[Any]) -> Definition:
        """Look up a SyntaxGraph Definition by Python thrift type.

        Auto-registers the type's module if not already registered. Prefers the
        type's `__get_thrift_definition_key__` bytes; falls back to URI lookup
        only when the codegen predates that static method.

        Raises:
            KeyError: if neither key is available, or no definition is found.
            RuntimeError: if `__get_thrift_definition_key__` returns invalid data.
        """
        cached = self._type_to_definition.get(thrift_type)
        if cached is not None:
            return cached

        defn_key = _get_definition_key(thrift_type)
        if defn_key is None:
            # Old codegen path: validate URI before module registration so we
            # surface a clear error for plain (non-Thrift) Python types.
            uri = _get_uri(thrift_type)
        else:
            uri = None

        # Auto-register the thrift_types module (schema bytes live there).
        # Types like enums/exceptions may live in sibling modules
        # (e.g. thrift_enums), so derive the thrift_types module name.
        types_module = self._find_types_module(thrift_type.__module__)
        if (
            types_module is not None
            and types_module.__name__ not in self._registered_modules
        ):
            self._register_module(types_module)

        if defn_key is not None:
            defn = self._builder.get_definition_by_definition_key(defn_key)
            if defn is None:
                raise KeyError(
                    f"Definition not found for {thrift_type.__name__} "
                    f"(definition_key={defn_key!r})"
                )
        else:
            assert uri is not None
            defn = self._builder.get_definition_by_uri(uri)
            if defn is None:
                raise KeyError(f"Definition not found for URI {uri!r}")

        self._type_to_definition[thrift_type] = defn
        return defn

    def get_definition_by_uri(self, uri: str) -> Definition:
        """Look up a Definition by Thrift URI.

        Auto-discovers imported thrift modules if the URI is not already
        registered. Raises KeyError if not found.
        """
        defn = self._builder.get_definition_by_uri(uri)
        if defn is not None:
            return defn

        module = self._resolve_module_for_uri(uri)
        if module is not None:
            self._register_module(module)
            defn = self._builder.get_definition_by_uri(uri)
            if defn is not None:
                return defn

        raise KeyError(f"Definition not found for URI {uri!r}")

    # -- TypeSystem interface (bridged from SyntaxGraph) --------------------

    def definition_by_uri(self, uri: str) -> Definition | None:
        """SyntaxResolver hook: AST Definition for ``uri`` (``None`` if unknown).

        Wraps ``get_definition_by_uri`` -- triggering the same lazy module
        discovery -- but returns ``None`` instead of raising on a miss."""
        try:
            return self.get_definition_by_uri(uri)
        except KeyError:
            return None

    def get_user_defined_type(self, uri: str) -> DefinitionNode | None:
        """Resolve a URI to a TypeSystem ``DefinitionNode`` (bridged from the
        SyntaxGraph, memoized). ``None`` if unknown or not a user-defined type
        (typedefs/constants/services/interactions are excluded)."""
        return self._ts_bridge.get_user_defined_type(uri)

    def get_known_uris(self) -> None:
        """``None`` -- the registry is lazy/module-discovery based and cannot
        enumerate all URIs up front."""
        return None

    def get_user_defined_type_by_source_identifier(
        self, locator: str, name: str
    ) -> DefinitionNode | None:
        """``None`` -- the registry is lazy/module-discovery based and cannot
        build a source-identifier index up front."""
        return None

    def get_user_defined_types_at_location(
        self, locator: str
    ) -> Mapping[str, DefinitionNode]:
        """``{}`` -- the registry is lazy/module-discovery based and cannot
        enumerate the types at a source location up front."""
        return {}

    def _resolve_module_for_uri(self, uri: str) -> types.ModuleType | None:
        """Find the module that defines the given URI."""
        self._build_uri_to_module_index()
        module = self._uri_to_module.get(uri)
        if module is not None:
            return module

        module = self._resolve_from_uri_map(uri)
        if module is not None:
            return module

        if self._module_resolver is not None:
            return self._module_resolver(uri)

        return None

    def _resolve_from_uri_map(self, uri: str) -> types.ModuleType | None:
        """Try to resolve a URI using the build-time uri_to_module_map."""
        if self._uri_module_map is None:
            try:
                from thrift.facebook.python.utils import uri_to_module_map  # @manual

                self._uri_module_map = uri_to_module_map()
            except ImportError:
                self._uri_module_map = {}

        module_name = self._uri_module_map.get(uri)
        if module_name is None:
            return None

        try:
            return importlib.import_module(f"{module_name}.thrift_types")
        except ImportError:
            return None

    def _build_uri_to_module_index(self) -> None:
        """Scan sys.modules for thrift types, build URI -> module map.

        Only scans modules that have bundled schema data and haven't been
        registered yet. Uses __get_thrift_uri__ on types to extract URIs
        without deserializing the schema.
        """
        self._uri_to_module.clear()
        for module in sys.modules.values():
            if module is None:
                continue
            if not _has_bundled_schema(module):
                continue
            if module.__name__ in self._registered_modules:
                continue
            for attr_val in vars(module).values():
                get_uri_fn = getattr(attr_val, "__get_thrift_uri__", None)
                if get_uri_fn is None:
                    continue
                uri = get_uri_fn()
                if uri and not isinstance(uri, NotImplementedError):
                    self._uri_to_module[uri] = module

    @staticmethod
    def _find_types_module(module_name: str) -> types.ModuleType | None:
        """Find the thrift_types module for a given thrift module name.

        Schema bytes are always on the thrift_types module. Enums live in
        thrift_enums, exceptions in thrift_exceptions, etc. This derives
        the thrift_types module from any sibling module name.
        """
        # If already a thrift_types module, use it directly
        module = sys.modules.get(module_name)
        if module is not None:
            for attr in vars(module):
                if attr.startswith("_fbthrift_schema_"):
                    return module

        # Try the sibling thrift_types module
        parts = module_name.rsplit(".", 1)
        if len(parts) == 2:
            types_name = f"{parts[0]}.thrift_types"
            types_module = sys.modules.get(types_name)
            if types_module is not None:
                return types_module

        return None

    def _seed_omnibus_if_needed(self) -> None:
        """Merge the bundled omnibus schema into the builder once, so user
        modules that reference core types (``any``, etc.) resolve. The build
        itself happens in the caller's ``build_pending``."""
        if self._omnibus_seeded:
            return
        self._omnibus_seeded = True
        schema = _load_omnibus_schema()
        if schema is not None:
            self._builder.merge_schema(schema)

    def _register_module(self, module: types.ModuleType) -> None:
        """Register a thrift module and its transitive dependencies."""
        if module.__name__ in self._registered_modules:
            return

        self._seed_omnibus_if_needed()
        self._collect_transitive_schemas(module)
        self._builder.build_pending()

    def _collect_transitive_schemas(self, module: types.ModuleType) -> None:
        """Recursively merge schema data from a module and its dependencies."""
        if module.__name__ in self._registered_modules:
            return

        data = _extract_schema_bytes(module)
        self._registered_modules.add(module.__name__)
        schema = self._deserialize_schema(data)
        self._builder.merge_schema(schema)

        # Transitively register dependency modules whose thrift_types are
        # already imported (generated thrift_types.py files import their
        # dependencies at the top level, so they're in sys.modules).
        for prog in schema.programs or []:
            dep_module = self._find_dep_types_module(prog)
            if dep_module is not None:
                self._collect_transitive_schemas(dep_module)

    def _find_dep_types_module(
        self, prog: _schema_types.Program
    ) -> types.ModuleType | None:
        """Derive the Python thrift_types module for a schema program entry."""
        ns = prog.namespaces.get("py3") if prog.namespaces else None
        name = prog.attrs.name if prog.attrs else None
        if not ns or not name:
            return None
        module_name = f"{ns}.{name}.thrift_types"
        if module_name in self._registered_modules:
            return None
        module = sys.modules.get(module_name)
        # Skip dependency modules without a bundled schema (e.g. core thrift
        # libraries built without `with_schema`, such as `any`/`any_rep`). Their
        # definitions arrive via the seeded omnibus schema, not by recursing into
        # the module.
        if module is None or not _has_bundled_schema(module):
            return None
        return module

    @staticmethod
    def _deserialize_schema(data: bytes) -> _schema_types.Schema:
        """Decompress and deserialize bundled schema bytes."""
        decompressor = zstandard.ZstdDecompressor()
        decompressed = decompressor.decompress(data)
        return deserialize(
            _schema_types.Schema, decompressed, protocol=Protocol.COMPACT
        )
