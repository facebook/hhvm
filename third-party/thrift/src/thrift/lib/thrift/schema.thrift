/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

include "thrift/annotation/hack.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/id.thrift"
include "thrift/lib/thrift/standard.thrift"
include "thrift/lib/thrift/protocol.thrift"
include "thrift/lib/thrift/type.thrift"
include "thrift/annotation/python.thrift"

/**
 * The canonical representations for a Thrift schema.
 *
 * The definitions in this file contain the absolute minimal amount of
 * information needed to work with a 'dynamic' thrift value (e.g., one where the
 * type is only known at runtime).
 *
 *
 * Note that, while an AST representation may reuse some of the definitions from
 * this file, it has significantly different design goals. For example, an ideal
 * AST representation is a ~pure representation of the source file, with as
 * little 'interpretation' as possible. While a 'schema' is the result of
 * performing semantic analysis on an AST, to distill the result of interpreting
 * the source file in to the representations found in this file.
 */
@thrift.Experimental
package "facebook.com/thrift/type"

namespace cpp2 apache.thrift.type
namespace py3 apache.thrift.type
namespace java.swift com.facebook.thrift.type
namespace go thrift.lib.thrift.schema

/**
 * An unordered set of value ids, that can contain *at most one* value of any type.
 * Values correspond to the struct StructuredAnnotation.
 */
@python.Py3Hidden
@thrift.Experimental // TODO(afuller): Adapt!
typedef set<id.ValueId> AnnotationIds

/**
 * An instance of an annotation, applied to some definition.
 */
@python.Py3Hidden
struct Annotation {
  1: map<string, protocol.Value> fields;
}

/**
 * An instance of an annotation, applied to some definition.
 */
@python.Py3Hidden
struct StructuredAnnotation {
  1: standard.TypeUri type;
  2: map<string, protocol.Value> fields;
}

/**
 * An list of definition ids, in the order they were declared in the IDL/AST.
 *
 * Changing the order of definitions is always backward compatible.
 */
// TODO(afuller): Add conformance tests to make sure this is true.
@python.Py3Hidden
@thrift.Experimental // TODO(afuller): Adapt!
typedef list<id.DefinitionId> DefinitionIds

/**
 * An list of programs ids, in the order they were included in the IDL/AST.
 *
 * Changing the order of include is always backwards compatible.
 */
// TODO(afuller): Add conformance tests to make sure this is true.
@python.Py3Hidden
@thrift.Experimental // TODO(afuller): Adapt!
typedef list<id.ProgramId> IncludeIds

/**
 * A decoded URI.
 *
 *   {scheme}://{domain}/{path}?{query}#{fragment}
 */
@python.Py3Hidden
@thrift.Experimental
struct DecodedUri {
  /** The scheme, if present. */
  1: string scheme;

  /** The domain, for example "meta.com" -> ["meta", "com"] */
  2: list<string> domain;

  /** The path, for example "path/to/file" -> ["path", "to", "file"] */
  4: list<string> path;

  /** The query args. */
  5: map<string, string> query;

  /** The fragment, if present. */
  6: string fragment;
}

/** The attributes that can be associated with any Thrift definition. */
@python.Py3Hidden
struct DefinitionAttrs {
  /**
   * The un-scoped 'name' for this definition.
   *
   * Changing this is backward compatible for all Thrift v1+ supported protocols
   * (e.g. Binary, Compact).
   *
   * Changing this is backward *incompatible* for any component that converts a
   * 'name' into another form of identity (e.g. uri, field id, enum value's
   * value). For example:
   *  - generated code,
   *  - IDL const and literal values,
   *  - YAML parsers.
   *  - Protocols deprecated in v1+, e.g. JSON and SimpleJson.
   */
  // TODO(afuller): Support aliases to help with renaming.
  1: string name;

  /**
   * The globally unique Thrift URI for this definition.
   *
   * Must match the pattern:
   *     {domainLabel}(.{domainLabel})+(/{pathSegment})+/{name}
   * Where:
   *  - domainLabel: [a-z0-9-]+
   *  - pathSegment: [a-z0-9_-]+
   *  - name:        [a-zA-Z0-9_-]+
   *
   * Changing this is backward *incompatible* for any component that
   * uses URI-based lookup. For example:
   *  - URI-base AST features.
   *  - serialized Any values,
   * This means that value previously serialized in an Any with the old
   * URI can no longer be deserialized.
   */
  // TODO(afuller): Support aliases to help with renaming.
  2: string uri;

