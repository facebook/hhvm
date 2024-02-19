(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *)

(* @generated
   regenerate: buck2 run fbcode//glean/schema/gen:gen-schema  -- --ocaml fbcode/hphp/hack/src/typing/write_symbol_info/schema --dir DEST_DIR *)

open Hh_json


module rec MethodDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
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
    JSON_Object ([
      ("declaration", Some (MethodDeclaration.to_json declaration));
      ("signature", Some (Signature.to_json signature));
      ("visibility", Some (Visibility.to_json visibility));
      ("isAbstract", Some (JSON_Bool is_abstract));
      ("isAsync", Some (JSON_Bool is_async));
      ("isFinal", Some (JSON_Bool is_final));
      ("isStatic", Some (JSON_Bool is_static));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
      ("typeParams", Some (JSON_Array (List.map (fun x -> TypeParameter.to_json x) type_params)));
      ("isReadonlyThis", Option.map (fun x -> JSON_Bool x) is_readonly_this);
      ("readonlyRet", Option.map (fun x -> ReadonlyKind.to_json x) readonly_ret);
    ] |> List.filter_map Util.rem_opt)

end

and TraitDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (QName.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and FunctionDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (QName.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and TypedefDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
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
    JSON_Object ([
      ("declaration", Some (TypedefDeclaration.to_json declaration));
      ("isTransparent", Some (JSON_Bool is_transparent));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
      ("typeParams", Some (JSON_Array (List.map (fun x -> TypeParameter.to_json x) type_params)));
      ("module_", Option.map (fun x -> ModuleMembership.to_json x) module_);
    ] |> List.filter_map Util.rem_opt)

end

and AttributeToDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    attribute: UserAttribute.t;
    definition: Definition.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {attribute; definition} = 
    JSON_Object ([
      ("attribute", Some (UserAttribute.to_json attribute));
      ("definition", Some (Definition.to_json definition));
    ] |> List.filter_map Util.rem_opt)

end

and NamespaceMember: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    namespace_: NamespaceQName.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {namespace_; decl} = 
    JSON_Object ([
      ("namespace_", Some (NamespaceQName.to_json namespace_));
      ("decl", Some (Declaration.to_json decl));
    ] |> List.filter_map Util.rem_opt)

end

and GlobalConstDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    declaration: GlobalConstDeclaration.t;
    type_: Type.t option;
    value: string;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    declaration: GlobalConstDeclaration.t;
    type_: Type.t option;
    value: string;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; type_; value} = 
    JSON_Object ([
      ("declaration", Some (GlobalConstDeclaration.to_json declaration));
      ("type", Option.map (fun x -> Type.to_json x) type_);
      ("value", Some (JSON_String value));
    ] |> List.filter_map Util.rem_opt)

end

and DeclarationNamespace: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    decl: Declaration.t;
    namespace_: NamespaceQName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {decl; namespace_} = 
    JSON_Object ([
      ("decl", Some (Declaration.to_json decl));
      ("namespace_", Some (NamespaceQName.to_json namespace_));
    ] |> List.filter_map Util.rem_opt)

end

and ContainerParent: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    container: ContainerDeclaration.t;
    parent: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {container; parent} = 
    JSON_Object ([
      ("container", Some (ContainerDeclaration.to_json container));
      ("parent", Some (ContainerDeclaration.to_json parent));
    ] |> List.filter_map Util.rem_opt)

end

and InterfaceDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
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
    JSON_Object ([
      ("declaration", Some (InterfaceDeclaration.to_json declaration));
      ("members", Some (JSON_Array (List.map (fun x -> Declaration.to_json x) members)));
      ("extends_", Some (JSON_Array (List.map (fun x -> InterfaceDeclaration.to_json x) extends_)));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
      ("typeParams", Some (JSON_Array (List.map (fun x -> TypeParameter.to_json x) type_params)));
      ("requireExtends", Some (JSON_Array (List.map (fun x -> ClassDeclaration.to_json x) require_extends)));
      ("module_", Option.map (fun x -> ModuleMembership.to_json x) module_);
    ] |> List.filter_map Util.rem_opt)

end

and ModuleChild: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    module_: ModuleDeclaration.t;
    decl: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {module_; decl} = 
    JSON_Object ([
      ("module", Some (ModuleDeclaration.to_json module_));
      ("decl", Some (Declaration.to_json decl));
    ] |> List.filter_map Util.rem_opt)

end

and Context_: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key str = JSON_String str
end

and ContainerDeclarationQName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= ContainerDeclaration.t
  [@@deriving ord]
  and value= QName.t
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json
  val to_json_value: value -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= ContainerDeclaration.t
  [@@deriving ord]
  and value= QName.t
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = ContainerDeclaration.to_json x
  and to_json_value x = QName.to_json x
end

and TargetUses: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    target: XRefTarget.t;
    file: Src.File.t;
    uses: Src.RelByteSpan.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; file; uses} = 
    JSON_Object ([
      ("target", Some (XRefTarget.to_json target));
      ("file", Some (Src.File.to_json file));
      ("uses", Some (JSON_Array (List.map (fun x -> Src.RelByteSpan.to_json x) uses)));
    ] |> List.filter_map Util.rem_opt)

end

and TargetUsesAbs: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    target: XRefTarget.t;
    file: Src.File.t;
    uses: Src.ByteSpan.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; file; uses} = 
    JSON_Object ([
      ("target", Some (XRefTarget.to_json target));
      ("file", Some (Src.File.to_json file));
      ("uses", Some (JSON_Array (List.map (fun x -> Src.ByteSpan.to_json x) uses)));
    ] |> List.filter_map Util.rem_opt)

