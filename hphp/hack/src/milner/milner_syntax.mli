(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Hack syntax shared by composable generation rules. This module only renders
    syntax; the generator must establish types, coeffects, legal assignment
    targets, and lexical scope before constructing a tree. *)
type local

(** A local has a unique name. Reuse the token for a binding and all its uses;
    composed scopes may capture existing tokens without renaming them. *)
val fresh_local : string -> local

type expr =
  | Atom of string
      (** Compatibility with existing literal leaves and qualified names. *)
  | Local of local
  | New of string * expr list
  | Member of expr * string
  | StaticMember of string * string
  | EnumLabel of string * string
  | StaticProperty of string * string
  | Call of expr * expr list
  | Index of expr * expr
  | Inout of expr
  | Unpack of expr
  | Lambda of parameter list * string list * string * stmt list
  | Array of string * expr list
  | KeyValue of expr * expr
  | Tuple of expr list
  | Shape of (string * expr) list
  | Async of stmt list

and parameter = {
  hint: string;
  local: local;
  variadic: bool;
  default: expr option;
}

and stmt =
  | Bind of local * expr
  | Assign of expr * expr
  | Eval of expr
  | Return of expr option
  | Throw of expr
  | Block of stmt list

val parameter : ?variadic:bool -> ?default:expr -> string -> local -> parameter

val render_expr : expr -> string

val render_stmt : stmt -> string

val render_body : stmt list -> string
