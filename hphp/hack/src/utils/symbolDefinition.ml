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

type member_kind =
  | Method
  | Property
  | ClassConst
  | TypeConst
[@@deriving ord, show]

type 'a kind =
  | Function
  | Classish of {
      classish_kind: classish_kind;
      members: 'a t list;
    }
  | Member of {
      member_kind: member_kind;
      class_name: string;
    }
  | GlobalConst
  | LocalVar
  | TypeVar
  | Param
  | Typedef
  | Module

and 'a t = {
  kind: 'a kind;
  name: string;
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
    { kind; name; pos; span; modifiers; params; docblock; detail } =
  {
    kind = kind_to_absolute kind;
    name;
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
  | Member { member_kind; class_name } -> Member { member_kind; class_name }
  | GlobalConst -> GlobalConst
  | LocalVar -> LocalVar
  | TypeVar -> TypeVar
  | Param -> Param
  | Typedef -> Typedef
  | Module -> Module

let rec to_relative
    { kind; name; pos; span; modifiers; params; docblock; detail } =
  {
    kind = kind_to_relative kind;
    name;
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
  | Member { member_kind; class_name } -> Member { member_kind; class_name }
  | GlobalConst -> GlobalConst
  | LocalVar -> LocalVar
  | TypeVar -> TypeVar
  | Param -> Param
  | Typedef -> Typedef
  | Module -> Module

let full_name { name; kind; _ } =
  match kind with
  | Member { class_name; _ } -> Printf.sprintf "%s::%s" class_name name
  | Classish _
  | Typedef
  | Param
  | Function
  | GlobalConst
  | LocalVar
  | Module
  | TypeVar ->
    name

let identifier def =
  match def.kind with
  | TypeVar -> Some def.name
  | _ ->
    let kind_string =
      match def.kind with
      | Member { member_kind; _ } ->
        (match member_kind with
        | Property -> Some "property"
        | ClassConst
        | TypeConst ->
          Some "class_const"
        | Method -> Some "method")
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
  | Member { member_kind; _ } ->
    (match member_kind with
    | Method -> "method"
    | Property -> "property"
    | ClassConst -> "class constant"
    | TypeConst -> "typeconst")
  | GlobalConst -> "const"
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
