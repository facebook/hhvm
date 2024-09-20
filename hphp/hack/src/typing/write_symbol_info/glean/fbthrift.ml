(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *)

(* @generated
   regenerate: buck2 run fbcode//glean/schema/gen:gen-schema  -- --ocaml fbcode/hphp/hack/src/typing/write_symbol_info/schema --dir DEST_DIR *)

[@@@warning "-33-39"]
open Hh_json
open Core


module rec StructuredAnnotation: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    type_: TypeSpecification.t;
    value: StructVal.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    type_: TypeSpecification.t;
    value: StructVal.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {type_; value} = 
    let fields = [
      ("type_", TypeSpecification.to_json type_);
      ("value", StructVal.to_json value);
    ] in
    JSON_Object fields

end

and File: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = Src.File.t
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = Src.File.t
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = Src.File.to_json x
end

and FunctionName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    service_: ServiceName.t;
    name: Identifier.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    service_: ServiceName.t;
    name: Identifier.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {service_; name} = 
    let fields = [
      ("service_", ServiceName.to_json service_);
      ("name", Identifier.to_json name);
    ] in
    JSON_Object fields

end

and FileXRefs: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: File.t;
    xrefs: XRef.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: File.t;
    xrefs: XRef.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; xrefs} = 
    let fields = [
      ("file", File.to_json file);
      ("xrefs", JSON_Array (List.map ~f:(fun x -> XRef.to_json x) xrefs));
    ] in
    JSON_Object fields

end

and NamedDecl: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: NamedType.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: NamedType.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", NamedType.to_json name);
    ] in
    JSON_Object fields

end

and Identifier: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = JSON_String x
end

and EnumValueDef: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: EnumValue.t;
    value: IntegerLiteral.t;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: EnumValue.t;
    value: IntegerLiteral.t;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; value; structured_annotations} = 
    let fields = [
      ("name", EnumValue.to_json name);
      ("value", IntegerLiteral.to_json value);
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and ServiceChild: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    parent: ServiceName.t;
    child: ServiceName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    parent: ServiceName.t;
    child: ServiceName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {parent; child} = 
    let fields = [
      ("parent", ServiceName.to_json parent);
      ("child", ServiceName.to_json child);
    ] in
    JSON_Object fields

end

and FunctionDeclarationName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    qname: QualName.t;
    name: Identifier.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    qname: QualName.t;
    name: Identifier.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {qname; name; decl} = 
    let fields = [
      ("qname", QualName.to_json qname);
      ("name", Identifier.to_json name);
      ("decl", Declaration.to_json decl);
    ] in
    JSON_Object fields

end

and ExceptionName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QualName.to_json name);
    ] in
    JSON_Object fields

end

and FileDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: File.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: File.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; decl} = 
    let fields = [
      ("file", File.to_json file);
      ("decl", Declaration.to_json decl);
    ] in
    JSON_Object fields

end

and ServiceName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QualName.to_json name);
    ] in
    JSON_Object fields

end

and ServiceParent: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    child: ServiceName.t;
    parent: ServiceName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    child: ServiceName.t;
    parent: ServiceName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {child; parent} = 
    let fields = [
      ("child", ServiceName.to_json child);
      ("parent", ServiceName.to_json parent);
    ] in
    JSON_Object fields

end

and TypeDefException: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    alias: NamedDecl.t;
    type_: ExceptionSpecName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    alias: NamedDecl.t;
    type_: ExceptionSpecName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {alias; type_} = 
    let fields = [
      ("alias", NamedDecl.to_json alias);
      ("type_", ExceptionSpecName.to_json type_);
    ] in
    JSON_Object fields

end

and DeclarationNameSpan: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    decl: Declaration.t;
    name: Identifier.t;
    file: File.t;
    span: Loc.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    decl: Declaration.t;
    name: Identifier.t;
    file: File.t;
    span: Loc.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {decl; name; file; span} = 
    let fields = [
      ("decl", Declaration.to_json decl);
      ("name", Identifier.to_json name);
      ("file", File.to_json file);
      ("span", Loc.to_json span);
    ] in
    JSON_Object fields

end

and Constant: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QualName.to_json name);
    ] in
    JSON_Object fields

end

and QualName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: File.t;
    name: Identifier.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: File.t;
    name: Identifier.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; name} = 
    let fields = [
      ("file", File.to_json file);
      ("name", Identifier.to_json name);
    ] in
    JSON_Object fields

end

and InteractionDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: InteractionName.t;
    functions: FunctionSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: InteractionName.t;
    functions: FunctionSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; functions; structured_annotations} = 
    let fields = [
      ("name", InteractionName.to_json name);
      ("functions", JSON_Array (List.map ~f:(fun x -> FunctionSpecification.to_json x) functions));
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and ConstantDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    constant: Constant.t;
    definition: TypedConstT.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    constant: Constant.t;
    definition: TypedConstT.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {constant; definition} = 
    let fields = [
      ("constant", Constant.to_json constant);
      ("definition", TypedConstT.to_json definition);
    ] in
    JSON_Object fields

end

and EnumValue: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    enum_: NamedType.t;
    name: Identifier.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    enum_: NamedType.t;
    name: Identifier.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {enum_; name} = 
    let fields = [
      ("enum_", NamedType.to_json enum_);
      ("name", Identifier.to_json name);
    ] in
    JSON_Object fields

end

and NamespaceValue: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = JSON_String x
end

and StructType: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
    fields: FieldSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
    fields: FieldSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; fields; structured_annotations} = 
    let fields = [
      ("name", QualName.to_json name);
      ("fields", JSON_Array (List.map ~f:(fun x -> FieldSpecification.to_json x) fields));
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and NameLowerCase: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name_str: string;
    name: Identifier.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name_str: string;
    name: Identifier.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name_str; name} = 
    let fields = [
      ("nameStr", JSON_String name_str);
      ("name", Identifier.to_json name);
    ] in
    JSON_Object fields

end

and StructVal: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    fields: StructFieldVal.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    fields: StructFieldVal.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {fields} = 
    let fields = [
      ("fields", JSON_Array (List.map ~f:(fun x -> StructFieldVal.to_json x) fields));
    ] in
    JSON_Object fields

end

and FieldDecl: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    qname: QualName.t;
    kind: FieldKind.t;
    name: Identifier.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    qname: QualName.t;
    kind: FieldKind.t;
    name: Identifier.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {qname; kind; name} = 
    let fields = [
      ("qname", QualName.to_json qname);
      ("kind", FieldKind.to_json kind);
      ("name", Identifier.to_json name);
    ] in
    JSON_Object fields

end

and ExceptionVal: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = StructVal.t
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = StructVal.t
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = StructVal.to_json x
end

and TypeSpecification: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = 
     | Primitive of PrimitiveType.t
     | Container of ContainerType.t
     | Named of NamedType.t
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = 
     | Primitive of PrimitiveType.t
     | Container of ContainerType.t
     | Named of NamedType.t
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key  = function
     | Primitive primitive -> JSON_Object [("primitive", PrimitiveType.to_json primitive)]
     | Container container -> JSON_Object [("container", ContainerType.to_json container)]
     | Named named -> JSON_Object [("named", NamedType.to_json named)]

end