end

and MethodOverridden: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    base: MethodDeclaration.t;
    derived: MethodDeclaration.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    base: MethodDeclaration.t;
    derived: MethodDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {base; derived} = 
    JSON_Object ([
      ("base", Some (MethodDeclaration.to_json base));
      ("derived", Some (MethodDeclaration.to_json derived));
    ] |> List.filter_map Util.rem_opt)

end

and ClassDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (QName.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and IndexerInputsHash: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]
  and value= string list
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json
  val to_json_value: value -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]
  and value= string list
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key str = JSON_String str
  and to_json_value v = JSON_String (List.map Base64.encode_string v |> String.concat "")
end

and QName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    namespace_: NamespaceQName.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; namespace_} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("namespace_", Option.map (fun x -> NamespaceQName.to_json x) namespace_);
    ] |> List.filter_map Util.rem_opt)

end

and TypedefDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (QName.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and ModuleDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    declaration: ModuleDeclaration.t;
    attributes: UserAttribute.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; attributes} = 
    JSON_Object ([
      ("declaration", Some (ModuleDeclaration.to_json declaration));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
    ] |> List.filter_map Util.rem_opt)

end

and AttributeHasParameter: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    parameter: string;
    attribute: UserAttribute.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; parameter; attribute} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("parameter", Some (JSON_String parameter));
      ("attribute", Some (UserAttribute.to_json attribute));
    ] |> List.filter_map Util.rem_opt)

end

and Name: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key str = JSON_String str
end

and MethodDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    container: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; container} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("container", Some (ContainerDeclaration.to_json container));
    ] |> List.filter_map Util.rem_opt)

end

and AttributeToDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    attribute: UserAttribute.t;
    declaration: Declaration.t;
    file: Src.File.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {attribute; declaration; file} = 
    JSON_Object ([
      ("attribute", Some (UserAttribute.to_json attribute));
      ("declaration", Some (Declaration.to_json declaration));
      ("file", Some (Src.File.to_json file));
    ] |> List.filter_map Util.rem_opt)

end

and FileXRefs: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    file: Src.File.t;
    xrefs: XRef.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; xrefs} = 
    JSON_Object ([
      ("file", Some (Src.File.to_json file));
      ("xrefs", Some (JSON_Array (List.map (fun x -> XRef.to_json x) xrefs)));
    ] |> List.filter_map Util.rem_opt)

end

and Enumerator: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    enumeration: EnumDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; enumeration} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("enumeration", Some (EnumDeclaration.to_json enumeration));
    ] |> List.filter_map Util.rem_opt)

end

and DeclarationSpan: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    declaration: Declaration.t;
    file: Src.File.t;
    span: Src.ByteSpan.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; file; span} = 
    JSON_Object ([
      ("declaration", Some (Declaration.to_json declaration));
      ("file", Some (Src.File.to_json file));
      ("span", Some (Src.ByteSpan.to_json span));
    ] |> List.filter_map Util.rem_opt)

end

