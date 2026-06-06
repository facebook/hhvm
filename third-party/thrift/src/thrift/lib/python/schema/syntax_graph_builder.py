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
Builder that constructs a SyntaxGraph from a deserialized type::Schema.
"""

from __future__ import annotations

import dataclasses
import functools
from typing import Any

from apache.thrift.syntax_graph.syntax_graph.thrift_types import Primitive
from apache.thrift.type.schema import thrift_types as _schema_types
from apache.thrift.type.standard import thrift_types as _standard_types
from apache.thrift.type.type_rep import thrift_types as _type_rep_types
from thrift.lib.python.schema.syntax_graph import (
    _Lazy,
    _Resolver,
    Annotation,
    ConstantNode,
    Definition,
    EnumNode,
    EnumTypeRef,
    EnumValue,
    ExceptionNode,
    ExceptionTypeRef,
    FieldNode,
    FunctionException,
    FunctionNode,
    FunctionParam,
    FunctionResponse,
    FunctionSink,
    FunctionStream,
    InteractionNode,
    ListTypeRef,
    MapTypeRef,
    PrimitiveTypeRef,
    ProgramNode,
    ServiceNode,
    SetTypeRef,
    StructNode,
    StructTypeRef,
    SyntaxGraph,
    TypedefNode,
    TypedefTypeRef,
    TypeRef,
    UnionNode,
    UnionTypeRef,
)


@dataclasses.dataclass
class IncrementalBuildResult:
    programs: dict[int, ProgramNode]


class IncrementalGraphBuilder:
    """Persistent builder for incremental schema merging and graph construction.

    Owns all accumulated schema state, the resolver, and the target
    SyntaxGraph. Call merge_schema() to add new schema data, then
    build_pending() to construct any new definitions and programs added
    since the last build. New programs are automatically added to the
    owned SyntaxGraph.
    """

    def __init__(self, syntax_graph: SyntaxGraph | None = None) -> None:
        self._syntax_graph: SyntaxGraph = (
            syntax_graph if syntax_graph is not None else SyntaxGraph._create_empty()
        )
        self._resolver: _Resolver = _Resolver()
        self._programs_by_id: dict[int, ProgramNode] = {}
        self._merged_definitions: dict[bytes, _schema_types.Definition] = {}
        self._merged_programs: list[_schema_types.Program] = []
        self._merged_program_ids: set[int] = set()
        self._merged_values: dict[int, Any] = {}
        self._new_definition_keys: set[bytes] = set()
        self._new_program_ids: set[int] = set()

    @property
    def syntax_graph(self) -> SyntaxGraph:
        return self._syntax_graph

    def merge_schema(self, schema: _schema_types.Schema) -> None:
        """Merge a deserialized Schema, tracking what's new since last build.

        Raises:
            RuntimeError: if a previously-merged ``definition_key`` arrives with
            a different serialized ``Definition``. This catches inconsistent
            partial schema syncs that would otherwise be silently masked by
            keeping whichever copy arrived first.
        """
        if schema.definitionsMap:
            for key, defn in schema.definitionsMap.items():
                existing = self._merged_definitions.get(key)
                if existing is None:
                    self._merged_definitions[key] = defn
                    self._new_definition_keys.add(key)
                elif existing != defn:
                    raise RuntimeError(
                        f"Conflicting definitions merged for definition_key {key!r}"
                    )

        if schema.programs:
            for prog in schema.programs:
                if prog.id not in self._merged_program_ids:
                    self._merged_programs.append(prog)
                    self._merged_program_ids.add(prog.id)
                    self._new_program_ids.add(prog.id)

        if schema.valuesMap:
            for vid, val in schema.valuesMap.items():
                if vid not in self._merged_values:
                    self._merged_values[vid] = val

    def get_definition_by_uri(self, uri: str) -> Definition | None:
        return self._resolver.get_by_uri(uri)

    def get_definition_by_definition_key(self, key: bytes) -> Definition | None:
        return self._resolver.get_by_key(key)

    def build_pending(self) -> IncrementalBuildResult:
        """Build definitions and programs added since the last build."""
        new_definition_keys = self._new_definition_keys
        self._new_definition_keys = set()
        new_program_ids = self._new_program_ids
        self._new_program_ids = set()

        new_programs_to_build = [
            p for p in self._merged_programs if p.id in new_program_ids
        ]
        builder = _GraphBuilder(
            self._merged_definitions,
            [],
            self._merged_values,
            resolver=self._resolver,
            existing_programs_by_id=self._programs_by_id,
            new_definition_keys=new_definition_keys,
            new_programs=new_programs_to_build,
        )
        result = builder.build_new_programs()

        for prog_id, program in result.programs.items():
            self._programs_by_id[prog_id] = program
            self._syntax_graph._add_program(program)

        return result


@functools.cache
def _typename_to_primitive_map() -> dict[int, Primitive]:
    _TN = _standard_types.TypeName.Type
    return {
        _TN.boolType.value: Primitive.BOOL,
        _TN.byteType.value: Primitive.BYTE,
        _TN.i16Type.value: Primitive.I16,
        _TN.i32Type.value: Primitive.I32,
        _TN.i64Type.value: Primitive.I64,
        _TN.floatType.value: Primitive.FLOAT,
        _TN.doubleType.value: Primitive.DOUBLE,
        _TN.stringType.value: Primitive.STRING,
        _TN.binaryType.value: Primitive.BINARY,
    }


def _extract_definition_key(type_uri: _standard_types.TypeUri) -> bytes:
    """Extract DefinitionKey from a TypeUri."""
    # With use_hash=true, definitionKey is the primary field
    if type_uri.type == _standard_types.TypeUri.Type.definitionKey:
        return type_uri.definitionKey
    raise RuntimeError(f"Cannot extract definition key from TypeUri: {type_uri}")


def _value_to_python(val: Any) -> Any:
    """Convert a protocol.Value to a JSON-like Python value."""
    if val is None or val.value is None:
        return None
    _VT = type(val).Type
    vtype = val.type

    if vtype == _VT.boolValue:
        return val.boolValue
    elif vtype == _VT.byteValue:
        return val.byteValue
    elif vtype == _VT.i16Value:
        return val.i16Value
    elif vtype == _VT.i32Value:
        return val.i32Value
    elif vtype == _VT.i64Value:
        return val.i64Value
    elif vtype == _VT.floatValue:
        return val.floatValue
    elif vtype == _VT.doubleValue:
        return val.doubleValue
    elif vtype == _VT.stringValue:
        raw = val.stringValue
        return raw.decode("utf-8") if isinstance(raw, bytes) else raw
    elif vtype == _VT.binaryValue:
        return val.binaryValue
    elif vtype == _VT.listValue:
        return [_value_to_python(v) for v in val.listValue]
    elif vtype == _VT.setValue:
        return [_value_to_python(v) for v in val.setValue]
    elif vtype == _VT.mapValue:
        return {
            _value_to_python(k): _value_to_python(v) for k, v in val.mapValue.items()
        }
    elif vtype == _VT.objectValue:
        obj = val.objectValue
        return {k: _value_to_python(v) for k, v in obj.members.items()}
    return None


class _GraphBuilder:
    """Builds a SyntaxGraph from a definitions map and programs list."""

    def __init__(
        self,
        definitions_map: dict[bytes, _schema_types.Definition],
        all_programs: list[_schema_types.Program],
        values_map: dict[int, Any] | None = None,
        *,
        resolver: _Resolver | None = None,
        existing_programs_by_id: dict[int, ProgramNode] | None = None,
        new_definition_keys: set[bytes] | None = None,
        new_programs: list[_schema_types.Program] | None = None,
    ) -> None:
        self._definitions_map = definitions_map
        self._all_programs = all_programs
        self._values_map: dict[int, Any] = values_map or {}
        self._resolver: _Resolver = resolver if resolver is not None else _Resolver()
        self._existing_programs_by_id: dict[int, ProgramNode] = (
            existing_programs_by_id if existing_programs_by_id is not None else {}
        )
        self._new_definition_keys: set[bytes] | None = new_definition_keys
        self._new_programs: list[_schema_types.Program] | None = new_programs
        self._all_lazys: list[_Lazy[Any]] = []

    def build(self, validate: bool = True) -> SyntaxGraph:
        self._build_definitions()
        new_programs = self._build_programs()
        programs = list(new_programs.values())
        by_name = {p.name: p for p in programs}
        by_path = {p.path: p for p in programs if p.path}
        if validate:
            self._validate_lazys()
        return SyntaxGraph(programs, by_name, by_path)

    def build_new_programs(self, validate: bool = True) -> IncrementalBuildResult:
        """Build only new definitions and programs.

        Requires that all transitive dependency schemas have been merged
        before calling, so that programs are created with complete definitions.
        """
        self._build_definitions()
        new_programs = self._build_programs()
        if validate:
            self._validate_lazys()
        return IncrementalBuildResult(
            programs=new_programs,
        )

    # -- Type conversion ---------------------------------------------------

    def _type_of(self, type_struct: _type_rep_types.TypeStruct) -> TypeRef:
        """Convert a TypeStruct to a TypeRef."""
        type_name = type_struct.name
        params = type_struct.params or []
        tn_type = type_name.type
        _TN = _standard_types.TypeName.Type

        # Check primitives
        prim = _typename_to_primitive_map().get(tn_type.value)
        if prim is not None:
            return PrimitiveTypeRef(prim)

        # Named types
        if tn_type == _TN.structType:
            key = _extract_definition_key(type_name.structType)
            lazy = _Lazy(self._resolver, key)
            self._all_lazys.append(lazy)
            return StructTypeRef(lazy)

        if tn_type == _TN.unionType:
            key = _extract_definition_key(type_name.unionType)
            lazy = _Lazy(self._resolver, key)
            self._all_lazys.append(lazy)
            return UnionTypeRef(lazy)

        if tn_type == _TN.exceptionType:
            key = _extract_definition_key(type_name.exceptionType)
            lazy = _Lazy(self._resolver, key)
            self._all_lazys.append(lazy)
            return ExceptionTypeRef(lazy)

        if tn_type == _TN.enumType:
            key = _extract_definition_key(type_name.enumType)
            lazy = _Lazy(self._resolver, key)
            self._all_lazys.append(lazy)
            return EnumTypeRef(lazy)

        if tn_type == _TN.typedefType:
            key = _extract_definition_key(type_name.typedefType)
            lazy = _Lazy(self._resolver, key)
            self._all_lazys.append(lazy)
            return TypedefTypeRef(lazy)

        # Container types
        if tn_type == _TN.listType:
            return ListTypeRef(self._type_of(params[0]))

        if tn_type == _TN.setType:
            return SetTypeRef(self._type_of(params[0]))

        if tn_type == _TN.mapType:
            return MapTypeRef(self._type_of(params[0]), self._type_of(params[1]))

        raise RuntimeError(f"Unknown TypeName variant: {tn_type}")

    def _type_of_or_none(
        self, type_struct: _type_rep_types.TypeStruct | None
    ) -> TypeRef | None:
        """Convert TypeStruct to TypeRef, or None for void."""
        if type_struct is None:
            return None
        type_name = type_struct.name
        if type_name is None or type_name.value is None:
            return None
        return self._type_of(type_struct)

    # -- Field conversion --------------------------------------------------

    def _doc_block_of(self, attrs: _schema_types.DefinitionAttrs) -> str | None:
        if attrs.docs and attrs.docs.contents:
            return attrs.docs.contents
        return None

    def _resolve_value(self, value_id: int) -> Any | None:
        if value_id == 0:
            return None
        val = self._values_map.get(value_id)
        if val is None:
            return None
        return val

    def _create_field(self, field: _schema_types.Field) -> FieldNode:
        return FieldNode(
            id=field.id,
            name=field.attrs.name,
            type=self._type_of(field.type),
            qualifier=field.qualifier,
            doc_block=self._doc_block_of(field.attrs),
            annotations=self._create_annotations(field.attrs),
            default_value=self._resolve_value(field.customDefault),
        )

    def _create_fields(self, fields: _schema_types.Fields) -> list[FieldNode]:
        return [self._create_field(f) for f in (fields or [])]

    # -- Annotation conversion ---------------------------------------------

    def _get_annotation_type_ref(self, defn_key: bytes) -> TypeRef | None:
        """Build a TypeRef for an annotation type from the raw schema."""
        raw_defn = self._definitions_map.get(defn_key)
        if raw_defn is None:
            raise RuntimeError(
                f"Annotation references unknown definition key {defn_key!r}; "
                "the defining module is missing from the merged schema."
            )
        _DT = _schema_types.Definition.Type
        dtype = raw_defn.type
        if dtype == _DT.structDef:
            type_ref_cls = StructTypeRef
        elif dtype == _DT.typedefDef:
            type_ref_cls = TypedefTypeRef
        elif dtype == _DT.unionDef:
            type_ref_cls = UnionTypeRef
        elif dtype == _DT.exceptionDef:
            type_ref_cls = ExceptionTypeRef
        elif dtype == _DT.enumDef:
            type_ref_cls = EnumTypeRef
        else:
            return None
        lazy = _Lazy(self._resolver, defn_key)
        self._all_lazys.append(lazy)
        return type_ref_cls(lazy)

    def _create_annotations(
        self, attrs: _schema_types.DefinitionAttrs
    ) -> list[Annotation]:
        annotations = []
        if not attrs.annotationsByKey:
            return annotations
        for defn_key, annot in attrs.annotationsByKey.items():
            type_ref = self._get_annotation_type_ref(defn_key)
            if type_ref is None:
                continue
            value = {}
            if annot.fields:
                for fname, fval in annot.fields.items():
                    value[fname] = _value_to_python(fval)
            annotations.append(Annotation(type_ref, value))
        return annotations

    # -- Definition builders -----------------------------------------------

    def _common_kwargs(
        self, key: bytes, attrs: _schema_types.DefinitionAttrs
    ) -> dict[str, Any]:
        return {
            "name": attrs.name,
            "definition_key": key,
            "resolver": self._resolver,
            "doc_block": self._doc_block_of(attrs),
            "annotations": self._create_annotations(attrs),
        }

    def _create_struct(self, key: bytes, s: _schema_types.Struct) -> StructNode:
        return StructNode(
            uri=s.attrs.uri or "",
            fields=self._create_fields(s.fields),
            **self._common_kwargs(key, s.attrs),
        )

    def _create_union(self, key: bytes, u: _schema_types.Union) -> UnionNode:
        return UnionNode(
            uri=u.attrs.uri or "",
            fields=self._create_fields(u.fields),
            **self._common_kwargs(key, u.attrs),
        )

    def _create_exception(
        self, key: bytes, e: _schema_types.Exception
    ) -> ExceptionNode:
        return ExceptionNode(
            uri=e.attrs.uri or "",
            fields=self._create_fields(e.fields),
            **self._common_kwargs(key, e.attrs),
        )

    def _create_enum(self, key: bytes, e: _schema_types.Enum) -> EnumNode:
        values = []
        for ev in e.values or []:
            values.append(
                EnumValue(
                    name=ev.attrs.name,
                    value=ev.value,
                    annotations=self._create_annotations(ev.attrs),
                )
            )
        return EnumNode(
            uri=e.attrs.uri or "",
            values=values,
            **self._common_kwargs(key, e.attrs),
        )

    def _create_typedef(self, key: bytes, td: _schema_types.Typedef) -> TypedefNode:
        return TypedefNode(
            target_type=self._type_of(td.type),
            **self._common_kwargs(key, td.attrs),
        )

    def _create_constant(self, key: bytes, c: _schema_types.Const) -> ConstantNode:
        return ConstantNode(
            type=self._type_of(c.type),
            value=self._resolve_value(c.value),
            **self._common_kwargs(key, c.attrs),
        )

    def _create_function_exceptions(
        self, exceptions: _schema_types.Exceptions
    ) -> list[FunctionException]:
        result = []
        for exc in exceptions or []:
            result.append(
                FunctionException(
                    id=exc.id,
                    name=exc.attrs.name,
                    type=self._type_of(exc.type),
                )
            )
        return result

    def _create_function(self, func: _schema_types.Function) -> FunctionNode:
        # Return type
        response_type = self._type_of_or_none(func.returnType)

        # Stream or sink
        stream = None
        sink = None
        if func.streamOrSink and func.streamOrSink.value is not None:
            _RT = _schema_types.ReturnType.Type
            if func.streamOrSink.type == _RT.streamType:
                s = func.streamOrSink.streamType
                stream = FunctionStream(
                    payload_type=self._type_of(s.payload),
                    exceptions=self._create_function_exceptions(s.exceptions),
                )
            elif func.streamOrSink.type == _RT.sinkType:
                s = func.streamOrSink.sinkType
                sink = FunctionSink(
                    payload_type=self._type_of(s.payload),
                    final_response_type=self._type_of(s.finalResponse),
                    client_exceptions=self._create_function_exceptions(
                        s.clientExceptions
                    ),
                    server_exceptions=self._create_function_exceptions(
                        s.serverExceptions
                    ),
                )

        # Interaction
        interaction_lazy: _Lazy[InteractionNode] | None = None
        if (
            func.interactionType
            and func.interactionType.uri
            and func.interactionType.uri.value is not None
        ):
            ikey = _extract_definition_key(func.interactionType.uri)
            interaction_lazy = _Lazy(self._resolver, ikey)
            self._all_lazys.append(interaction_lazy)

        response = FunctionResponse(
            type=response_type,
            interaction_lazy=interaction_lazy,
            sink=sink,
            stream=stream,
        )

        params = []
        if func.paramlist and func.paramlist.fields:
            for p in func.paramlist.fields:
                params.append(
                    FunctionParam(
                        id=p.id,
                        name=p.attrs.name,
                        type=self._type_of(p.type),
                    )
                )

        return FunctionNode(
            name=func.attrs.name,
            doc_block=self._doc_block_of(func.attrs),
            response=response,
            params=params,
            exceptions=self._create_function_exceptions(func.exceptions),
        )

    def _create_service(self, key: bytes, svc: _schema_types.Service) -> ServiceNode:
        # Base service
        base_lazy: _Lazy[ServiceNode] | None = None
        if (
            svc.baseService
            and svc.baseService.uri
            and svc.baseService.uri.value is not None
        ):
            bkey = _extract_definition_key(svc.baseService.uri)
            base_lazy = _Lazy(self._resolver, bkey)
            self._all_lazys.append(base_lazy)

        functions = [self._create_function(f) for f in (svc.functions or [])]
        return ServiceNode(
            uri=svc.attrs.uri or "",
            base_service_lazy=base_lazy,
            functions=functions,
            **self._common_kwargs(key, svc.attrs),
        )

    def _create_interaction(
        self, key: bytes, ix: _schema_types.Interaction
    ) -> InteractionNode:
        functions = [self._create_function(f) for f in (ix.functions or [])]
        return InteractionNode(
            uri=ix.attrs.uri or "",
            functions=functions,
            **self._common_kwargs(key, ix.attrs),
        )

    # -- Top-level build steps ---------------------------------------------

    def _build_definitions(self) -> None:
        _DT = _schema_types.Definition.Type
        if self._new_definition_keys is not None:
            keys = self._new_definition_keys
        else:
            keys = self._definitions_map.keys()
        for key in keys:
            if key in self._resolver:
                continue

            defn = self._definitions_map[key]
            dtype = defn.type
            if dtype == _DT.structDef:
                node = self._create_struct(key, defn.structDef)
            elif dtype == _DT.unionDef:
                node = self._create_union(key, defn.unionDef)
            elif dtype == _DT.exceptionDef:
                node = self._create_exception(key, defn.exceptionDef)
            elif dtype == _DT.enumDef:
                node = self._create_enum(key, defn.enumDef)
            elif dtype == _DT.typedefDef:
                node = self._create_typedef(key, defn.typedefDef)
            elif dtype == _DT.constDef:
                node = self._create_constant(key, defn.constDef)
            elif dtype == _DT.serviceDef:
                node = self._create_service(key, defn.serviceDef)
            elif dtype == _DT.interactionDef:
                node = self._create_interaction(key, defn.interactionDef)
            else:
                continue

            self._resolver.register(key, node)

    def _build_programs(self) -> dict[int, ProgramNode]:
        """Build ProgramNodes from the programs list.

        When new_programs is provided, builds only those programs.
        Otherwise builds all programs from the schema.
        Existing programs are available for include resolution.
        Returns a dict mapping program ID to newly created ProgramNode.
        """
        programs_by_id: dict[int, ProgramNode] = dict(self._existing_programs_by_id)
        new_programs: dict[int, ProgramNode] = {}
        new_prog_data: list[tuple[_schema_types.Program, ProgramNode]] = []

        progs_to_build = (
            self._new_programs if self._new_programs is not None else self._all_programs
        )

        for prog in progs_to_build:
            name = prog.attrs.name
            path = prog.path or ""
            if not name and path:
                basename = path.rsplit("/", 1)[-1]
                if basename.endswith(".thrift"):
                    name = basename[: -len(".thrift")]
                else:
                    name = basename

            program = ProgramNode(
                name=name,
                package=prog.attrs.uri or "",
                path=path,
                namespaces=dict(prog.namespaces) if prog.namespaces else {},
            )

            for dk in prog.definitionKeys or []:
                if dk in self._resolver:
                    defn = self._resolver.resolve(dk)
                    program._definitions.append(defn)
                    program._definitions_by_name[defn.name] = defn
                    if defn._program is None:
                        defn._program = program

            programs_by_id[prog.id] = program
            new_programs[prog.id] = program
            new_prog_data.append((prog, program))

        for prog, program in new_prog_data:
            for inc_id in prog.includes or []:
                inc_program = programs_by_id.get(inc_id)
                if inc_program is not None:
                    program._includes.append(inc_program)

        return new_programs

    def _validate_lazys(self) -> None:
        for lazy in self._all_lazys:
            lazy.resolve()  # raises RuntimeError if unresolvable