  /**
   * DEPRECATED! Prefer `annotations`.
   * The structured annotations associated with this definition.
   */
  3: AnnotationIds structuredAnnotations;

  /**
   * The structured annotations associated with this definition indexed by
   * their universal names.
   */
  8: map<string, Annotation> annotations;

  /**
   * The unstructured annotations (deprecated) associated with this definition.
   */
  4: map<string, string> unstructuredAnnotations;

  /** Information about the documentation preceding the definition. */
  6: DocBlock docs;

  /** The source range containing the definition (excluding its docblock). */
  7: SourceRange sourceRange;
}

// A lightweight source range that can be resolved into the file name,
// line and column when the schema is produced from an IDL file.
@python.Py3Hidden
struct SourceRange {
  1: id.ProgramId programId;
  2: i32 beginLine;
  3: i32 beginColumn;
  4: i32 endLine;
  5: i32 endColumn;
}

/**
 * Information about the comment preceding a definition (like this one!).
 */
@python.Py3Hidden
struct DocBlock {
  1: string contents;
  2: SourceRange sourceRange;
}

/**
 * A reference to a Thrift interface type.
 * Analogous to type.Type.
 */
struct InterfaceRef {
  1: standard.TypeUri uri;
// TODO: symbol span
}

/**
 * A Thrift enum value.
 *
 *     enum ... {
 *       {attrs.name} = {value}
 *     }
 */
@python.Py3Hidden
struct EnumValue {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /**
   * The associated numeric value.
   *
   * Changing value is always backward *incompatible*.
   */
  // TODO(afuller): Consider adding support for specifying aliases by specifying
  // multiple definitions for the same value in the IDL.
  2: i32 value;
}

/**
 * A Thrift enum.
 *
 *     enum {attrs.name} { ... values ... }
 */
@python.Py3Hidden
struct Enum {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /**
   * The values, in the order as defined in the IDL/AST.
   *
   * Changing the order of values is always backward compatible.
   */
  2: list<EnumValue> values;
}

/** The field qualifier. */
enum FieldQualifier {
  /** Unqualified: always written. */
  Default = 0,
  /** `optional`: written if explicitly 'set'. */
  Optional = 1,
  /** `@thrift.TerseWrite`: written if not 'empty'. */
  Terse = 2,
  /** `required`: always written, not actually required. */
  Fill = 3,
}

/**
 * A Thrift field.
 *
 *     {id}: {qualifier} {type} {attrs.name} = {customDefault}
 */
@python.Py3Hidden
struct Field {
  /**
   * The static ID specified for the field.
   *
   * Changing the field ID is always backward *incompatible*.
   */
  1: id.FieldId id;

  /** The qualifier for the field. */
  // TODO(afuller): Document compatibility semantics, and add conformance tests.
  2: FieldQualifier qualifier;

  /** The type of the field. */
  // TODO(afuller): Document compatibility semantics, and add conformance tests.
  3: type.Type type;

  /** The definition attributes. */
  @thrift.Mixin
  4: DefinitionAttrs attrs;

  /**
   * The custom default value for this field.
   *
   * If no value is set, the intrinsic default for the field type is used.
   */
  // TODO(afuller): Document compatibility semantics, and add conformance tests.
  5: id.ValueId customDefault;
}

/**
 * A container for the fields of a structured type (e.g. struct, union, exception).
 *
 * Changing the order of fields is always backward compatible.
 */
// TODO(afuller): Add native wrappers that provide O(1) access to fields by id,
// name, type, etc.
@python.Py3Hidden
@thrift.Experimental // TODO: Adapt!
typedef list<Field> Fields

/**
 * A Thrift struct.
 *
 *     struct {attrs.name} { ... fields ... }
 */
@python.Py3Hidden
struct Struct {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /**
   * The fields, in the order as defined in the IDL/AST.
   *
   * Changing the order of the fields is always backward compatible.
   */
  2: Fields fields;
}

/**
 * A Thrift union.
 *
 *   union {attrs.name} { ... fields ... }
 */
@python.Py3Hidden
struct Union {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /**
   * The fields, in the order as defined in the IDL/AST.
   *
   * Changing the order of the fields is always backward compatible.
   */
  2: Fields fields;
}