and Signature: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    returns: Type.t option;
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

  and key= {
    returns: Type.t option;
    parameters: Parameter.t list;
    contexts: Context_.t list option;
    returns_type_info: TypeInfo.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {returns; parameters; contexts; returns_type_info} = 
    JSON_Object ([
      ("returns", Option.map (fun x -> Type.to_json x) returns);
      ("parameters", Some (JSON_Array (List.map (fun x -> Parameter.to_json x) parameters)));
      ("contexts", Option.map (fun x -> JSON_Array (List.map (fun x -> Context_.to_json x) x)) contexts);
      ("returnsTypeInfo", Option.map (fun x -> TypeInfo.to_json x) returns_type_info);
    ] |> List.filter_map Util.rem_opt)

end

and ContainerChild: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    container: ContainerDeclaration.t;
    child: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {container; child} = 
    JSON_Object ([
      ("container", Some (ContainerDeclaration.to_json container));
      ("child", Some (ContainerDeclaration.to_json child));
    ] |> List.filter_map Util.rem_opt)

end

and NamespaceQName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    parent: NamespaceQName.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; parent} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("parent", Option.map (fun x -> NamespaceQName.to_json x) parent);
    ] |> List.filter_map Util.rem_opt)

end

and InterfaceDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (QName.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and DeclarationSource: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    target: Declaration.t;
    source: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {target; source} = 
    JSON_Object ([
      ("target", Some (Declaration.to_json target));
      ("source", Some (Declaration.to_json source));
    ] |> List.filter_map Util.rem_opt)

end

and UserAttribute: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    parameters: string list;
    qname: QName.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; parameters; qname} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("parameters", Some (JSON_Array (List.map (fun x -> JSON_String x) parameters)));
      ("qname", Option.map (fun x -> QName.to_json x) qname);
    ] |> List.filter_map Util.rem_opt)

end

and ModuleDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and PropertyDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    declaration: PropertyDeclaration.t;
    type_: Type.t option;
    visibility: Visibility.t;
    is_final: bool;
    is_abstract: bool;
    is_static: bool;
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

  and key= {
    declaration: PropertyDeclaration.t;
    type_: Type.t option;
    visibility: Visibility.t;
    is_final: bool;
    is_abstract: bool;
    is_static: bool;
    attributes: UserAttribute.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; type_; visibility; is_final; is_abstract; is_static; attributes} = 
    JSON_Object ([
      ("declaration", Some (PropertyDeclaration.to_json declaration));
      ("type", Option.map (fun x -> Type.to_json x) type_);
      ("visibility", Some (Visibility.to_json visibility));
      ("isFinal", Some (JSON_Bool is_final));
      ("isAbstract", Some (JSON_Bool is_abstract));
      ("isStatic", Some (JSON_Bool is_static));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
    ] |> List.filter_map Util.rem_opt)

end

and MemberCluster: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    members: Declaration.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {members} = 
    JSON_Object ([
      ("members", Some (JSON_Array (List.map (fun x -> Declaration.to_json x) members)));
    ] |> List.filter_map Util.rem_opt)

end

and ClassConstDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    container: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; container} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("container", Some (ContainerDeclaration.to_json container));
    ] |> List.filter_map Util.rem_opt)

end

and EnumDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (QName.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and DeclarationComment: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    declaration: Declaration.t;
    file: Src.File.t;
    span: Src.ByteSpan.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; file; span} = 
    JSON_Object ([
      ("declaration", Some (Declaration.to_json declaration));
      ("file", Some (Src.File.to_json file));
      ("span", Some (Src.ByteSpan.to_json span));
    ] |> List.filter_map Util.rem_opt)

end

and NamespaceDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: NamespaceQName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (NamespaceQName.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and ClassDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
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
    JSON_Object ([
      ("declaration", Some (ClassDeclaration.to_json declaration));
      ("isAbstract", Some (JSON_Bool is_abstract));
      ("isFinal", Some (JSON_Bool is_final));
      ("members", Some (JSON_Array (List.map (fun x -> Declaration.to_json x) members)));
      ("extends_", Option.map (fun x -> ClassDeclaration.to_json x) extends_);
      ("implements_", Some (JSON_Array (List.map (fun x -> InterfaceDeclaration.to_json x) implements_)));
      ("uses", Some (JSON_Array (List.map (fun x -> TraitDeclaration.to_json x) uses)));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
      ("typeParams", Some (JSON_Array (List.map (fun x -> TypeParameter.to_json x) type_params)));
      ("module_", Option.map (fun x -> ModuleMembership.to_json x) module_);
    ] |> List.filter_map Util.rem_opt)

end

and MethodOccurrence: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    class_name: Name.t option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; class_name} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("className", Option.map (fun x -> Name.to_json x) class_name);
    ] |> List.filter_map Util.rem_opt)

