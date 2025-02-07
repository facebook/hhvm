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

let identifier def =
  match def.kind with
  | TypeVar -> Some def.name
  | _ ->
    let kind_string =
      match def.kind with
      | Property -> Some "property"
      | ClassConst
      | Typeconst ->
        Some "class_const"
      | Method -> Some "method"
      | Class
      | Interface
      | Trait
      | Enum
      | Typedef ->
        Some "type_id"
      | Function -> Some "function"
      | GlobalConst -> Some "global_const"
      | Module -> Some "module"
      | TypeVar
      | LocalVar
      | Param ->
        None
    in
    (match kind_string with
    | None -> None
    | Some kind_string ->
      let full_name = full_name def in
      Some (Printf.sprintf "%s::%s" kind_string full_name))

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
