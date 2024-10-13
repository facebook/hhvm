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


module rec DeclarationName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = Declaration.t
  [@@deriving ord]
  and value = Name.t
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json
  val to_json_value: value -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = Declaration.t
  [@@deriving ord]
  and value = Name.t
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = Declaration.to_json x
  and to_json_value x = Name.to_json x
end

and TraitDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: TraitDeclaration.t;
    members: Declaration.t list;
    implements_: InterfaceDeclaration.t list;
    uses: TraitDeclaration.t list;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    require_extends: ClassDeclaration.t list;
    require_implements: InterfaceDeclaration.t list;
    module_: ModuleMembership.t option;
    require_class: ClassDeclaration.t list option;
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
    declaration: TraitDeclaration.t;
    members: Declaration.t list;
    implements_: InterfaceDeclaration.t list;
    uses: TraitDeclaration.t list;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    require_extends: ClassDeclaration.t list;
    require_implements: InterfaceDeclaration.t list;
    module_: ModuleMembership.t option;
    require_class: ClassDeclaration.t list option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; members; implements_; uses; attributes; type_params; require_extends; require_implements; module_; require_class} = 
    let fields = [
      ("declaration", TraitDeclaration.to_json declaration);
      ("members", JSON_Array (List.map ~f:(fun x -> Declaration.to_json x) members));
      ("implements_", JSON_Array (List.map ~f:(fun x -> InterfaceDeclaration.to_json x) implements_));
      ("uses", JSON_Array (List.map ~f:(fun x -> TraitDeclaration.to_json x) uses));
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
      ("typeParams", JSON_Array (List.map ~f:(fun x -> TypeParameter.to_json x) type_params));
      ("requireExtends", JSON_Array (List.map ~f:(fun x -> ClassDeclaration.to_json x) require_extends));
      ("requireImplements", JSON_Array (List.map ~f:(fun x -> InterfaceDeclaration.to_json x) require_implements));
    ] in
    let fields =
      match module_ with
      | None -> fields
      | Some module_ -> ("module_", ModuleMembership.to_json module_) :: fields in
    let fields =
      match require_class with
      | None -> fields
      | Some require_class -> ("requireClass", JSON_Array (List.map ~f:(fun x -> ClassDeclaration.to_json x) require_class)) :: fields in
    JSON_Object fields

end

and TraitDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QName.t;
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
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QName.to_json name);
    ] in
    JSON_Object fields

end

and MethodDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: MethodDeclaration.t;
    signature: Signature.t;
    visibility: Visibility.t;
    is_abstract: bool;
    is_async: bool;
    is_final: bool;
    is_static: bool;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    is_readonly_this: bool option;
    readonly_ret: ReadonlyKind.t option;
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
    declaration: MethodDeclaration.t;
    signature: Signature.t;
    visibility: Visibility.t;
    is_abstract: bool;
    is_async: bool;
    is_final: bool;
    is_static: bool;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    is_readonly_this: bool option;
    readonly_ret: ReadonlyKind.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; signature; visibility; is_abstract; is_async; is_final; is_static; attributes; type_params; is_readonly_this; readonly_ret} = 
    let fields = [
      ("declaration", MethodDeclaration.to_json declaration);
      ("signature", Signature.to_json signature);
      ("visibility", Visibility.to_json visibility);
      ("isAbstract", JSON_Bool is_abstract);
      ("isAsync", JSON_Bool is_async);
      ("isFinal", JSON_Bool is_final);
      ("isStatic", JSON_Bool is_static);
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
      ("typeParams", JSON_Array (List.map ~f:(fun x -> TypeParameter.to_json x) type_params));
    ] in
    let fields =
      match is_readonly_this with
      | None -> fields
      | Some is_readonly_this -> ("isReadonlyThis", JSON_Bool is_readonly_this) :: fields in
    let fields =
      match readonly_ret with
      | None -> fields
      | Some readonly_ret -> ("readonlyRet", ReadonlyKind.to_json readonly_ret) :: fields in
    JSON_Object fields

end

and MethodDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    container: ContainerDeclaration.t;
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
    name: Name.t;
    container: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; container} = 
    let fields = [
      ("name", Name.to_json name);
      ("container", ContainerDeclaration.to_json container);
    ] in
    JSON_Object fields

end

and ClassConstDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: ClassConstDeclaration.t;
    value: string option;
    type_info: TypeInfo.t option;
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
    declaration: ClassConstDeclaration.t;
    value: string option;
    type_info: TypeInfo.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; value; type_info} = 
    let fields = [
      ("declaration", ClassConstDeclaration.to_json declaration);
    ] in
    let fields =
      match value with
      | None -> fields
      | Some value -> ("value", JSON_String value) :: fields in
    let fields =
      match type_info with
      | None -> fields
      | Some type_info -> ("typeInfo", TypeInfo.to_json type_info) :: fields in
    JSON_Object fields

end

and GlobalConstDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: GlobalConstDeclaration.t;
    value: string;
    type_info: TypeInfo.t option;
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
    declaration: GlobalConstDeclaration.t;
    value: string;
    type_info: TypeInfo.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; value; type_info} = 
    let fields = [
      ("declaration", GlobalConstDeclaration.to_json declaration);
      ("value", JSON_String value);
    ] in
    let fields =
      match type_info with
      | None -> fields
      | Some type_info -> ("typeInfo", TypeInfo.to_json type_info) :: fields in
    JSON_Object fields

end

and IndexerInputsHash: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]
  and value = string list
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json
  val to_json_value: value -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = string
  [@@deriving ord]
  and value = string list
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = JSON_String x
  and to_json_value x = JSON_String (List.map ~f:Base64.encode_string x|> String.concat ~sep:"")
end

and ClassDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: ClassDeclaration.t;
    is_abstract: bool;
    is_final: bool;
    members: Declaration.t list;
    extends_: ClassDeclaration.t option;
    implements_: InterfaceDeclaration.t list;
    uses: TraitDeclaration.t list;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    module_: ModuleMembership.t option;
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
    declaration: ClassDeclaration.t;
    is_abstract: bool;
    is_final: bool;
    members: Declaration.t list;
    extends_: ClassDeclaration.t option;
    implements_: InterfaceDeclaration.t list;
    uses: TraitDeclaration.t list;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    module_: ModuleMembership.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; is_abstract; is_final; members; extends_; implements_; uses; attributes; type_params; module_} = 
    let fields = [
      ("declaration", ClassDeclaration.to_json declaration);
      ("isAbstract", JSON_Bool is_abstract);
      ("isFinal", JSON_Bool is_final);
      ("members", JSON_Array (List.map ~f:(fun x -> Declaration.to_json x) members));
      ("implements_", JSON_Array (List.map ~f:(fun x -> InterfaceDeclaration.to_json x) implements_));
      ("uses", JSON_Array (List.map ~f:(fun x -> TraitDeclaration.to_json x) uses));
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
      ("typeParams", JSON_Array (List.map ~f:(fun x -> TypeParameter.to_json x) type_params));
    ] in
    let fields =
      match extends_ with
      | None -> fields
      | Some extends_ -> ("extends_", ClassDeclaration.to_json extends_) :: fields in
    let fields =
      match module_ with
      | None -> fields
      | Some module_ -> ("module_", ModuleMembership.to_json module_) :: fields in
    JSON_Object fields

end

and Type: sig
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

and DeclarationLocation: sig
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

and ClassDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QName.t;
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
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QName.to_json name);
    ] in
    JSON_Object fields

end

and QName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    namespace_: NamespaceQName.t option;
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
    name: Name.t;
    namespace_: NamespaceQName.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; namespace_} = 
    let fields = [
      ("name", Name.to_json name);
    ] in
    let fields =
      match namespace_ with
      | None -> fields
      | Some namespace_ -> ("namespace_", NamespaceQName.to_json namespace_) :: fields in
    JSON_Object fields

end

and NamespaceDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: NamespaceQName.t;
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
    name: NamespaceQName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", NamespaceQName.to_json name);
    ] in
    JSON_Object fields

end

and TypedefDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: TypedefDeclaration.t;
    is_transparent: bool;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    module_: ModuleMembership.t option;
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
    declaration: TypedefDeclaration.t;
    is_transparent: bool;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    module_: ModuleMembership.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; is_transparent; attributes; type_params; module_} = 
    let fields = [
      ("declaration", TypedefDeclaration.to_json declaration);
      ("isTransparent", JSON_Bool is_transparent);
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
      ("typeParams", JSON_Array (List.map ~f:(fun x -> TypeParameter.to_json x) type_params));
    ] in
    let fields =
      match module_ with
      | None -> fields
      | Some module_ -> ("module_", ModuleMembership.to_json module_) :: fields in
    JSON_Object fields

end

and StringLiteral: sig
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

and MethodOccurrence: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    class_name: Name.t option;
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
    name: Name.t;
    class_name: Name.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; class_name} = 
    let fields = [
      ("name", Name.to_json name);
    ] in
    let fields =
      match class_name with
      | None -> fields
      | Some class_name -> ("className", Name.to_json class_name) :: fields in
    JSON_Object fields

end

and ClassConstDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    container: ContainerDeclaration.t;
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
    name: Name.t;
    container: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; container} = 
    let fields = [
      ("name", Name.to_json name);
      ("container", ContainerDeclaration.to_json container);
    ] in
    JSON_Object fields

end

and ModuleParent: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    decl: Declaration.t;
    module_: ModuleDeclaration.t;
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
    module_: ModuleDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {decl; module_} = 
    let fields = [
      ("decl", Declaration.to_json decl);
      ("module", ModuleDeclaration.to_json module_);
    ] in
    JSON_Object fields

end

and ThriftToHack: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    to_: Fbthrift.Declaration.t;
    from: Declaration.t;
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
    to_: Fbthrift.Declaration.t;
    from: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {to_; from} = 
    let fields = [
      ("to", Fbthrift.Declaration.to_json to_);
      ("from", Declaration.to_json from);
    ] in
    JSON_Object fields

end

and DeclarationSpan: sig
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

and UserAttribute: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    parameters: string list;
    qname: QName.t option;
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
    name: Name.t;
    parameters: string list;
    qname: QName.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; parameters; qname} = 
    let fields = [
      ("name", Name.to_json name);
      ("parameters", JSON_Array (List.map ~f:(fun x -> JSON_String x) parameters));
    ] in
    let fields =
      match qname with
      | None -> fields
      | Some qname -> ("qname", QName.to_json qname) :: fields in
    JSON_Object fields

end

and PropertyDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: PropertyDeclaration.t;
    visibility: Visibility.t;
    is_final: bool;
    is_abstract: bool;
    is_static: bool;
    attributes: UserAttribute.t list;
    type_info: TypeInfo.t option;
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
    declaration: PropertyDeclaration.t;
    visibility: Visibility.t;
    is_final: bool;
    is_abstract: bool;
    is_static: bool;
    attributes: UserAttribute.t list;
    type_info: TypeInfo.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; visibility; is_final; is_abstract; is_static; attributes; type_info} = 
    let fields = [
      ("declaration", PropertyDeclaration.to_json declaration);
      ("visibility", Visibility.to_json visibility);
      ("isFinal", JSON_Bool is_final);
      ("isAbstract", JSON_Bool is_abstract);
      ("isStatic", JSON_Bool is_static);
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
    ] in
    let fields =
      match type_info with
      | None -> fields
      | Some type_info -> ("typeInfo", TypeInfo.to_json type_info) :: fields in
    JSON_Object fields

end

and GlobalConstDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QName.t;
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
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QName.to_json name);
    ] in
    JSON_Object fields

end

and FileDeclarations: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: Src.File.t;
    declarations: Declaration.t list;
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
    file: Src.File.t;
    declarations: Declaration.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; declarations} = 
    let fields = [
      ("file", Src.File.to_json file);
      ("declarations", JSON_Array (List.map ~f:(fun x -> Declaration.to_json x) declarations));
    ] in
    JSON_Object fields

end

and NamespaceQName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    parent: NamespaceQName.t option;
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
    name: Name.t;
    parent: NamespaceQName.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; parent} = 
    let fields = [
      ("name", Name.to_json name);
    ] in
    let fields =
      match parent with
      | None -> fields
      | Some parent -> ("parent", NamespaceQName.to_json parent) :: fields in
    JSON_Object fields

end

