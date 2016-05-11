(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

type kind =
  | Function
  | Class
  | Method
  | Property
  | Const
  | Enum
  | Interface
  | Trait
  | LocalVar
  | Typeconst

and modifier =
  | Final
  | Static
  | Abstract
  | Private
  | Public
  | Protected
  | Async

and 'a t = {
  kind : kind;
  name : string;
  pos : 'a Pos.pos;
  span : 'a Pos.pos;
  modifiers : modifier list;
  children : 'a t list;
}

let rec to_absolute x = {
  kind = x.kind;
  name = x.name;
  pos = Pos.to_absolute x.pos;
  span = Pos.to_absolute x.span;
  modifiers = x.modifiers;
  children = List.map x.children to_absolute;
}

let string_of_kind = function
  | Function -> "function"
  | Class -> "class"
  | Method -> "method"
  | Property -> "property"
  | Const -> "const"
  | Enum -> "enum"
  | Interface -> "interface"
  | Trait -> "trait"
  | Typeconst -> "typeconst"
  | LocalVar -> "local"

let string_of_modifier = function
  | Final -> "final"
  | Static -> "static"
  | Abstract -> "abstract"
  | Private -> "private"
  | Public -> "public"
  | Protected -> "protected"
  | Async -> "async"