end

and MethodOverrides: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    derived: MethodDeclaration.t;
    base: MethodDeclaration.t;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    derived: MethodDeclaration.t;
    base: MethodDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {derived; base} = 
    JSON_Object ([
      ("derived", Some (MethodDeclaration.to_json derived));
      ("base", Some (MethodDeclaration.to_json base));
    ] |> List.filter_map Util.rem_opt)

end

and Type: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key str = JSON_String str
end

and NameLowerCase: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name_lowercase: string;
    name: Name.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name_lowercase; name} = 
    JSON_Object ([
      ("nameLowercase", Some (JSON_String name_lowercase));
      ("name", Some (Name.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and TypeConstDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    declaration: TypeConstDeclaration.t;
    type_: Type.t option;
    kind: TypeConstKind.t;
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

  and key= {
    declaration: TypeConstDeclaration.t;
    type_: Type.t option;
    kind: TypeConstKind.t;
    attributes: UserAttribute.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; type_; kind; attributes} = 
    JSON_Object ([
      ("declaration", Some (TypeConstDeclaration.to_json declaration));
      ("type", Option.map (fun x -> Type.to_json x) type_);
      ("kind", Some (TypeConstKind.to_json kind));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
    ] |> List.filter_map Util.rem_opt)

end

and DeclarationTarget: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    source: Declaration.t;
    target: Declaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {source; target} = 
    JSON_Object ([
      ("source", Some (Declaration.to_json source));
      ("target", Some (Declaration.to_json target));
    ] |> List.filter_map Util.rem_opt)

end

and GlobalNamespaceAlias: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    from: Name.t;
    to_: NamespaceQName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {from; to_} = 
    JSON_Object ([
      ("from", Some (Name.to_json from));
      ("to", Some (NamespaceQName.to_json to_));
    ] |> List.filter_map Util.rem_opt)

end

and TypeInfo: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    display_type: Type.t;
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

  and key= {
    display_type: Type.t;
    xrefs: XRef.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {display_type; xrefs} = 
    JSON_Object ([
      ("displayType", Some (Type.to_json display_type));
      ("xrefs", Some (JSON_Array (List.map (fun x -> XRef.to_json x) xrefs)));
    ] |> List.filter_map Util.rem_opt)

end

and EnumDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    declaration: EnumDeclaration.t;
    enum_base: Type.t;
    enum_constraint: Type.t option;
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

  and key= {
    declaration: EnumDeclaration.t;
    enum_base: Type.t;
    enum_constraint: Type.t option;
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

  and to_json_key {declaration; enum_base; enum_constraint; enumerators; attributes; includes; is_enum_class; module_} = 
    JSON_Object ([
      ("declaration", Some (EnumDeclaration.to_json declaration));
      ("enumBase", Some (Type.to_json enum_base));
      ("enumConstraint", Option.map (fun x -> Type.to_json x) enum_constraint);
      ("enumerators", Some (JSON_Array (List.map (fun x -> Enumerator.to_json x) enumerators)));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
      ("includes", Some (JSON_Array (List.map (fun x -> EnumDeclaration.to_json x) includes)));
      ("isEnumClass", Some (JSON_Bool is_enum_class));
      ("module_", Option.map (fun x -> ModuleMembership.to_json x) module_);
    ] |> List.filter_map Util.rem_opt)

end

and ModuleParent: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    decl: Declaration.t;
    module_: ModuleDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {decl; module_} = 
    JSON_Object ([
      ("decl", Some (Declaration.to_json decl));
      ("module", Some (ModuleDeclaration.to_json module_));
    ] |> List.filter_map Util.rem_opt)

end

and ClassConstDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    declaration: ClassConstDeclaration.t;
    type_: Type.t option;
    value: string option;
  }
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
    declaration: ClassConstDeclaration.t;
    type_: Type.t option;
    value: string option;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; type_; value} = 
    JSON_Object ([
      ("declaration", Some (ClassConstDeclaration.to_json declaration));
      ("type", Option.map (fun x -> Type.to_json x) type_);
      ("value", Option.map (fun x -> JSON_String x) value);
    ] |> List.filter_map Util.rem_opt)

end

and StringLiteral: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= string
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key str = JSON_String str
end

and GlobalConstDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: QName.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name} = 
    JSON_Object ([
      ("name", Some (QName.to_json name));
    ] |> List.filter_map Util.rem_opt)

end

and FunctionDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
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
    JSON_Object ([
      ("declaration", Some (FunctionDeclaration.to_json declaration));
      ("signature", Some (Signature.to_json signature));
      ("isAsync", Some (JSON_Bool is_async));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
      ("typeParams", Some (JSON_Array (List.map (fun x -> TypeParameter.to_json x) type_params)));
      ("module_", Option.map (fun x -> ModuleMembership.to_json x) module_);
      ("readonlyRet", Option.map (fun x -> ReadonlyKind.to_json x) readonly_ret);
    ] |> List.filter_map Util.rem_opt)

end

and TraitDefinition: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
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
    JSON_Object ([
      ("declaration", Some (TraitDeclaration.to_json declaration));
      ("members", Some (JSON_Array (List.map (fun x -> Declaration.to_json x) members)));
      ("implements_", Some (JSON_Array (List.map (fun x -> InterfaceDeclaration.to_json x) implements_)));
      ("uses", Some (JSON_Array (List.map (fun x -> TraitDeclaration.to_json x) uses)));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
      ("typeParams", Some (JSON_Array (List.map (fun x -> TypeParameter.to_json x) type_params)));
      ("requireExtends", Some (JSON_Array (List.map (fun x -> ClassDeclaration.to_json x) require_extends)));
      ("requireImplements", Some (JSON_Array (List.map (fun x -> InterfaceDeclaration.to_json x) require_implements)));
      ("module_", Option.map (fun x -> ModuleMembership.to_json x) module_);
      ("requireClass", Option.map (fun x -> JSON_Array (List.map (fun x -> ClassDeclaration.to_json x) x)) require_class);
    ] |> List.filter_map Util.rem_opt)

end

and DeclarationName: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= Declaration.t
  [@@deriving ord]
  and value= Name.t
  [@@deriving ord]

  val to_json: t -> json

  val to_json_key: key -> json
  val to_json_value: value -> json

end = struct
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= Declaration.t
  [@@deriving ord]
  and value= Name.t
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key x = Declaration.to_json x
  and to_json_value x = Name.to_json x
end

and InheritedMembers: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    container: ContainerDeclaration.t;
    inherited_members: MemberCluster.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {container; inherited_members} = 
    JSON_Object ([
      ("container", Some (ContainerDeclaration.to_json container));
      ("inheritedMembers", Some (JSON_Array (List.map (fun x -> MemberCluster.to_json x) inherited_members)));
    ] |> List.filter_map Util.rem_opt)

end

and PropertyDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    container: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; container} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("container", Some (ContainerDeclaration.to_json container));
    ] |> List.filter_map Util.rem_opt)

