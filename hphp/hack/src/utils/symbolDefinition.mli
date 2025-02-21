(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type classish_kind =
  | Class
  | Interface
  | Trait
  | Enum

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
[@@deriving ord, show]

and 'a t = {
  kind: 'a kind;
  name: string;
  pos: 'a Pos.pos;  (** covers the span of just the identifier *)
  span: 'a Pos.pos;
      (** covers the span of the entire construct, including children *)
  modifiers: modifier list;
  params: 'a t list option;
      (** For functions and methods, the list of its parameter definitions *)
  docblock: string option;
      (** Only provided on some code paths,
          such as FileOutline.outline (but not FileOutline.outline_entry_no_comments) *)
  detail: string option;
      (** misc. unstructured information about the symbol.
          For functions, we include function signature with param names added.
          Current use case is providing additional context for AI coding assistants.
          Only provided on some code paths: File_outline.outline *)
}
[@@deriving ord, show]

val to_absolute : Relative_path.t t -> string t

val to_relative : string t -> Relative_path.t t

(** For a class member definition, a string like `ClassName::memberName`. Otherwise just the `name`. *)
val full_name : 'a t -> string

(** A string like <kind>::Type or <kind>::Type::member where `kind` is one of "function",
   "type_id", "method", etc. None for type var, local var and params *)
val identifier : 'a t -> string option

val string_of_kind : 'a kind -> string

val string_of_modifier : modifier -> string

(** Whether the list of modifiers contain the 'static' modifier *)
val is_static : modifier list -> bool
