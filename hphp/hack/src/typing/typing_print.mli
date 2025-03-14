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

val error : ?ignore_dynamic:bool -> env -> Typing_defs.locl_ty -> string

val full : hide_internals:bool -> env -> Typing_defs.locl_ty -> string

val full_i : hide_internals:bool -> env -> Typing_defs.internal_type -> string

val full_rec :
  hide_internals:bool -> env -> Tvid.t -> Typing_defs.locl_ty -> string

val full_strip_ns : hide_internals:bool -> env -> Typing_defs.locl_ty -> string

val full_strip_ns_i :
  hide_internals:bool -> env -> Typing_defs.internal_type -> string

(** Print a decl type, stripping backslash namespaces.

  @param msg          if fuel reaches 0, if msg = true, an error message is added to the
                      type representation. msg is true by default

  @param verbose_fun  print function types verbosely,
                      with type parameters, `async` keyword,
                      `where` constraints and disposable attributes.
  *)
val full_strip_ns_decl :
  ?msg:bool -> verbose_fun:bool -> env -> Typing_defs.decl_ty -> string

val full_decl :
  ?msg:bool -> TypecheckerOptions.t -> Typing_defs.decl_ty -> string

val fun_type : TypecheckerOptions.t -> Typing_defs.decl_fun_type -> string

(** Pretty print a type and all of its associated declaration information. *)
val full_with_identity :
  hide_internals:bool ->
  env ->
  Typing_defs.locl_ty ->
  'b SymbolOccurrence.t ->
  'b SymbolDefinition.t option ->
  string

(** Pretty print a type and all of its associated declaration information. *)
val full_decl_with_identity :
  env ->
  verbose_fun:bool ->
  Typing_defs.decl_ty ->
  'b SymbolOccurrence.t ->
  'b SymbolDefinition.t option ->
  string

(** Print a locl_type to a string, stripping namespaces, including exact annotations and type variable numbering *)
val debug : env -> Typing_defs.locl_ty -> string

val debug_decl : env -> Typing_defs.decl_ty -> string

(** Print an internal type to a string, stripping namespaces, including exact annotations and type variable numbering *)
val debug_i : env -> Typing_defs.internal_type -> string

val constraints_for_type :
  hide_internals:bool -> env -> Typing_defs.locl_ty -> string

val classish_kind : Ast_defs.classish_kind -> bool -> string

(** Print a suptype proposition, stripping namespaces, including exact annotations and type variable numbering *)
val subtype_prop : env -> Typing_logic.subtype_prop -> string

val coeffects : env -> Typing_defs.locl_ty -> string

val strip_ns : string -> string

(* Split type parameters [tparams] into normal user-denoted type
    parameters, and a list of polymorphic context names that were
    desugared to type parameters. *)

val split_desugared_ctx_tparams_gen :
  tparams:'a list -> param_name:('a -> string) -> string list * 'a list
