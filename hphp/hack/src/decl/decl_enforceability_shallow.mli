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
  return_from_async:bool ->
  this_class:Shallow_decl_defs.shallow_class option ->
  Provider_context.t ->
  Typing_defs.decl_ty ->
  bool

(** If the type is not enforceable, turn it into a like type (~ty) otherwise
    return the type *)
val pessimise_type :
  reason:Typing_reason.decl_t ->
  is_xhp_attr:bool ->
  this_class:Shallow_decl_defs.shallow_class option ->
  Provider_context.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty

(** Pessimise the type if in implicit pessimisation mode, otherwise
    return the type *)
val maybe_pessimise_type :
  reason:Typing_reason.decl_t ->
  is_xhp_attr:bool ->
  this_class:Shallow_decl_defs.shallow_class option ->
  Provider_context.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty

type fun_kind =
  | Function
  | Abstract_method
  | Concrete_method

(** If the return type is not enforceable, turn it into a like type (~ty) otherwise
    return the original function type. Also add supportdyn<mixed> to the type parameters. *)
val pessimise_fun_type :
  fun_kind:fun_kind ->
  this_class:Shallow_decl_defs.shallow_class option ->
  no_auto_likes:bool ->
  Provider_context.t ->
  Pos_or_decl.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty
