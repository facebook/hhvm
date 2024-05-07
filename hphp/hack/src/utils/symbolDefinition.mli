(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
  full_name: string;
  class_name: string option;
  id: string option;
  pos: 'a Pos.pos;
  (* covers the span of just the identifier *)
  span: 'a Pos.pos;
  (* covers the span of the entire construct, including children *)
  modifiers: modifier list;
  children: 'a t list option;
  params: 'a t list option;
      (** Only provided on some code paths,
      * such as FileOutline.outline (but not FileOutline.outline_entry_no_comments)
      *)
  docblock: string option;
      (** misc. unstructured information about the symbol.
      * For functions, we include function signature with param names added.
      * Current use case is providing additional context for AI coding assistants.
      * Only provided on some code paths: File_outline.outline*
      *)
  detail: string option;
}
[@@deriving ord, show]

val to_absolute : Relative_path.t t -> string t

val to_relative : string t -> Relative_path.t t

val string_of_kind : kind -> string

val string_of_modifier : modifier -> string

val function_kind_name : string

val type_id_kind_name : string

val method_kind_name : string

val property_kind_name : string

val class_const_kind_name : string

val get_symbol_id : kind -> string option -> string -> string option
