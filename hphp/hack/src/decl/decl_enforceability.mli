(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Checks if a type is one that HHVM will enforce as a parameter or return.
    If the function is async, then the contents of the Awaitable return are
    enforced. Otherwise they aren't. *)
val is_enforceable :
  return_from_async:bool -> Provider_context.t -> Typing_defs.decl_ty -> bool

(** If the type is not enforceable, turn it into a like type (~ty) otherwise
    return the type *)
val pessimise_type :
  is_xhp_attr:bool ->
  Provider_context.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty

(** Pessimise the type if in implicit pessimisation mode, otherwise
    return the type *)
val maybe_pessimise_type :
  is_xhp_attr:bool ->
  Provider_context.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty

(** If the return type is not enforceable, turn it into a like type (~ty) otherwise
    return the original function type. Also add supportdyn<mixed> to the type parameters. *)
val pessimise_fun_type :
  is_method:bool ->
  Provider_context.t ->
  Pos_or_decl.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty

(** Pessimise the type if in implicit pessimisation mode, otherwise
    return the type *)
val maybe_pessimise_fun_type :
  is_method:bool ->
  Provider_context.t ->
  Pos_or_decl.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty

(** Add as supportdyn<mixed> constraints to the type parameters *)
val add_supportdyn_constraints :
  Pos_or_decl.t ->
  Typing_defs.decl_ty Typing_defs_core.tparam list ->
  Typing_defs.decl_ty Typing_defs_core.tparam list

(** Add as supportdyn<mixed> constraints to the type parameters if in implicit pessimisation mode.*)
val maybe_add_supportdyn_constraints :
  Provider_context.t ->
  Pos_or_decl.t ->
  Typing_defs.decl_ty Typing_defs_core.tparam list ->
  Typing_defs.decl_ty Typing_defs_core.tparam list

val supportdyn_mixed :
  Pos_or_decl.t -> Typing_defs.Reason.decl_t -> Typing_defs.decl_ty
