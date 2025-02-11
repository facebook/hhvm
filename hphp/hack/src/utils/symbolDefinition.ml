(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type classish_kind =
  | Class
  | Interface
  | Trait
  | Enum
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

type 'a kind =
  | Function
  | Classish of {
      classish_kind: classish_kind;
      members: 'a t list;
    }
  | Method
  | Property
  | ClassConst
  | GlobalConst
  | LocalVar
  | TypeVar
  | Typeconst
  | Param
  | Typedef
  | Module

and 'a t = {
  kind: 'a kind;
  name: string;
  class_name: string option;
  pos: 'a Pos.pos;
  span: 'a Pos.pos;  (** covers the span of just the identifier *)
  modifiers: modifier list;
      (** covers the span of the entire construct, including children *)
  params: 'a t list option;
  docblock: string option;
  detail: string option;
}
[@@deriving ord, show]

let rec to_absolute
    { kind; name; class_name; pos; span; modifiers; params; docblock; detail } =
  {
    kind = kind_to_absolute kind;
    name;
    class_name;
    pos = Pos.to_absolute pos;
    span = Pos.to_absolute span;
    modifiers;
    params = Option.map params ~f:(fun x -> List.map x ~f:to_absolute);
    docblock;
    detail;
  }

and kind_to_absolute kind =
  match kind with
  | Classish { classish_kind; members } ->
    Classish { classish_kind; members = List.map members ~f:to_absolute }
  | Function -> Function
  | Method -> Method
  | Property -> Property
  | ClassConst -> ClassConst
  | GlobalConst -> GlobalConst
  | LocalVar -> LocalVar
  | TypeVar -> TypeVar
  | Typeconst -> Typeconst
  | Param -> Param
  | Typedef -> Typedef
  | Module -> Module

let rec to_relative
    { kind; name; class_name; pos; span; modifiers; params; docblock; detail } =
  {
    kind = kind_to_relative kind;
    name;
    class_name;
    pos = Pos.to_relative pos;
    span = Pos.to_relative span;
    modifiers;
    params = Option.map params ~f:(fun x -> List.map x ~f:to_relative);
    docblock;
    detail;
  }

and kind_to_relative kind =
  match kind with
  | Classish { classish_kind; members } ->
    Classish { classish_kind; members = List.map members ~f:to_relative }
  | Function -> Function
  | Method -> Method
  | Property -> Property
  | ClassConst -> ClassConst
  | GlobalConst -> GlobalConst
  | LocalVar -> LocalVar
  | TypeVar -> TypeVar
  | Typeconst -> Typeconst
  | Param -> Param
  | Typedef -> Typedef
  | Module -> Module

let full_name { name; class_name; kind; _ } =
  match (kind, class_name) with
  | (_, None)
  | ( ( Classish _ | Typedef | Param | Function | GlobalConst | LocalVar
      | Module | TypeVar ),
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
      | Classish _
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
  | Classish { classish_kind; _ } ->
    (match classish_kind with
    | Class -> "class"
    | Enum -> "enum"
    | Trait -> "trait"
    | Interface -> "interface")
  | Method -> "method"
  | Property -> "property"
  | ClassConst -> "class constant"
  | GlobalConst -> "const"
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
