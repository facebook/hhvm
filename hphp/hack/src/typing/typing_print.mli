(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_env_types

(*****************************************************************************)
(* Pretty printing of types *)
(*****************************************************************************)

val error : env -> Typing_defs.locl_ty -> string

val full : env -> Typing_defs.locl_ty -> string

val full_rec : env -> int -> Typing_defs.locl_ty -> string

val full_strip_ns : env -> Typing_defs.locl_ty -> string

val full_strip_ns_decl : env -> Typing_defs.decl_ty -> string

val full_decl : TypecheckerOptions.t -> Typing_defs.decl_ty -> string

val fun_type : TypecheckerOptions.t -> Typing_defs.decl_fun_type -> string

val full_with_identity :
  env ->
  Typing_defs.locl_ty ->
  'b SymbolOccurrence.t ->
  'b SymbolDefinition.t option ->
  string
(** Pretty print a type and all of its associated declaration information. *)

val debug : env -> Typing_defs.locl_ty -> string

val debug_decl : env -> Typing_defs.decl_ty -> string

val with_blank_tyvars : (unit -> 'a) -> 'a

val class_ : TypecheckerOptions.t -> Decl_provider.class_decl -> string

val gconst : TypecheckerOptions.t -> Decl_provider.gconst_decl -> string

val fun_ : TypecheckerOptions.t -> Decl_provider.fun_decl -> string

val typedef : TypecheckerOptions.t -> Decl_provider.typedef_decl -> string

val constraints_for_type : env -> Typing_defs.locl_ty -> string option

val class_kind : Ast_defs.class_kind -> bool -> string

val subtype_prop : env -> Typing_logic.subtype_prop -> string

(* Convert a type to a structured JSON value, as follows:
 * <prim> ::= "int" | "bool" | "float" | "string" | "num" | "arraykey"
 *         | "mixed" | "resource" | "void" | "noreturn"
 * <array-args> ::= [ <type> ] | [ <type>, <type> ]
 * <type> ::=
 *   mixed
 *     { "kind":"mixed" }
 *   this
 *     { "kind":"this" }
 *   any
 *     { "kind":"any" }
 *   Anonymous lambda types
 *     { "kind":"anon" }
 *   Primitives
 *     { "kind":"primitive", "name":<prim> }
 *   Arrays, with 0, 1 or 2 type arguments
 *     { "kind":"array", "args":<array-args> }
 *   Enums (with optional bound)
 *     { "kind":"enum", "name":<identifier> [,"as":<type>] }
 *   Newtype (with optional bound)
 *     { "kind":"newtype", "name":<identifier>, "args":<type-arguments>
 *        [,"as":<type>] }
 *   Nullable types, with singleton type argument
 *     { "kind":"nullable", "args":<type-arguments> }
 *   Classes (or interfaces, or traits) with 0 or more type arguments
 *     { "kind":"class", "name":<identifier>, "args":<type-arguments> }
 *   Generic parameters
 *     { "kind":"generic", "name":<identifier> }
 *   Tuples (and tuple-like arrays)
 *     { "kind":"tuple", "args":<type-arguments> }
 *   Shapes (and shape-like arrays) with fields
 *     { "kind":"shape", "fields":<fields> }
 *   Function types, with 0 or more arguments and possibly-void result type
 *     { "kind":"function", "args":<type-arguments>, "result":<type> }
 *   Unions of types, with 2 or more arguments
 *     { "kind":"union", "args":<type-arguments> }
 *   Type constant paths, with base type and list of identifiers
 *     { "kind":"path", "type":<type>, "path":<identifiers> [,"as":<type>] }
 *     Base type may be
 *         { "kind":"static" }
 *         { "kind":"this" }
 *         { "kind":"class", "name":<identifier>, "args":[]}
 *       Expression-dependent type
 *         { "kind":"expr" }
 *)
val to_json : env -> Typing_defs.locl_ty -> Hh_json.json

(* Attempt to deserialize a previously-serialized type back into a type we can
manipulate. Note that this function accesses the global state in
`Decl_provider` to verify that certain type names exist. *)
val json_to_locl_ty :
  ?keytrace:Hh_json.Access.keytrace ->
  Hh_json.json ->
  (Typing_defs.locl_ty, Typing_defs.deserialization_error) result