and TargetUses: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: XRefTarget.t;
    file: Src.File.t;
    uses: Src.RelByteSpan.t list;
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
    target: XRefTarget.t;
    file: Src.File.t;
    uses: Src.RelByteSpan.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; file; uses} = 
    let fields = [
      ("target", XRefTarget.to_json target);
      ("file", Src.File.to_json file);
      ("uses", JSON_Array (List.map ~f:(fun x -> Src.RelByteSpan.to_json x) uses));
    ] in
    JSON_Object fields

end

and InheritedMembers: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    container: ContainerDeclaration.t;
    inherited_members: MemberCluster.t list;
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
    container: ContainerDeclaration.t;
    inherited_members: MemberCluster.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {container; inherited_members} = 
    let fields = [
      ("container", ContainerDeclaration.to_json container);
      ("inheritedMembers", JSON_Array (List.map ~f:(fun x -> MemberCluster.to_json x) inherited_members));
    ] in
    JSON_Object fields

end

and ContainerChild: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    container: ContainerDeclaration.t;
    child: ContainerDeclaration.t;
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
    container: ContainerDeclaration.t;
    child: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {container; child} = 
    let fields = [
      ("container", ContainerDeclaration.to_json container);
      ("child", ContainerDeclaration.to_json child);
    ] in
    JSON_Object fields

end

and DeclarationSource: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: Declaration.t;
    source: Declaration.t;
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
    source: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; source} = 
    let fields = [
      ("target", Declaration.to_json target);
      ("source", Declaration.to_json source);
    ] in
    JSON_Object fields

end

and MethodOverridden: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    base: MethodDeclaration.t;
    derived: MethodDeclaration.t;
    annotation: bool option;
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
    base: MethodDeclaration.t;
    derived: MethodDeclaration.t;
    annotation: bool option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {base; derived; annotation} = 
    let fields = [
      ("base", MethodDeclaration.to_json base);
      ("derived", MethodDeclaration.to_json derived);
    ] in
    let fields =
      match annotation with
      | None -> fields
      | Some annotation -> ("annotation", JSON_Bool annotation) :: fields in
    JSON_Object fields

end

and NameLowerCase: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name_lowercase: string;
    name: Name.t;
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
    name_lowercase: string;
    name: Name.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name_lowercase; name} = 
    let fields = [
      ("nameLowercase", JSON_String name_lowercase);
      ("name", Name.to_json name);
    ] in
    JSON_Object fields

end

and TypedefDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QName.t;
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
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QName.to_json name);
    ] in
    JSON_Object fields

end

and TypeConstDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    container: ContainerDeclaration.t;
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
    name: Name.t;
    container: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; container} = 
    let fields = [
      ("name", Name.to_json name);
      ("container", ContainerDeclaration.to_json container);
    ] in
    JSON_Object fields

end

and GlobalNamespaceAlias: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    from: Name.t;
    to_: NamespaceQName.t;
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
    from: Name.t;
    to_: NamespaceQName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {from; to_} = 
    let fields = [
      ("from", Name.to_json from);
      ("to", NamespaceQName.to_json to_);
    ] in
    JSON_Object fields

end

and AttributeToDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    attribute: UserAttribute.t;
    definition: Definition.t;
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
    attribute: UserAttribute.t;
    definition: Definition.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {attribute; definition} = 
    let fields = [
      ("attribute", UserAttribute.to_json attribute);
      ("definition", Definition.to_json definition);
    ] in
    JSON_Object fields

end

and TypeConstDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: TypeConstDeclaration.t;
    kind: TypeConstKind.t;
    attributes: UserAttribute.t list;
    type_info: TypeInfo.t option;
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
    declaration: TypeConstDeclaration.t;
    kind: TypeConstKind.t;
    attributes: UserAttribute.t list;
    type_info: TypeInfo.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; kind; attributes; type_info} = 
    let fields = [
      ("declaration", TypeConstDeclaration.to_json declaration);
      ("kind", TypeConstKind.to_json kind);
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
    ] in
    let fields =
      match type_info with
      | None -> fields
      | Some type_info -> ("typeInfo", TypeInfo.to_json type_info) :: fields in
    JSON_Object fields

end

and MemberCluster: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    members: Declaration.t list;
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
    members: Declaration.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {members} = 
    let fields = [
      ("members", JSON_Array (List.map ~f:(fun x -> Declaration.to_json x) members));
    ] in
    JSON_Object fields

end

and ModuleChild: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    module_: ModuleDeclaration.t;
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
    module_: ModuleDeclaration.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {module_; decl} = 
    let fields = [
      ("module", ModuleDeclaration.to_json module_);
      ("decl", Declaration.to_json decl);
    ] in
    JSON_Object fields

end

and FileCall: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: Src.File.t;
    callee_span: Src.ByteSpan.t;
    call_args: CallArgument.t list;
    callee_xref: XRefTarget.t option;
    dispatch_arg: CallArgument.t option;
    receiver_type: Declaration.t option;
    callee_xrefs: XRefTarget.t list;
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
    file: Src.File.t;
    callee_span: Src.ByteSpan.t;
    call_args: CallArgument.t list;
    callee_xref: XRefTarget.t option;
    dispatch_arg: CallArgument.t option;
    receiver_type: Declaration.t option;
    callee_xrefs: XRefTarget.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; callee_span; call_args; callee_xref; dispatch_arg; receiver_type; callee_xrefs} = 
    let fields = [
      ("file", Src.File.to_json file);
      ("callee_span", Src.ByteSpan.to_json callee_span);
      ("call_args", JSON_Array (List.map ~f:(fun x -> CallArgument.to_json x) call_args));
      ("callee_xrefs", JSON_Array (List.map ~f:(fun x -> XRefTarget.to_json x) callee_xrefs));
    ] in
    let fields =
      match callee_xref with
      | None -> fields
      | Some callee_xref -> ("callee_xref", XRefTarget.to_json callee_xref) :: fields in
    let fields =
      match dispatch_arg with
      | None -> fields
      | Some dispatch_arg -> ("dispatch_arg", CallArgument.to_json dispatch_arg) :: fields in
    let fields =
      match receiver_type with
      | None -> fields
      | Some receiver_type -> ("receiver_type", Declaration.to_json receiver_type) :: fields in
    JSON_Object fields

end

and PropertyDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    container: ContainerDeclaration.t;
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
    name: Name.t;
    container: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; container} = 
    let fields = [
      ("name", Name.to_json name);
      ("container", ContainerDeclaration.to_json container);
    ] in
    JSON_Object fields

end

and EnumDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: EnumDeclaration.t;
    enum_base: Type.t;
    enum_base_type_info: TypeInfo.t option;
    enum_constraint_type_info: TypeInfo.t option;
    enumerators: Enumerator.t list;
    attributes: UserAttribute.t list;
    includes: EnumDeclaration.t list;
    is_enum_class: bool;
    module_: ModuleMembership.t option;
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
    declaration: EnumDeclaration.t;
    enum_base: Type.t;
    enum_base_type_info: TypeInfo.t option;
    enum_constraint_type_info: TypeInfo.t option;
    enumerators: Enumerator.t list;
    attributes: UserAttribute.t list;
    includes: EnumDeclaration.t list;
    is_enum_class: bool;
    module_: ModuleMembership.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; enum_base; enum_base_type_info; enum_constraint_type_info; enumerators; attributes; includes; is_enum_class; module_} = 
    let fields = [
      ("declaration", EnumDeclaration.to_json declaration);
      ("enumBase", Type.to_json enum_base);
      ("enumerators", JSON_Array (List.map ~f:(fun x -> Enumerator.to_json x) enumerators));
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
      ("includes", JSON_Array (List.map ~f:(fun x -> EnumDeclaration.to_json x) includes));
      ("isEnumClass", JSON_Bool is_enum_class);
    ] in
    let fields =
      match enum_base_type_info with
      | None -> fields
      | Some enum_base_type_info -> ("enumBaseTypeInfo", TypeInfo.to_json enum_base_type_info) :: fields in
    let fields =
      match enum_constraint_type_info with
      | None -> fields
      | Some enum_constraint_type_info -> ("enumConstraintTypeInfo", TypeInfo.to_json enum_constraint_type_info) :: fields in
    let fields =
      match module_ with
      | None -> fields
      | Some module_ -> ("module_", ModuleMembership.to_json module_) :: fields in
    JSON_Object fields

end

and EnumDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QName.t;
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
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QName.to_json name);
    ] in
    JSON_Object fields

end

and AttributeToDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    attribute: UserAttribute.t;
    declaration: Declaration.t;
    file: Src.File.t;
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
    attribute: UserAttribute.t;
    declaration: Declaration.t;
    file: Src.File.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {attribute; declaration; file} = 
    let fields = [
      ("attribute", UserAttribute.to_json attribute);
      ("declaration", Declaration.to_json declaration);
      ("file", Src.File.to_json file);
    ] in
    JSON_Object fields

end

and AttributeHasParameter: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    parameter: string;
    attribute: UserAttribute.t;
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
    name: Name.t;
    parameter: string;
    attribute: UserAttribute.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; parameter; attribute} = 
    let fields = [
      ("name", Name.to_json name);
      ("parameter", JSON_String parameter);
      ("attribute", UserAttribute.to_json attribute);
    ] in
    JSON_Object fields

end

and TypeInfo: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    display_type: Type.t;
    xrefs: XRef.t list;
    hint: Hint.t option;
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
    display_type: Type.t;
    xrefs: XRef.t list;
    hint: Hint.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {display_type; xrefs; hint} = 
    let fields = [
      ("displayType", Type.to_json display_type);
      ("xrefs", JSON_Array (List.map ~f:(fun x -> XRef.to_json x) xrefs));
    ] in
    let fields =
      match hint with
      | None -> fields
      | Some hint -> ("hint", Hint.to_json hint) :: fields in
    JSON_Object fields

end

and MethodOverrides: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    derived: MethodDeclaration.t;
    base: MethodDeclaration.t;
    annotation: bool option;
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
    derived: MethodDeclaration.t;
    base: MethodDeclaration.t;
    annotation: bool option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {derived; base; annotation} = 
    let fields = [
      ("derived", MethodDeclaration.to_json derived);
      ("base", MethodDeclaration.to_json base);
    ] in
    let fields =
      match annotation with
      | None -> fields
      | Some annotation -> ("annotation", JSON_Bool annotation) :: fields in
    JSON_Object fields

end

and ModuleDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: ModuleDeclaration.t;
    attributes: UserAttribute.t list;
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
    declaration: ModuleDeclaration.t;
    attributes: UserAttribute.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; attributes} = 
    let fields = [
      ("declaration", ModuleDeclaration.to_json declaration);
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
    ] in
    JSON_Object fields

end

and Context_: sig
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

and TargetUsesAbs: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    target: XRefTarget.t;
    file: Src.File.t;
    uses: Src.ByteSpan.t list;
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
    target: XRefTarget.t;
    file: Src.File.t;
    uses: Src.ByteSpan.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; file; uses} = 
    let fields = [
      ("target", XRefTarget.to_json target);
      ("file", Src.File.to_json file);
      ("uses", JSON_Array (List.map ~f:(fun x -> Src.ByteSpan.to_json x) uses));
    ] in
    JSON_Object fields

end

and InterfaceDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QName.t;
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
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QName.to_json name);
    ] in
    JSON_Object fields

end

and FileXRefs: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    file: Src.File.t;
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
    file: Src.File.t;
    xrefs: XRef.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; xrefs} = 
    let fields = [
      ("file", Src.File.to_json file);
      ("xrefs", JSON_Array (List.map ~f:(fun x -> XRef.to_json x) xrefs));
    ] in
    JSON_Object fields

end

and FunctionDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: FunctionDeclaration.t;
    signature: Signature.t;
    is_async: bool;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    module_: ModuleMembership.t option;
    readonly_ret: ReadonlyKind.t option;
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
    declaration: FunctionDeclaration.t;
    signature: Signature.t;
    is_async: bool;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    module_: ModuleMembership.t option;
    readonly_ret: ReadonlyKind.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; signature; is_async; attributes; type_params; module_; readonly_ret} = 
    let fields = [
      ("declaration", FunctionDeclaration.to_json declaration);
      ("signature", Signature.to_json signature);
      ("isAsync", JSON_Bool is_async);
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
      ("typeParams", JSON_Array (List.map ~f:(fun x -> TypeParameter.to_json x) type_params));
    ] in
    let fields =
      match module_ with
      | None -> fields
      | Some module_ -> ("module_", ModuleMembership.to_json module_) :: fields in
    let fields =
      match readonly_ret with
      | None -> fields
      | Some readonly_ret -> ("readonlyRet", ReadonlyKind.to_json readonly_ret) :: fields in
    JSON_Object fields