end

and FileCall: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
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
    JSON_Object ([
      ("file", Some (Src.File.to_json file));
      ("callee_span", Some (Src.ByteSpan.to_json callee_span));
      ("call_args", Some (JSON_Array (List.map (fun x -> CallArgument.to_json x) call_args)));
      ("callee_xref", Option.map (fun x -> XRefTarget.to_json x) callee_xref);
      ("dispatch_arg", Option.map (fun x -> CallArgument.to_json x) dispatch_arg);
      ("receiver_type", Option.map (fun x -> Declaration.to_json x) receiver_type);
      ("callee_xrefs", Some (JSON_Array (List.map (fun x -> XRefTarget.to_json x) callee_xrefs)));
    ] |> List.filter_map Util.rem_opt)

end

and FileDeclarations: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    file: Src.File.t;
    declarations: Declaration.t list;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {file; declarations} = 
    JSON_Object ([
      ("file", Some (Src.File.to_json file));
      ("declarations", Some (JSON_Array (List.map (fun x -> Declaration.to_json x) declarations)));
    ] |> List.filter_map Util.rem_opt)

end

and TypeConstDeclaration: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    name: Name.t;
    container: ContainerDeclaration.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {name; container} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("container", Some (ContainerDeclaration.to_json container));
    ] |> List.filter_map Util.rem_opt)

end

