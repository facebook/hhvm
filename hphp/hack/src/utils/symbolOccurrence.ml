(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type override_info = {
  class_name: string;
  method_name: string;
  is_static: bool;
}
[@@deriving ord, eq]

type class_id_type =
  | ClassId
  | Other
[@@deriving ord, eq]

type receiver_class =
  | ClassName of string
  | UnknownClass (* invoked dynamically *)
[@@deriving ord, eq]

type keyword_with_hover_docs =
  | FinalOnClass
  | FinalOnMethod
  | AbstractOnClass
  | AbstractOnMethod
  | ExtendsOnClass
  | ExtendsOnInterface
  | ReadonlyOnMethod
  | ReadonlyOnParameter
  | ReadonlyOnReturnType
  | ReadonlyOnExpression
  | Async
  | AsyncBlock
  | Await
  | Concurrent
[@@deriving ord, eq]

type built_in_type_hint =
  | BIprimitive of Aast_defs.tprim
  | BImixed
  | BIdynamic
  | BInothing
  | BInonnull
  | BIshape
  (* TODO: support self and static too.*)
  | BIthis
  | BIoption
[@@deriving ord, eq]

type kind =
  | Class of class_id_type
  | BuiltInType of built_in_type_hint
  | Function
  | Method of receiver_class * string
  | LocalVar
  | TypeVar
  | Property of receiver_class * string
  | XhpLiteralAttr of string * string
  | ClassConst of receiver_class * string
  | Typeconst of string * string
  | GConst
  (* For __Override occurrences, we track the associated method and class. *)
  | Attribute of override_info option
  (* enum class name, label name *)
  | EnumClassLabel of string * string
  | Keyword of keyword_with_hover_docs
  | PureFunctionContext
[@@deriving ord, eq]

type 'a t = {
  name: string;
  type_: kind;
  is_declaration: bool;
  (* Span of the symbol itself *)
  pos: 'a Pos.pos;
}
[@@deriving ord]

let to_absolute x = { x with pos = Pos.to_absolute x.pos }

let kind_to_string = function
  | Class _ -> "class"
  | BuiltInType _ -> "built_in_type"
  | Method _ -> "method"
  | Function -> "function"
  | LocalVar -> "local"
  | Property _ -> "property"
  | XhpLiteralAttr _ -> "xhp_literal_attribute"
  | ClassConst _ -> "member_const"
  | Typeconst _ -> "typeconst"
  | GConst -> "global_const"
  | TypeVar -> "generic_type_var"
  | Attribute _ -> "attribute"
  | EnumClassLabel _ -> "enum_class_label"
  | Keyword _ -> "keyword"
  | PureFunctionContext -> "context_braces"

let enclosing_class occurrence =
  match occurrence.type_ with
  | Method (ClassName c, _)
  | Property (ClassName c, _)
  | XhpLiteralAttr (c, _)
  | ClassConst (ClassName c, _)
  | Typeconst (c, _)
  | EnumClassLabel (c, _) ->
    Some c
  | _ -> None

let get_class_name occurrence =
  match enclosing_class occurrence with
  | Some _ as res -> res
  | None ->
    (match occurrence.type_ with
    | Class _
    | Attribute _ ->
      Some occurrence.name
    | _ -> None)

let is_constructor occurrence =
  match occurrence.type_ with
  | Method (_, name) when name = Naming_special_names.Members.__construct ->
    true
  | _ -> false

let is_class occurrence =
  match occurrence.type_ with
  | Class _ -> true
  | _ -> false

let is_xhp_literal_attr occurrence =
  match occurrence.type_ with
  | XhpLiteralAttr _ -> true
  | _ -> false