/** The error kind. */
enum ErrorKind {
  /** The error kind was not specified. The associated RPC might succeed if retried. */
  Unspecified = 0,
  /** The associated RPC might succeed if retried. */
  Transient = 1,
  /** The server state must be change for the associated RPC to have any chance of succeeding. */
  Stateful = 2,
  /** The associated RPC can never succeed and should not be retried. */
  Permanent = 3,
}

/** The error blame. */
enum ErrorBlame {
  /** The blame for the error was not specified. */
  Unspecified = 0,
  /** The error was the fault of the server. */
  Server = 1,
  /** The error was the fault of the client's request. */
  Client = 2,
}

/** The error safety. */
enum ErrorSafety {
  /** The safety for the error was not specified. */
  Unspecified = 0,
  /** The failed RPC has no side effects. */
  Safe = 1,
}

/**
 * A Thrift exception.
 *
 *  {safety} {kind} {blame} exception {attrs.name} { ... fields ... }
 */
@python.Py3Hidden
struct Exception {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /**
   * The fields, in the order as defined in the IDL/AST.
   *
   * Changing the order of the fields is always backward compatible.
   */
  2: Fields fields;

  /** The safety of the exception. */
  3: ErrorSafety safety;

  /** The error kind of the exception. */
  4: ErrorKind kind;

  /** The fault attribution of the exception. */
  5: ErrorBlame blame;
}

/**
 * A container of exceptions.
 *
 * Changing the order of exceptions is always backward compatible.
 */
@python.Py3Hidden
@thrift.Experimental // TODO: Adapt!
typedef list<Field> Exceptions

/**
 * A Thrift Param list. A param list is unnamed.
 */
// TODO(dokwon): Ensure a param list is unnamed.
// TODO(dokwon): Consider replacing (or removing) 'Paramlist' with 'Fields'.
@python.Py3Hidden
struct Paramlist {
  /**
   * The fields, in the order as defined in the IDL/AST.
   *
   * Changing the order of the fields is always backward compatible.
   */
  1: Fields fields;
}

/** The function qualifier. */
enum FunctionQualifier {
  Unspecified = 0,
  /** Client does not expect response back from server. */
  OneWay = 1,
  /** Safe to retry immediately after a transient failure. */
  Idempotent = 2,
  /** Always safe to retry. */
  ReadOnly = 3,
}

/**
 * A Thrift stream type.
 *
 *     stream<{payload} throws (... exceptions ...)>
 */
@python.Py3Hidden
struct Stream {
  /** The payload from the stream. */
  1: type.Type payload;
  /** The exceptions from the stream. */
  2: Exceptions exceptions;
}

/**
 * A Thrift sink type.
 *
 *     sink<{payload} throws (... clientExceptions ...),
 *          {finalResponse} throws (... serverExceptions ...)>
 */
@python.Py3Hidden
struct Sink {
  /** The payload from the sink. */
  1: type.Type payload;
  /** The exceptions from the client. */
  2: Exceptions clientExceptions;
  /** The final response from the sink. */
  3: type.Type finalResponse;
  /** The exceptions from the server. */
  4: Exceptions serverExceptions;
}

/**
 * A Thrift interaction.
 *
 *     interaction {attrs.name} { ... functions ... }
 */
@python.Py3Hidden
struct Interaction {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /**
   * The functions, in the order as defined in the IDL/AST.
   *
   * Changing the order of the fields is always backward compatible.
   */
  2: Functions functions;
}

/** A Thrift function return type. */
// TODO (T161963504): rename to StreamOrSink
@python.Py3Hidden
union ReturnType {
  // DEPRECATED
  1: type.Type thriftType;
  /** The stream return type. */
  2: Stream streamType;
  /** The sink return type. */
  3: Sink sinkType;
  /** DEPRECATED: The interaction return type. */
  4: id.DefinitionId interactionType;
}

/** A container of Thrift function return type. */
@python.Py3Hidden
typedef list<ReturnType> ReturnTypes

/**
 * A Thrift function.
 *
 *     {qualifier} { ... returnTypes ... } {attrs.name}({paramlist}) throws ( ... exceptions ... )
 */
@python.Py3Hidden
@hack.Name{name = "TFunction"}
struct Function {
  /** The qualifier for the function. */
  // TODO(dokwon): Document compatibility semantics, and add conformance tests.
  1: FunctionQualifier qualifier;

  /** DEPRECATED (T161963504): a list containing `returnType` if present. */
  2: ReturnTypes returnTypes;