and DeclarationLocation: sig
  type t =
    | Id of Fact_id.t
    | Key of key
  [@@deriving ord]

  and key= {
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

  and key= {
    declaration: Declaration.t;
    file: Src.File.t;
    span: Src.ByteSpan.t;
  }
  [@@deriving ord]

  let rec to_json = function
    | Id f -> Util.id f
    | Key t -> Util.key (to_json_key t)

  and to_json_key {declaration; file; span} = 
    JSON_Object ([
      ("declaration", Some (Declaration.to_json declaration));
      ("file", Some (Src.File.to_json file));
      ("span", Some (Src.ByteSpan.to_json span));
    ] |> List.filter_map Util.rem_opt)

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

  let to_json  = function
     | Lit lit -> JSON_Object [("lit", StringLiteral.to_json lit)]
     | Xref xref -> JSON_Object [("xref", XRefTarget.to_json xref)]

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

  let to_json {name; variance; reify_kind; constraints; attributes} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("variance", Some (Variance.to_json variance));
      ("reifyKind", Some (ReifyKind.to_json reify_kind));
      ("constraints", Some (JSON_Array (List.map (fun x -> Constraint.to_json x) constraints)));
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
    ] |> List.filter_map Util.rem_opt)

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

  let to_json  = function
     | Readonly -> JSON_Number (string_of_int 0)

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

  let to_json  = function
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

  let to_json  = function
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

  let to_json  = function
     | Declaration declaration -> JSON_Object [("declaration", Declaration.to_json declaration)]
     | Occurrence occurrence -> JSON_Object [("occurrence", Occurrence.to_json occurrence)]

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

  let to_json {declaration; internal} = 
    JSON_Object ([
      ("declaration", Some (ModuleDeclaration.to_json declaration));
      ("internal", Some (JSON_Bool internal));
    ] |> List.filter_map Util.rem_opt)

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

  let to_json  = function
     | Erased -> JSON_Number (string_of_int 0)
     | Reified -> JSON_Number (string_of_int 1)
     | SoftReified -> JSON_Number (string_of_int 2)

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

  let to_json {target; ranges} = 
    JSON_Object ([
      ("target", Some (XRefTarget.to_json target));
      ("ranges", Some (JSON_Array (List.map (fun x -> Src.RelByteSpan.to_json x) ranges)));
    ] |> List.filter_map Util.rem_opt)

end

and Parameter: sig
  type t = {
    name: Name.t;
    type_: Type.t option;
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
    type_: Type.t option;
    is_inout: bool;
    is_variadic: bool;
    default_value: string option;
    attributes: UserAttribute.t list;
    type_info: TypeInfo.t option;
    readonly: ReadonlyKind.t option;
  }
  [@@deriving ord]

  let to_json {name; type_; is_inout; is_variadic; default_value; attributes; type_info; readonly} = 
    JSON_Object ([
      ("name", Some (Name.to_json name));
      ("type", Option.map (fun x -> Type.to_json x) type_);
      ("isInout", Some (JSON_Bool is_inout));
      ("isVariadic", Some (JSON_Bool is_variadic));
      ("defaultValue", Option.map (fun x -> JSON_String x) default_value);
      ("attributes", Some (JSON_Array (List.map (fun x -> UserAttribute.to_json x) attributes)));
      ("typeInfo", Option.map (fun x -> TypeInfo.to_json x) type_info);
      ("readonly", Option.map (fun x -> ReadonlyKind.to_json x) readonly);
    ] |> List.filter_map Util.rem_opt)

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

  let to_json {span; argument} = 
    JSON_Object ([
      ("span", Some (Src.RelByteSpan.to_json span));
      ("argument", Option.map (fun x -> Argument.to_json x) argument);
    ] |> List.filter_map Util.rem_opt)

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

  let to_json  = function
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

  let to_json  = function
     | Method method_ -> JSON_Object [("method", MethodOccurrence.to_json method_)]

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

  let to_json  = function
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

  let to_json  = function
     | Abstract -> JSON_Number (string_of_int 0)
     | Concrete -> JSON_Number (string_of_int 1)
     | PartiallyAbstract -> JSON_Number (string_of_int 2)

end

and Constraint: sig
  type t = {
    constraint_kind: ConstraintKind.t;
    type_: Type.t;
  }
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = {
    constraint_kind: ConstraintKind.t;
    type_: Type.t;
  }
  [@@deriving ord]

  let to_json {constraint_kind; type_} = 
    JSON_Object ([
      ("constraintKind", Some (ConstraintKind.to_json constraint_kind));
      ("type", Some (Type.to_json type_));
    ] |> List.filter_map Util.rem_opt)

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

  let to_json  = function
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

  let to_json  = function
     | As -> JSON_Number (string_of_int 0)
     | Equal -> JSON_Number (string_of_int 1)
     | Super -> JSON_Number (string_of_int 2)

end


