(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type kind =
  | Function
  | Class
  | Method
  | Property
  | ClassConst
  | GlobalConst
  | Enum
  | Interface
  | Trait
  | LocalVar
  | TypeVar
  | Typeconst
  | Param
  | Typedef
  | Module
[@@deriving ord, show]

type modifier =
  | Final
  | Static
  | Abstract
  | Private
  | Public
  | Protected
  | Async
  | Inout
  | Internal
[@@deriving ord, show]

type 'a t = {
  kind: kind;
  name: string;
  class_name: string option;
  id: string option;
  pos: 'a Pos.pos;
  span: 'a Pos.pos;  (** covers the span of just the identifier *)
  modifiers: modifier list;
      (** covers the span of the entire construct, including children *)
  children: 'a t list option;
  params: 'a t list option;
  docblock: string option;
  detail: string option;
}
[@@deriving ord, show]

let rec to_absolute x =
  {
    x with
    pos = Pos.to_absolute x.pos;
    span = Pos.to_absolute x.span;
    children = Option.map x.children ~f:(fun x -> List.map x ~f:to_absolute);
    params = Option.map x.params ~f:(fun x -> List.map x ~f:to_absolute);
    docblock = x.docblock;
  }

let rec to_relative x =
  {
    x with
    pos = Pos.to_relative x.pos;
    span = Pos.to_relative x.span;
    children = Option.map x.children ~f:(fun x -> List.map x ~f:to_relative);
    params = Option.map x.params ~f:(fun x -> List.map x ~f:to_relative);
  }

let full_name { name; class_name; kind; _ } =
  match (kind, class_name) with
  | (_, None)
  | ( ( Class | Interface | Trait | Enum | Typedef | Param | Function
      | GlobalConst | LocalVar | Module | TypeVar ),
      _ ) ->
    name
  | ((Method | Property | ClassConst | Typeconst), Some class_name) ->
    Printf.sprintf "%s::%s" class_name name

let string_of_kind = function
  | Function -> "function"
  | Class -> "class"
  | Method -> "method"
  | Property -> "property"
  | ClassConst -> "class constant"
  | GlobalConst -> "const"
  | Enum -> "enum"
  | Interface -> "interface"
  | Trait -> "trait"
  | Typeconst -> "typeconst"
  | LocalVar -> "local"
  | TypeVar -> "type_var"
  | Param -> "param"
  | Typedef -> "typedef"
  | Module -> "module"

let string_of_modifier = function
  | Final -> "final"
  | Static -> "static"
  | Abstract -> "abstract"
  | Private -> "private"
  | Public -> "public"
  | Protected -> "protected"
  | Async -> "async"
  | Inout -> "inout"
  | Internal -> "internal"

let function_kind_name = "function"

let type_id_kind_name = "type_id"

let method_kind_name = "method"

let property_kind_name = "property"

let class_const_kind_name = "class_const"

let global_const_kind_name = "global_const"

let module_kind_name = "module"

let get_symbol_id kind parent_class name =
  let prefix =
    match kind with
    | Function -> Some function_kind_name
    | Class
    | Typedef
    | Enum
    | Interface
    | Trait ->
      Some type_id_kind_name
    | Method -> Some method_kind_name
    | Property -> Some property_kind_name
    | Typeconst
    | ClassConst ->
      Some class_const_kind_name
    | GlobalConst -> Some global_const_kind_name
    | Module -> Some module_kind_name
    | LocalVar
    | TypeVar
    | Param ->
      None
  in
  match (prefix, parent_class) with
  | (Some prefix, Some parent_class) ->
    Some (Printf.sprintf "%s::%s::%s" prefix parent_class name)
  | (Some prefix, None) -> Some (Printf.sprintf "%s::%s" prefix name)
  | (None, _) -> None