  /** The definition attributes. */
  @thrift.Mixin
  3: DefinitionAttrs attrs;

  /** The paramlist of the function. */
  4: Paramlist paramlist;

  /** The exceptions of the function. */
  5: Exceptions exceptions;

  /** The return or first response type of the function. */
  6: type.Type returnType;

  /** The stream or sink type returned by the function. */
  7: ReturnType streamOrSink;

  /** The interaction type created by the function. */
  8: InterfaceRef interactionType;
}

/**
 * A container for the functions of a interface type (e.g. service and interaction).
 *
 * Changing the order of fields is always backward compatible.
 */
@python.Py3Hidden
@thrift.Experimental // TODO: Adapt!
typedef list<Function> Functions

/**
 * A Thrift service.
 *
 *     service {attrs.name} [extends {baseService}] { ... functions ... }
 */
@python.Py3Hidden
struct Service {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /**
   * The functions, in the order as defined in the IDL/AST.
   *
   * Changing the order of the fields is always backward compatible.
   */
  2: Functions functions;

  /** The service that it inherits functions from. */
  3: InterfaceRef baseService;
}

/**
 * A Thrift constant.
 *
 *     const {type} {attrs.name} = {value}
 */
@python.Py3Hidden
@hack.Name{name = "TConst"}
struct Const {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /** The type of the constant. */
  2: type.Type type;

  /** The value the const is initialized to. */
  3: id.ValueId value;
}

/**
 * A Thrift typedef.
 *
 *     typedef {type} {attrs.name}
 */
@python.Py3Hidden
struct Typedef {
  /** The definition attributes. */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /** The underlying type. */
  2: type.Type type;
}

/**
 * Any Thrift definition.
 *
 * Each type must have DefinitionAttrs.
 */
@python.Py3Hidden
union Definition {
  1: Struct structDef;
  2: Union unionDef;
  3: Exception exceptionDef;
  4: Enum enumDef;
  5: Typedef typedefDef;
  6: Const constDef;
  7: Service serviceDef;
  8: Interaction interactionDef;
}

/** A list of definitions (Structs, Enums, Services, etc), accessible by `DefinitionId`. */
@python.Py3Hidden
typedef list<Definition> DefinitionList

/**
 * A Thrift program.
 *
 *     {attrs.name}.thrift:
 *       ... {attrs.annotations} ...
 *       package {package/definition.uri}
 *
 *       ... {includes} ...
 *
 *       ... {definitions} ...
 */
@python.Py3Hidden
struct Program {
  /**
   * The definition attributes.
   * The package name is available as attrs.uri.
   */
  @thrift.Mixin
  1: DefinitionAttrs attrs;

  /**
   * The included programs, in the order included in the IDL/AST.
   *
   * Changing the order of includes is always backward compatible.
   */
  3: IncludeIds includes;

  /**
   * The definitions included in this program, in the order declared in the
   * IDL/AST.
   *
   * Changing the order of definitions is always backward compatible.
   */
  // TODO(afuller): Fix type resolution order bugs in the parser to make this
  // comment true in all cases.
  4: DefinitionIds definitions;

  /**
   * The raw path used to identify this program's file to the compiler.
   */
  5: string path;

  /**
   * Per-language include statements.
   * The key is the beginning of the *_include keyword ("cpp", "hs").
   */
  6: map<string, list<string>> languageIncludes;

  /**
   * Per-language namespace.
   * The key matches the IDL ("cpp2", "java.swift").
   */
  7: map<string, string> namespaces;
}

/** A list of programs, accessible by `ProgramId`. */
@python.Py3Hidden
typedef list<Program> ProgramList

/**
 * A Thrift schema represented as a collection of Thrift programs and associated
 * schema values.
 */
// TODO(afuller): Add native wrappers/adapters that 'index' all the stored
// information and maybe even converts stored `ExternId`s into pointers to the
// values owned by the schema.
@python.Py3Hidden
@thrift.Experimental // TODO(afuller): Adapt!
@cpp.UseOpEncode
struct Schema {
  /** The programs included in the schema, accessible by `ProgramId`. */
  1: ProgramList programs;

  /** The instantiated types, accessible by `TypeId`. */
  2: type.TypeList types;

  /** The values, accessible by `ValueId`. */
  3: list<protocol.Value> values;

  /** The definitions, accessible by `DefinitionId`. */
  5: DefinitionList definitions;
}