and EnumerationType: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
    value: EnumValueDef.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
    value: EnumValueDef.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; value; structured_annotations} = 
    let fields = [
      ("name", QualName.to_json name);
      ("value", JSON_Array (List.map ~f:(fun x -> EnumValueDef.to_json x) value));
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and ExceptionType: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
    fields: FieldSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
    fields: FieldSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; fields; structured_annotations} = 
    let fields = [
      ("name", QualName.to_json name);
      ("fields", JSON_Array (List.map ~f:(fun x -> FieldSpecification.to_json x) fields));
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and UnionType: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
    alts: UnqualField.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
    alts: UnqualField.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; alts; structured_annotations} = 
    let fields = [
      ("name", QualName.to_json name);
      ("alts", JSON_Array (List.map ~f:(fun x -> UnqualField.to_json x) alts));
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and NamespaceName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = JSON_String x
end

and ServiceDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: ServiceName.t;
    functions: FunctionSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
    interactions: InteractionName.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: ServiceName.t;
    functions: FunctionSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
    interactions: InteractionName.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; functions; structured_annotations; interactions} = 
    let fields = [
      ("name", ServiceName.to_json name);
      ("functions", JSON_Array (List.map ~f:(fun x -> FunctionSpecification.to_json x) functions));
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
      ("interactions", JSON_Array (List.map ~f:(fun x -> InteractionName.to_json x) interactions));
    ] in
    JSON_Object fields

end

and Package: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: File.t;
    name: PackageName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: File.t;
    name: PackageName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; name} = 
    let fields = [
      ("target", File.to_json target);
      ("name", PackageName.to_json name);
    ] in
    JSON_Object fields

end

and DeclarationComment: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: Declaration.t;
    file: Src.File.t;
    span: Src.ByteSpan.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: Declaration.t;
    file: Src.File.t;
    span: Src.ByteSpan.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; file; span} = 
    let fields = [
      ("declaration", Declaration.to_json declaration);
      ("file", Src.File.to_json file);
      ("span", Src.ByteSpan.to_json span);
    ] in
    JSON_Object fields

end

and InteractionName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QualName.to_json name);
    ] in
    JSON_Object fields

end

and DeclarationFile: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: Declaration.t;
    file: File.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: Declaration.t;
    file: File.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; file} = 
    let fields = [
      ("declaration", Declaration.to_json declaration);
      ("file", File.to_json file);
    ] in
    JSON_Object fields

end

and DeclarationName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    qname: QualName.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    qname: QualName.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {qname; decl} = 
    let fields = [
      ("qname", QualName.to_json qname);
      ("decl", Declaration.to_json decl);
    ] in
    JSON_Object fields

end

and UnionVal: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    field: UnionFieldVal.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    field: UnionFieldVal.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {field} = 
    let fields = [
      ("field", UnionFieldVal.to_json field);
    ] in
    JSON_Object fields

end

and EnumVal: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QualName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QualName.to_json name);
    ] in
    JSON_Object fields

end

and DeclarationUses: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: Declaration.t;
    file: Src.File.t;
    range: Loc.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: Declaration.t;
    file: Src.File.t;
    range: Loc.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; file; range} = 
    let fields = [
      ("target", Declaration.to_json target);
      ("file", Src.File.to_json file);
      ("range", Loc.to_json range);
    ] in
    JSON_Object fields

end

and FunctionSpecification: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: FunctionName.t;
    result: ResultType.t;
    arguments: UnqualField.t list;
    throws_: ExceptionSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: FunctionName.t;
    result: ResultType.t;
    arguments: UnqualField.t list;
    throws_: ExceptionSpecification.t list;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; result; arguments; throws_; structured_annotations} = 
    let fields = [
      ("name", FunctionName.to_json name);
      ("result", ResultType.to_json result);
      ("arguments", JSON_Array (List.map ~f:(fun x -> UnqualField.to_json x) arguments));
      ("throws_", JSON_Array (List.map ~f:(fun x -> ExceptionSpecification.to_json x) throws_));
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and PackageName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = JSON_String x
end

and Namespace: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: File.t;
    name: NamespaceName.t;
    namespace_: NamespaceValue.t;
    quoted: bool;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: File.t;
    name: NamespaceName.t;
    namespace_: NamespaceValue.t;
    quoted: bool;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; name; namespace_; quoted} = 
    let fields = [
      ("target", File.to_json target);
      ("name", NamespaceName.to_json name);
      ("namespace_", NamespaceValue.to_json namespace_);
      ("quoted", JSON_Bool quoted);
    ] in
    JSON_Object fields

end

and SearchByName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Identifier.t;
    qname: QualName.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Identifier.t;
    qname: QualName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; qname} = 
    let fields = [
      ("name", Identifier.to_json name);
      ("qname", QualName.to_json qname);
    ] in
    JSON_Object fields

end

and ServiceInteractionFunctions: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: ServiceName.t;
    function_: FunctionSpecification.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: ServiceName.t;
    function_: FunctionSpecification.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; function_} = 
    let fields = [
      ("name", ServiceName.to_json name);
      ("function_", FunctionSpecification.to_json function_);
    ] in
    JSON_Object fields

end

and DeclarationMember: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    qname: QualName.t;
    member: Identifier.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    qname: QualName.t;
    member: Identifier.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {qname; member; decl} = 
    let fields = [
      ("qname", QualName.to_json qname);
      ("member", Identifier.to_json member);
      ("decl", Declaration.to_json decl);
    ] in
    JSON_Object fields

end

and TypeDefType: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    alias: QualName.t;
    type_: TypeSpecification.t;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    alias: QualName.t;
    type_: TypeSpecification.t;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {alias; type_; structured_annotations} = 
    let fields = [
      ("alias", QualName.to_json alias);
      ("type_", TypeSpecification.to_json type_);
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and Literal: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = 
     | Byte_ of IntegerLiteral.t
     | I16_ of IntegerLiteral.t
     | I32_ of IntegerLiteral.t
     | I64_ of IntegerLiteral.t
     | Float_ of FloatLiteral.t
     | Double_ of FloatLiteral.t
     | Bool_ of bool
     | String_ of string
     | Binary_ of string list
     | Set_ of TypedConst.t list
     | List_ of TypedConst.t list
     | Map_ of KeyValue.t list
     | Newtype_ of Literal.t
     | Struct_ of StructVal.t
     | Exception_ of ExceptionVal.t
     | Union_ of UnionVal.t
     | Enum_ of EnumVal.t
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = 
     | Byte_ of IntegerLiteral.t
     | I16_ of IntegerLiteral.t
     | I32_ of IntegerLiteral.t
     | I64_ of IntegerLiteral.t
     | Float_ of FloatLiteral.t
     | Double_ of FloatLiteral.t
     | Bool_ of bool
     | String_ of string
     | Binary_ of string list
     | Set_ of TypedConst.t list
     | List_ of TypedConst.t list
     | Map_ of KeyValue.t list
     | Newtype_ of Literal.t
     | Struct_ of StructVal.t
     | Exception_ of ExceptionVal.t
     | Union_ of UnionVal.t
     | Enum_ of EnumVal.t
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key  = function
     | Byte_ byte_ -> JSON_Object [("byte_", IntegerLiteral.to_json byte_)]
     | I16_ i16_ -> JSON_Object [("i16_", IntegerLiteral.to_json i16_)]
     | I32_ i32_ -> JSON_Object [("i32_", IntegerLiteral.to_json i32_)]
     | I64_ i64_ -> JSON_Object [("i64_", IntegerLiteral.to_json i64_)]
     | Float_ float_ -> JSON_Object [("float_", FloatLiteral.to_json float_)]
     | Double_ double_ -> JSON_Object [("double_", FloatLiteral.to_json double_)]
     | Bool_ bool_ -> JSON_Object [("bool_", JSON_Bool bool_)]
     | String_ string_ -> JSON_Object [("string_", JSON_String string_)]
     | Binary_ binary_ -> JSON_Object [("binary_", JSON_String (List.map ~f:Base64.encode_string binary_|> String.concat ~sep:""))]
     | Set_ set_ -> JSON_Object [("set_", JSON_Array (List.map ~f:(fun x -> TypedConst.to_json x) set_))]
     | List_ list_ -> JSON_Object [("list_", JSON_Array (List.map ~f:(fun x -> TypedConst.to_json x) list_))]
     | Map_ map_ -> JSON_Object [("map_", JSON_Array (List.map ~f:(fun x -> KeyValue.to_json x) map_))]
     | Newtype_ newtype_ -> JSON_Object [("newtype_", Literal.to_json newtype_)]
     | Struct_ struct_ -> JSON_Object [("struct_", StructVal.to_json struct_)]
     | Exception_ exception_ -> JSON_Object [("exception_", ExceptionVal.to_json exception_)]
     | Union_ union_ -> JSON_Object [("union_", UnionVal.to_json union_)]
     | Enum_ enum_ -> JSON_Object [("enum_", EnumVal.to_json enum_)]

end


and UnionFieldVal: sig
  type t = {
    name: Identifier.t;
    value: TypedConstT.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    name: Identifier.t;
    value: TypedConstT.t;
  }
  [@@deriving ord]

  let rec to_json {name; value} = 
    let fields = [
      ("name", Identifier.to_json name);
      ("value", TypedConstT.to_json value);
    ] in
    JSON_Object fields

end

and ResultSink: sig
  type t = {
    type_: TypeSpecification.t;
    first_response: TypeSpecification.t option;
    final_response: TypeSpecification.t option;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    type_: TypeSpecification.t;
    first_response: TypeSpecification.t option;
    final_response: TypeSpecification.t option;
  }
  [@@deriving ord]

  let rec to_json {type_; first_response; final_response} = 
    let fields = [
      ("type_", TypeSpecification.to_json type_);
    ] in
    let fields =
      match first_response with
      | None -> fields
      | Some first_response -> ("firstResponse", TypeSpecification.to_json first_response) :: fields in
    let fields =
      match final_response with
      | None -> fields
      | Some final_response -> ("finalResponse", TypeSpecification.to_json final_response) :: fields in
    JSON_Object fields

end

and ExceptionSpecName: sig
  type t = 
     | Simple of ExceptionName.t
     | Typedef_ of TypeDefException.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Simple of ExceptionName.t
     | Typedef_ of TypeDefException.t
  [@@deriving ord]

  let rec to_json  = function
     | Simple simple -> JSON_Object [("simple", ExceptionName.to_json simple)]
     | Typedef_ typedef_ -> JSON_Object [("typedef_", TypeDefException.to_json typedef_)]

end

and ResultType: sig
  type t = 
     | Oneway_ of Builtin.Unit.t
     | Void_ of Builtin.Unit.t
     | Result of TypeSpecification.t
     | Stream_ of ResultStream.t
     | Service_ of ServiceName.t
     | Sink_ of ResultSink.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Oneway_ of Builtin.Unit.t
     | Void_ of Builtin.Unit.t
     | Result of TypeSpecification.t
     | Stream_ of ResultStream.t
     | Service_ of ServiceName.t
     | Sink_ of ResultSink.t
  [@@deriving ord]

  let rec to_json  = function
     | Oneway_ oneway_ -> JSON_Object [("oneway_", Builtin.Unit.to_json oneway_)]
     | Void_ void_ -> JSON_Object [("void_", Builtin.Unit.to_json void_)]
     | Result result -> JSON_Object [("result", TypeSpecification.to_json result)]
     | Stream_ stream_ -> JSON_Object [("stream_", ResultStream.to_json stream_)]
     | Service_ service_ -> JSON_Object [("service_", ServiceName.to_json service_)]
     | Sink_ sink_ -> JSON_Object [("sink_", ResultSink.to_json sink_)]

end

and FieldKind: sig
  type t = 
    | Struct_
    | Union_
    | Exception_

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Struct_
    | Union_
    | Exception_

  [@@deriving ord]

  let rec to_json  = function
     | Struct_ -> JSON_Number (string_of_int 0)
     | Union_ -> JSON_Number (string_of_int 1)
     | Exception_ -> JSON_Number (string_of_int 2)

end

and IntegerLiteral: sig
  type t = {
    is_non_negative: bool;
    abs_value: int;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    is_non_negative: bool;
    abs_value: int;
  }
  [@@deriving ord]

  let rec to_json {is_non_negative; abs_value} = 
    let fields = [
      ("isNonNegative", JSON_Bool is_non_negative);
      ("absValue", JSON_Number (string_of_int abs_value));
    ] in
    JSON_Object fields

end

and TypedConst: sig
  type t = 
     | Literal of Literal.t
     | Identifier of Constant.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Literal of Literal.t
     | Identifier of Constant.t
  [@@deriving ord]

  let rec to_json  = function
     | Literal literal -> JSON_Object [("literal", Literal.to_json literal)]
     | Identifier identifier -> JSON_Object [("identifier", Constant.to_json identifier)]

end

and Qualifier: sig
  type t = 
    | Default_
    | Optional_
    | Required_

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Default_
    | Optional_
    | Required_

  [@@deriving ord]

  let rec to_json  = function
     | Default_ -> JSON_Number (string_of_int 0)
     | Optional_ -> JSON_Number (string_of_int 1)
     | Required_ -> JSON_Number (string_of_int 2)

end

and MapType: sig
  type t = {
    key_: TypeSpecification.t;
    value: TypeSpecification.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    key_: TypeSpecification.t;
    value: TypeSpecification.t;
  }
  [@@deriving ord]

  let rec to_json {key_; value} = 
    let fields = [
      ("key_", TypeSpecification.to_json key_);
      ("value", TypeSpecification.to_json value);
    ] in
    JSON_Object fields

end

and Declaration: sig
  type t = XRefTarget.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = XRefTarget.t
  [@@deriving ord]

  let rec to_json x = XRefTarget.to_json x
end

and Loc: sig
  type t = {
    start_line: int;
    start_col: int;
    end_line: int;
    end_col: int;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    start_line: int;
    start_col: int;
    end_line: int;
    end_col: int;
  }
  [@@deriving ord]

  let rec to_json {start_line; start_col; end_line; end_col} = 
    let fields = [
      ("startLine", JSON_Number (string_of_int start_line));
      ("startCol", JSON_Number (string_of_int start_col));
      ("endLine", JSON_Number (string_of_int end_line));
      ("endCol", JSON_Number (string_of_int end_col));
    ] in
    JSON_Object fields

end

and FieldSpecification: sig
  type t = {
    id: FieldId.t;
    qualifier: Qualifier.t;
    type_: TypeSpecification.t;
    name: Identifier.t;
    value: TypedConst.t option;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    id: FieldId.t;
    qualifier: Qualifier.t;
    type_: TypeSpecification.t;
    name: Identifier.t;
    value: TypedConst.t option;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json {id; qualifier; type_; name; value; structured_annotations} = 
    let fields = [
      ("id", FieldId.to_json id);
      ("qualifier", Qualifier.to_json qualifier);
      ("type_", TypeSpecification.to_json type_);
      ("name", Identifier.to_json name);
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    let fields =
      match value with
      | None -> fields
      | Some value -> ("value", TypedConst.to_json value) :: fields in
    JSON_Object fields

end

and PrimitiveType: sig
  type t = 
    | Bool_
    | Byte_
    | I16_
    | I32_
    | I64_
    | Float_
    | Double_
    | Binary_
    | String_

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Bool_
    | Byte_
    | I16_
    | I32_
    | I64_
    | Float_
    | Double_
    | Binary_
    | String_

  [@@deriving ord]

  let rec to_json  = function
     | Bool_ -> JSON_Number (string_of_int 0)
     | Byte_ -> JSON_Number (string_of_int 1)
     | I16_ -> JSON_Number (string_of_int 2)
     | I32_ -> JSON_Number (string_of_int 3)
     | I64_ -> JSON_Number (string_of_int 4)
     | Float_ -> JSON_Number (string_of_int 5)
     | Double_ -> JSON_Number (string_of_int 6)
     | Binary_ -> JSON_Number (string_of_int 7)
     | String_ -> JSON_Number (string_of_int 8)

end

and FloatLiteral: sig
  type t = {
    is_na_n: bool;
    is_positive: bool;
    exponent: int;
    significand: int;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    is_na_n: bool;
    is_positive: bool;
    exponent: int;
    significand: int;
  }
  [@@deriving ord]

  let rec to_json {is_na_n; is_positive; exponent; significand} = 
    let fields = [
      ("isNaN", JSON_Bool is_na_n);
      ("isPositive", JSON_Bool is_positive);
      ("exponent", JSON_Number (string_of_int exponent));
      ("significand", JSON_Number (string_of_int significand));
    ] in
    JSON_Object fields

end

and NamedType: sig
  type t = {
    name: QualName.t;
    kind: NamedKind.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    name: QualName.t;
    kind: NamedKind.t;
  }
  [@@deriving ord]

  let rec to_json {name; kind} = 
    let fields = [
      ("name", QualName.to_json name);
      ("kind", NamedKind.to_json kind);
    ] in
    JSON_Object fields

end

and XRefTarget: sig
  type t = 
     | Include_ of File.t
     | Named of NamedDecl.t
     | Exception_ of ExceptionName.t
     | Service_ of ServiceName.t
     | Constant of Constant.t
     | EnumValue of EnumValue.t
     | Function_ of FunctionName.t
     | Field of FieldDecl.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Include_ of File.t
     | Named of NamedDecl.t
     | Exception_ of ExceptionName.t
     | Service_ of ServiceName.t
     | Constant of Constant.t
     | EnumValue of EnumValue.t
     | Function_ of FunctionName.t
     | Field of FieldDecl.t
  [@@deriving ord]

  let rec to_json  = function
     | Include_ include_ -> JSON_Object [("include_", File.to_json include_)]
     | Named named -> JSON_Object [("named", NamedDecl.to_json named)]
     | Exception_ exception_ -> JSON_Object [("exception_", ExceptionName.to_json exception_)]
     | Service_ service_ -> JSON_Object [("service_", ServiceName.to_json service_)]
     | Constant constant -> JSON_Object [("constant", Constant.to_json constant)]
     | EnumValue enum_value -> JSON_Object [("enumValue", EnumValue.to_json enum_value)]
     | Function_ function_ -> JSON_Object [("function_", FunctionName.to_json function_)]
     | Field field -> JSON_Object [("field", FieldDecl.to_json field)]

end

and XRef: sig
  type t = {
    loc_ref: Loc.t;
    target: XRefTarget.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    loc_ref: Loc.t;
    target: XRefTarget.t;
  }
  [@@deriving ord]

  let rec to_json {loc_ref; target} = 
    let fields = [
      ("locRef", Loc.to_json loc_ref);
      ("target", XRefTarget.to_json target);
    ] in
    JSON_Object fields

end

and Target: sig
  type t = {
    target: XRefTarget.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    target: XRefTarget.t;
  }
  [@@deriving ord]

  let rec to_json {target} = 
    let fields = [
      ("target", XRefTarget.to_json target);
    ] in
    JSON_Object fields

end

and FieldId: sig
  type t = IntegerLiteral.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = IntegerLiteral.t
  [@@deriving ord]

  let rec to_json x = IntegerLiteral.to_json x
end

and NamedKind: sig
  type t = 
    | Typedef_
    | Enum_
    | Struct_
    | Union_

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Typedef_
    | Enum_
    | Struct_
    | Union_

  [@@deriving ord]

  let rec to_json  = function
     | Typedef_ -> JSON_Number (string_of_int 0)
     | Enum_ -> JSON_Number (string_of_int 1)
     | Struct_ -> JSON_Number (string_of_int 2)
     | Union_ -> JSON_Number (string_of_int 3)

end

and TypedConstT: sig
  type t = {
    const_: TypedConst.t;
    type_: TypeSpecification.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    const_: TypedConst.t;
    type_: TypeSpecification.t;
  }
  [@@deriving ord]

  let rec to_json {const_; type_} = 
    let fields = [
      ("const_", TypedConst.to_json const_);
      ("type_", TypeSpecification.to_json type_);
    ] in
    JSON_Object fields

end

and KeyValue: sig
  type t = {
    key: TypedConst.t;
    value: TypedConst.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    key: TypedConst.t;
    value: TypedConst.t;
  }
  [@@deriving ord]

  let rec to_json {key; value} = 
    let fields = [
      ("key", TypedConst.to_json key);
      ("value", TypedConst.to_json value);
    ] in
    JSON_Object fields

end

and StructFieldVal: sig
  type t = {
    name: Identifier.t;
    value: StructFieldValValue.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    name: Identifier.t;
    value: StructFieldValValue.t;
  }
  [@@deriving ord]

  let rec to_json {name; value} = 
    let fields = [
      ("name", Identifier.to_json name);
      ("value", StructFieldValValue.to_json value);
    ] in
    JSON_Object fields

end

and UnqualField: sig
  type t = {
    id: FieldId.t;
    type_: TypeSpecification.t;
    name: Identifier.t;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    id: FieldId.t;
    type_: TypeSpecification.t;
    name: Identifier.t;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json {id; type_; name; structured_annotations} = 
    let fields = [
      ("id", FieldId.to_json id);
      ("type_", TypeSpecification.to_json type_);
      ("name", Identifier.to_json name);
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and ResultStream: sig
  type t = {
    response: TypeSpecification.t option;
    stream_: TypeSpecification.t;
    throws_: ExceptionSpecification.t list;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    response: TypeSpecification.t option;
    stream_: TypeSpecification.t;
    throws_: ExceptionSpecification.t list;
  }
  [@@deriving ord]

  let rec to_json {response; stream_; throws_} = 
    let fields = [
      ("stream_", TypeSpecification.to_json stream_);
      ("throws_", JSON_Array (List.map ~f:(fun x -> ExceptionSpecification.to_json x) throws_));
    ] in
    let fields =
      match response with
      | None -> fields
      | Some response -> ("response", TypeSpecification.to_json response) :: fields in
    JSON_Object fields

end

and ExceptionSpecification: sig
  type t = {
    id: FieldId.t;
    type_: ExceptionSpecName.t;
    name: Identifier.t;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    id: FieldId.t;
    type_: ExceptionSpecName.t;
    name: Identifier.t;
    structured_annotations: StructuredAnnotation.t list;
  }
  [@@deriving ord]

  let rec to_json {id; type_; name; structured_annotations} = 
    let fields = [
      ("id", FieldId.to_json id);
      ("type_", ExceptionSpecName.to_json type_);
      ("name", Identifier.to_json name);
      ("structuredAnnotations", JSON_Array (List.map ~f:(fun x -> StructuredAnnotation.to_json x) structured_annotations));
    ] in
    JSON_Object fields

end

and ContainerType: sig
  type t = 
     | List_ of TypeSpecification.t
     | Set_ of TypeSpecification.t
     | Map_ of MapType.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | List_ of TypeSpecification.t
     | Set_ of TypeSpecification.t
     | Map_ of MapType.t
  [@@deriving ord]

  let rec to_json  = function
     | List_ list_ -> JSON_Object [("list_", TypeSpecification.to_json list_)]
     | Set_ set_ -> JSON_Object [("set_", TypeSpecification.to_json set_)]
     | Map_ map_ -> JSON_Object [("map_", MapType.to_json map_)]

end

and StructFieldValValue: sig
  type t = 
     | Val of TypedConstT.t
     | Default_ of TypeSpecification.t
     | Just of TypedConstT.t
     | Nothing of unit
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Val of TypedConstT.t
     | Default_ of TypeSpecification.t
     | Just of TypedConstT.t
     | Nothing of unit
  [@@deriving ord]

  let rec to_json  = function
     | Val val_ -> JSON_Object [("val", TypedConstT.to_json val_)]
     | Default_ default_ -> JSON_Object [("default_", TypeSpecification.to_json default_)]
     | Just just -> JSON_Object [("just", TypedConstT.to_json just)]
     | Nothing nothing -> JSON_Object [("nothing", (ignore nothing; JSON_Object []))]

end