end

and FunctionDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: QName.t;
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
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", QName.to_json name);
    ] in
    JSON_Object fields

end

and Signature: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    parameters: Parameter.t list;
    contexts: Context_.t list option;
    returns_type_info: TypeInfo.t option;
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
    parameters: Parameter.t list;
    contexts: Context_.t list option;
    returns_type_info: TypeInfo.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {parameters; contexts; returns_type_info} = 
    let fields = [
      ("parameters", JSON_Array (List.map ~f:(fun x -> Parameter.to_json x) parameters));
    ] in
    let fields =
      match contexts with
      | None -> fields
      | Some contexts -> ("contexts", JSON_Array (List.map ~f:(fun x -> Context_.to_json x) contexts)) :: fields in
    let fields =
      match returns_type_info with
      | None -> fields
      | Some returns_type_info -> ("returnsTypeInfo", TypeInfo.to_json returns_type_info) :: fields in
    JSON_Object fields

end

and Enumerator: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
    enumeration: EnumDeclaration.t;
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
    name: Name.t;
    enumeration: EnumDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; enumeration} = 
    let fields = [
      ("name", Name.to_json name);
      ("enumeration", EnumDeclaration.to_json enumeration);
    ] in
    JSON_Object fields

end

and HackToThrift: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    from: Declaration.t;
    to_: Fbthrift.Declaration.t;
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
    from: Declaration.t;
    to_: Fbthrift.Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {from; to_} = 
    let fields = [
      ("from", Declaration.to_json from);
      ("to", Fbthrift.Declaration.to_json to_);
    ] in
    JSON_Object fields

end

and InterfaceDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    declaration: InterfaceDeclaration.t;
    members: Declaration.t list;
    extends_: InterfaceDeclaration.t list;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    require_extends: ClassDeclaration.t list;
    module_: ModuleMembership.t option;
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
    declaration: InterfaceDeclaration.t;
    members: Declaration.t list;
    extends_: InterfaceDeclaration.t list;
    attributes: UserAttribute.t list;
    type_params: TypeParameter.t list;
    require_extends: ClassDeclaration.t list;
    module_: ModuleMembership.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; members; extends_; attributes; type_params; require_extends; module_} = 
    let fields = [
      ("declaration", InterfaceDeclaration.to_json declaration);
      ("members", JSON_Array (List.map ~f:(fun x -> Declaration.to_json x) members));
      ("extends_", JSON_Array (List.map ~f:(fun x -> InterfaceDeclaration.to_json x) extends_));
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
      ("typeParams", JSON_Array (List.map ~f:(fun x -> TypeParameter.to_json x) type_params));
      ("requireExtends", JSON_Array (List.map ~f:(fun x -> ClassDeclaration.to_json x) require_extends));
    ] in
    let fields =
      match module_ with
      | None -> fields
      | Some module_ -> ("module_", ModuleMembership.to_json module_) :: fields in
    JSON_Object fields

end

and Name: sig
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

and DeclarationTarget: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    source: Declaration.t;
    target: Declaration.t;
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
    source: Declaration.t;
    target: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {source; target} = 
    let fields = [
      ("source", Declaration.to_json source);
      ("target", Declaration.to_json target);
    ] in
    JSON_Object fields

end

and NamespaceMember: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    namespace_: NamespaceQName.t;
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
    namespace_: NamespaceQName.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {namespace_; decl} = 
    let fields = [
      ("namespace_", NamespaceQName.to_json namespace_);
      ("decl", Declaration.to_json decl);
    ] in
    JSON_Object fields

end

and ContainerParent: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    container: ContainerDeclaration.t;
    parent: ContainerDeclaration.t;
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
    container: ContainerDeclaration.t;
    parent: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {container; parent} = 
    let fields = [
      ("container", ContainerDeclaration.to_json container);
      ("parent", ContainerDeclaration.to_json parent);
    ] in
    JSON_Object fields

end

and ContainerDeclarationQName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = ContainerDeclaration.t
  [@@deriving ord]
  and value = QName.t
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json
  val to_json_value: value -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = ContainerDeclaration.t
  [@@deriving ord]
  and value = QName.t
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = ContainerDeclaration.to_json x
  and to_json_value x = QName.to_json x
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

and ModuleDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    name: Name.t;
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
    name: Name.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    let fields = [
      ("name", Name.to_json name);
    ] in
    JSON_Object fields

end

