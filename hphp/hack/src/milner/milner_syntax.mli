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

(** Reuse the identifier carried by a named parameter's function type. *)
val named_local : string -> local

val local_name : local -> string

type expr =
  | Unary of string * expr
  | Binary of string * expr * expr
  | As of expr * string
  | NullsafeMember of expr * string
  | AsyncLambda of parameter list * string list * string * stmt list
  | Await of expr
  | Xhp of string * (string * expr) list * expr list
  | Quote of string * expr
  | Splice of expr
  | Atom of string
      (** Compatibility with existing literal leaves and qualified names. *)
  | Local of local
  | New of string * expr list
  | NewDynamic of local * expr list
  | DynamicStaticMember of local * string
  | Is of expr * string
  | Nameof of string
  | Member of expr * string
  | StaticMember of string * string
  | EnumLabel of string * string
  | StaticProperty of string * string
  | Call of expr * expr list
  | DeclaredCall of declaration * string list * expr list
      (** A closed declaration and its invocation. Materialization is mandatory:
          generic parameters and polymorphic contexts cannot be lambda syntax.
          An empty type argument list requests inference. *)
  | DeclaredCallable of declaration * string list
      (** A closed declaration materialized as a reusable callable value. Currently
          requires a monomorphic declaration and concrete contexts. *)
  | NamedArgument of string * expr
  | Index of expr * expr
  | Append of expr
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
  named: bool;
  default: expr option;
}

and declaration = {
  type_parameters: string list;
  is_async: bool;
  parameters: parameter list;
  contexts: string list;
  return_hint: string;
  body: stmt list;
  memoize: bool;
}

and stmt =
  | If of expr * stmt list * stmt list
  | While of expr * stmt list
  | Foreach of expr * local option * local * stmt list
  | Try of stmt list * (string * local * stmt list) list * stmt list
  | Concurrent of stmt list
  | Bind of local * expr
  | Assign of expr * expr
  | Eval of expr
  | Return of expr option
  | Throw of expr
  | Block of stmt list

val parameter :
  ?variadic:bool -> ?named:bool -> ?default:expr -> string -> local -> parameter

(** Whether a generated hint can have a named default under T289176736.
    Nominal hints require the generator to establish nonnullability separately. *)
val supports_named_default : string -> bool

(** Canonical order avoids T289079831 in executable declarations. *)
val render_parameters : ?canonical:bool -> parameter list -> string

val render_expr : expr -> string

val render_stmt : stmt -> string

val render_body : stmt list -> string