and DeclarationNamespace: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = {
    decl: Declaration.t;
    namespace_: NamespaceQName.t;
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
    namespace_: NamespaceQName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {decl; namespace_} = 
    let fields = [
      ("decl", Declaration.to_json decl);
      ("namespace_", NamespaceQName.to_json namespace_);
    ] in
    JSON_Object fields

end

and Hint: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = 
     | Apply of apply
     | Option of Hint.t
     | Like of Hint.t
     | Tuple of tuple
     | Class_args of Hint.t
     | Shape of shape
     | Soft of Hint.t
     | Intersection of Hint.t list
     | Union_ of Hint.t list
     | Vect_or_dict of vect_or_dict
     | Prim of Type.t
     | Var_ of string
     | Fun_context of string
     | Mixed of unit
     | Wildcard of unit
     | Nonnull of unit
     | This_ of unit
     | Dynamic of unit
     | Nothing of unit
     | Other of Type.t
  [@@deriving ord]
  and vect_or_dict = {
    maybe_key: Hint.t option;
    value_: Hint.t;
  }
  [@@deriving ord]
  and shape = {
    open_: bool;
    map_: ShapeKV.t list;
  }
  [@@deriving ord]
  and tuple = {
    req: Hint.t list;
    opt: Hint.t list;
    variadic: Hint.t option;
  }
  [@@deriving ord]
  and apply = {
    class_name: QName.t;
    values: Hint.t list;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json
  val vect_or_dict_to_json: vect_or_dict -> json
  val shape_to_json: shape -> json
  val tuple_to_json: tuple -> json
  val apply_to_json: apply -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key = 
     | Apply of apply
     | Option of Hint.t
     | Like of Hint.t
     | Tuple of tuple
     | Class_args of Hint.t
     | Shape of shape
     | Soft of Hint.t
     | Intersection of Hint.t list
     | Union_ of Hint.t list
     | Vect_or_dict of vect_or_dict
     | Prim of Type.t
     | Var_ of string
     | Fun_context of string
     | Mixed of unit
     | Wildcard of unit
     | Nonnull of unit
     | This_ of unit
     | Dynamic of unit
     | Nothing of unit
     | Other of Type.t
  [@@deriving ord]
  and vect_or_dict = {
    maybe_key: Hint.t option;
    value_: Hint.t;
  }
  [@@deriving ord]
  and shape = {
    open_: bool;
    map_: ShapeKV.t list;
  }
  [@@deriving ord]
  and tuple = {
    req: Hint.t list;
    opt: Hint.t list;
    variadic: Hint.t option;
  }
  [@@deriving ord]
  and apply = {
    class_name: QName.t;
    values: Hint.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key  = function
     | Apply apply -> JSON_Object [("apply", apply_to_json apply)]
     | Option option -> JSON_Object [("option", Hint.to_json option)]
     | Like like -> JSON_Object [("like", Hint.to_json like)]
     | Tuple tuple -> JSON_Object [("tuple", tuple_to_json tuple)]
     | Class_args class_args -> JSON_Object [("class_args", Hint.to_json class_args)]
     | Shape shape -> JSON_Object [("shape", shape_to_json shape)]
     | Soft soft -> JSON_Object [("soft", Hint.to_json soft)]
     | Intersection intersection -> JSON_Object [("intersection", JSON_Array (List.map ~f:(fun x -> Hint.to_json x) intersection))]
     | Union_ union_ -> JSON_Object [("union_", JSON_Array (List.map ~f:(fun x -> Hint.to_json x) union_))]
     | Vect_or_dict vect_or_dict -> JSON_Object [("vect_or_dict", vect_or_dict_to_json vect_or_dict)]
     | Prim prim -> JSON_Object [("prim", Type.to_json prim)]
     | Var_ var_ -> JSON_Object [("var_", JSON_String var_)]
     | Fun_context fun_context -> JSON_Object [("fun_context", JSON_String fun_context)]
     | Mixed mixed -> JSON_Object [("mixed", (ignore mixed; JSON_Object []))]
     | Wildcard wildcard -> JSON_Object [("wildcard", (ignore wildcard; JSON_Object []))]
     | Nonnull nonnull -> JSON_Object [("nonnull", (ignore nonnull; JSON_Object []))]
     | This_ this_ -> JSON_Object [("this_", (ignore this_; JSON_Object []))]
     | Dynamic dynamic -> JSON_Object [("dynamic", (ignore dynamic; JSON_Object []))]
     | Nothing nothing -> JSON_Object [("nothing", (ignore nothing; JSON_Object []))]
     | Other other -> JSON_Object [("other", Type.to_json other)]

  and vect_or_dict_to_json {maybe_key; value_} = 
    let fields = [
      ("value_", Hint.to_json value_);
    ] in
    let fields =
      match maybe_key with
      | None -> fields
      | Some maybe_key -> ("maybe_key", Hint.to_json maybe_key) :: fields in
    JSON_Object fields

  and shape_to_json {open_; map_} = 
    let fields = [
      ("open_", JSON_Bool open_);
      ("map_", JSON_Array (List.map ~f:(fun x -> ShapeKV.to_json x) map_));
    ] in
    JSON_Object fields

  and tuple_to_json {req; opt; variadic} = 
    let fields = [
      ("req", JSON_Array (List.map ~f:(fun x -> Hint.to_json x) req));
      ("opt", JSON_Array (List.map ~f:(fun x -> Hint.to_json x) opt));
    ] in
    let fields =
      match variadic with
      | None -> fields
      | Some variadic -> ("variadic", Hint.to_json variadic) :: fields in
    JSON_Object fields

  and apply_to_json {class_name; values} = 
    let fields = [
      ("class_name", QName.to_json class_name);
      ("values", JSON_Array (List.map ~f:(fun x -> Hint.to_json x) values));
    ] in
    JSON_Object fields

end


and Constraint: sig
  type t = {
    constraint_kind: ConstraintKind.t;
    type_: Type.t;
    type_info: TypeInfo.t option;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    constraint_kind: ConstraintKind.t;
    type_: Type.t;
    type_info: TypeInfo.t option;
  }
  [@@deriving ord]

  let rec to_json {constraint_kind; type_; type_info} = 
    let fields = [
      ("constraintKind", ConstraintKind.to_json constraint_kind);
      ("type", Type.to_json type_);
    ] in
    let fields =
      match type_info with
      | None -> fields
      | Some type_info -> ("typeInfo", TypeInfo.to_json type_info) :: fields in
    JSON_Object fields

end

and XRef: sig
  type t = {
    target: XRefTarget.t;
    ranges: Src.RelByteSpan.t list;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    target: XRefTarget.t;
    ranges: Src.RelByteSpan.t list;
  }
  [@@deriving ord]

  let rec to_json {target; ranges} = 
    let fields = [
      ("target", XRefTarget.to_json target);
      ("ranges", JSON_Array (List.map ~f:(fun x -> Src.RelByteSpan.to_json x) ranges));
    ] in
    JSON_Object fields

end

and FieldClassConst: sig
  type t = {
    container: QName.t;
    name: Name.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    container: QName.t;
    name: Name.t;
  }
  [@@deriving ord]

  let rec to_json {container; name} = 
    let fields = [
      ("container", QName.to_json container);
      ("name", Name.to_json name);
    ] in
    JSON_Object fields

end

and XRefTarget: sig
  type t = 
     | Declaration of Declaration.t
     | Occurrence of Occurrence.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Declaration of Declaration.t
     | Occurrence of Occurrence.t
  [@@deriving ord]

  let rec to_json  = function
     | Declaration declaration -> JSON_Object [("declaration", Declaration.to_json declaration)]
     | Occurrence occurrence -> JSON_Object [("occurrence", Occurrence.to_json occurrence)]

end

and Declaration: sig
  type t = 
     | ClassConst of ClassConstDeclaration.t
     | Container of ContainerDeclaration.t
     | Enumerator of Enumerator.t
     | Function_ of FunctionDeclaration.t
     | GlobalConst of GlobalConstDeclaration.t
     | Namespace_ of NamespaceDeclaration.t
     | Method of MethodDeclaration.t
     | Property_ of PropertyDeclaration.t
     | TypeConst of TypeConstDeclaration.t
     | Typedef_ of TypedefDeclaration.t
     | Module of ModuleDeclaration.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | ClassConst of ClassConstDeclaration.t
     | Container of ContainerDeclaration.t
     | Enumerator of Enumerator.t
     | Function_ of FunctionDeclaration.t
     | GlobalConst of GlobalConstDeclaration.t
     | Namespace_ of NamespaceDeclaration.t
     | Method of MethodDeclaration.t
     | Property_ of PropertyDeclaration.t
     | TypeConst of TypeConstDeclaration.t
     | Typedef_ of TypedefDeclaration.t
     | Module of ModuleDeclaration.t
  [@@deriving ord]

  let rec to_json  = function
     | ClassConst class_const -> JSON_Object [("classConst", ClassConstDeclaration.to_json class_const)]
     | Container container -> JSON_Object [("container", ContainerDeclaration.to_json container)]
     | Enumerator enumerator -> JSON_Object [("enumerator", Enumerator.to_json enumerator)]
     | Function_ function_ -> JSON_Object [("function_", FunctionDeclaration.to_json function_)]
     | GlobalConst global_const -> JSON_Object [("globalConst", GlobalConstDeclaration.to_json global_const)]
     | Namespace_ namespace_ -> JSON_Object [("namespace_", NamespaceDeclaration.to_json namespace_)]
     | Method method_ -> JSON_Object [("method", MethodDeclaration.to_json method_)]
     | Property_ property_ -> JSON_Object [("property_", PropertyDeclaration.to_json property_)]
     | TypeConst type_const -> JSON_Object [("typeConst", TypeConstDeclaration.to_json type_const)]
     | Typedef_ typedef_ -> JSON_Object [("typedef_", TypedefDeclaration.to_json typedef_)]
     | Module module_ -> JSON_Object [("module", ModuleDeclaration.to_json module_)]

end

and Argument: sig
  type t = 
     | Lit of StringLiteral.t
     | Xref of XRefTarget.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Lit of StringLiteral.t
     | Xref of XRefTarget.t
  [@@deriving ord]

  let rec to_json  = function
     | Lit lit -> JSON_Object [("lit", StringLiteral.to_json lit)]
     | Xref xref -> JSON_Object [("xref", XRefTarget.to_json xref)]

end

and Definition: sig
  type t = 
     | Class_ of ClassDefinition.t
     | ClassConst of ClassConstDefinition.t
     | Enum_ of EnumDefinition.t
     | Function_ of FunctionDefinition.t
     | GlobalConst of GlobalConstDefinition.t
     | Interface_ of InterfaceDefinition.t
     | Trait of TraitDefinition.t
     | Method of MethodDefinition.t
     | Property_ of PropertyDefinition.t
     | TypeConst of TypeConstDefinition.t
     | Typedef_ of TypedefDefinition.t
     | Module of ModuleDefinition.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Class_ of ClassDefinition.t
     | ClassConst of ClassConstDefinition.t
     | Enum_ of EnumDefinition.t
     | Function_ of FunctionDefinition.t
     | GlobalConst of GlobalConstDefinition.t
     | Interface_ of InterfaceDefinition.t
     | Trait of TraitDefinition.t
     | Method of MethodDefinition.t
     | Property_ of PropertyDefinition.t
     | TypeConst of TypeConstDefinition.t
     | Typedef_ of TypedefDefinition.t
     | Module of ModuleDefinition.t
  [@@deriving ord]

  let rec to_json  = function
     | Class_ class_ -> JSON_Object [("class_", ClassDefinition.to_json class_)]
     | ClassConst class_const -> JSON_Object [("classConst", ClassConstDefinition.to_json class_const)]
     | Enum_ enum_ -> JSON_Object [("enum_", EnumDefinition.to_json enum_)]
     | Function_ function_ -> JSON_Object [("function_", FunctionDefinition.to_json function_)]
     | GlobalConst global_const -> JSON_Object [("globalConst", GlobalConstDefinition.to_json global_const)]
     | Interface_ interface_ -> JSON_Object [("interface_", InterfaceDefinition.to_json interface_)]
     | Trait trait -> JSON_Object [("trait", TraitDefinition.to_json trait)]
     | Method method_ -> JSON_Object [("method", MethodDefinition.to_json method_)]
     | Property_ property_ -> JSON_Object [("property_", PropertyDefinition.to_json property_)]
     | TypeConst type_const -> JSON_Object [("typeConst", TypeConstDefinition.to_json type_const)]
     | Typedef_ typedef_ -> JSON_Object [("typedef_", TypedefDefinition.to_json typedef_)]
     | Module module_ -> JSON_Object [("module", ModuleDefinition.to_json module_)]

end

and Visibility: sig
  type t = 
    | Private
    | Protected
    | Public
    | Internal

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Private
    | Protected
    | Public
    | Internal

  [@@deriving ord]

  let rec to_json  = function
     | Private -> JSON_Number (string_of_int 0)
     | Protected -> JSON_Number (string_of_int 1)
     | Public -> JSON_Number (string_of_int 2)
     | Internal -> JSON_Number (string_of_int 3)

end

and Occurrence: sig
  type t = 
     | Method of MethodOccurrence.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Method of MethodOccurrence.t
  [@@deriving ord]

  let rec to_json  = function
     | Method method_ -> JSON_Object [("method", MethodOccurrence.to_json method_)]

end

and ReadonlyKind: sig
  type t = 
    | Readonly

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Readonly

  [@@deriving ord]

  let rec to_json  = function
     | Readonly -> JSON_Number (string_of_int 0)

end

and ContainerDeclaration: sig
  type t = 
     | Class_ of ClassDeclaration.t
     | Enum_ of EnumDeclaration.t
     | Interface_ of InterfaceDeclaration.t
     | Trait of TraitDeclaration.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
     | Class_ of ClassDeclaration.t
     | Enum_ of EnumDeclaration.t
     | Interface_ of InterfaceDeclaration.t
     | Trait of TraitDeclaration.t
  [@@deriving ord]

  let rec to_json  = function
     | Class_ class_ -> JSON_Object [("class_", ClassDeclaration.to_json class_)]
     | Enum_ enum_ -> JSON_Object [("enum_", EnumDeclaration.to_json enum_)]
     | Interface_ interface_ -> JSON_Object [("interface_", InterfaceDeclaration.to_json interface_)]
     | Trait trait -> JSON_Object [("trait", TraitDeclaration.to_json trait)]

end

and TypeConstKind: sig
  type t = 
    | Abstract
    | Concrete
    | PartiallyAbstract

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Abstract
    | Concrete
    | PartiallyAbstract

  [@@deriving ord]

  let rec to_json  = function
     | Abstract -> JSON_Number (string_of_int 0)
     | Concrete -> JSON_Number (string_of_int 1)
     | PartiallyAbstract -> JSON_Number (string_of_int 2)

end

and ReifyKind: sig
  type t = 
    | Erased
    | Reified
    | SoftReified

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Erased
    | Reified
    | SoftReified

  [@@deriving ord]

  let rec to_json  = function
     | Erased -> JSON_Number (string_of_int 0)
     | Reified -> JSON_Number (string_of_int 1)
     | SoftReified -> JSON_Number (string_of_int 2)

end

and Parameter: sig
  type t = {
    name: Name.t;
    is_inout: bool;
    is_variadic: bool;
    default_value: string option;
    attributes: UserAttribute.t list;
    type_info: TypeInfo.t option;
    readonly: ReadonlyKind.t option;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    name: Name.t;
    is_inout: bool;
    is_variadic: bool;
    default_value: string option;
    attributes: UserAttribute.t list;
    type_info: TypeInfo.t option;
    readonly: ReadonlyKind.t option;
  }
  [@@deriving ord]

  let rec to_json {name; is_inout; is_variadic; default_value; attributes; type_info; readonly} = 
    let fields = [
      ("name", Name.to_json name);
      ("isInout", JSON_Bool is_inout);
      ("isVariadic", JSON_Bool is_variadic);
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
    ] in
    let fields =
      match default_value with
      | None -> fields
      | Some default_value -> ("defaultValue", JSON_String default_value) :: fields in
    let fields =
      match type_info with
      | None -> fields
      | Some type_info -> ("typeInfo", TypeInfo.to_json type_info) :: fields in
    let fields =
      match readonly with
      | None -> fields
      | Some readonly -> ("readonly", ReadonlyKind.to_json readonly) :: fields in
    JSON_Object fields

end

and ShapeKV: sig
  type t = {
    key: key;
    value: Hint.t;
    opt: bool;
  }
  [@@deriving ord]
  and key = 
     | Sf_regex_group of string
     | Sf_lit_string of string
     | Sf_class_const of FieldClassConst.t
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    key: key;
    value: Hint.t;
    opt: bool;
  }
  [@@deriving ord]
  and key = 
     | Sf_regex_group of string
     | Sf_lit_string of string
     | Sf_class_const of FieldClassConst.t
  [@@deriving ord]

  let rec to_json {key; value; opt} = 
    let fields = [
      ("key", key_to_json key);
      ("value", Hint.to_json value);
      ("opt", JSON_Bool opt);
    ] in
    JSON_Object fields

  and key_to_json  = function
     | Sf_regex_group sf_regex_group -> JSON_Object [("sf_regex_group", JSON_String sf_regex_group)]
     | Sf_lit_string sf_lit_string -> JSON_Object [("sf_lit_string", JSON_String sf_lit_string)]
     | Sf_class_const sf_class_const -> JSON_Object [("sf_class_const", FieldClassConst.to_json sf_class_const)]

end

and Variance: sig
  type t = 
    | Contravariant
    | Covariant
    | Invariant

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | Contravariant
    | Covariant
    | Invariant

  [@@deriving ord]

  let rec to_json  = function
     | Contravariant -> JSON_Number (string_of_int 0)
     | Covariant -> JSON_Number (string_of_int 1)
     | Invariant -> JSON_Number (string_of_int 2)

end

and ConstraintKind: sig
  type t = 
    | As
    | Equal
    | Super

  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = 
    | As
    | Equal
    | Super

  [@@deriving ord]

  let rec to_json  = function
     | As -> JSON_Number (string_of_int 0)
     | Equal -> JSON_Number (string_of_int 1)
     | Super -> JSON_Number (string_of_int 2)

end

and TypeParameter: sig
  type t = {
    name: Name.t;
    variance: Variance.t;
    reify_kind: ReifyKind.t;
    constraints: Constraint.t list;
    attributes: UserAttribute.t list;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    name: Name.t;
    variance: Variance.t;
    reify_kind: ReifyKind.t;
    constraints: Constraint.t list;
    attributes: UserAttribute.t list;
  }
  [@@deriving ord]

  let rec to_json {name; variance; reify_kind; constraints; attributes} = 
    let fields = [
      ("name", Name.to_json name);
      ("variance", Variance.to_json variance);
      ("reifyKind", ReifyKind.to_json reify_kind);
      ("constraints", JSON_Array (List.map ~f:(fun x -> Constraint.to_json x) constraints));
      ("attributes", JSON_Array (List.map ~f:(fun x -> UserAttribute.to_json x) attributes));
    ] in
    JSON_Object fields

end

and ModuleMembership: sig
  type t = {
    declaration: ModuleDeclaration.t;
    internal: bool;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    declaration: ModuleDeclaration.t;
    internal: bool;
  }
  [@@deriving ord]

  let rec to_json {declaration; internal} = 
    let fields = [
      ("declaration", ModuleDeclaration.to_json declaration);
      ("internal", JSON_Bool internal);
    ] in
    JSON_Object fields

end

and CallArgument: sig
  type t = {
    span: Src.RelByteSpan.t;
    argument: Argument.t option;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    span: Src.RelByteSpan.t;
    argument: Argument.t option;
  }
  [@@deriving ord]

  let rec to_json {span; argument} = 
    let fields = [
      ("span", Src.RelByteSpan.to_json span);
    ] in
    let fields =
      match argument with
      | None -> fields
      | Some argument -> ("argument", Argument.to_json argument) :: fields in
    JSON_Object fields

end


