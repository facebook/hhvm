(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Provider :
  Decl_enforceability.Provider
    with type t = Provider_context.t
     and type class_t = Shallow_decl_defs.shallow_class

(** If the return type is not enforceable, turn it into a like type (~ty) otherwise
    return the original function type. Also add supportdyn<mixed> to the type parameters. *)
val pessimise_fun_type :
  fun_kind:Decl_enforceability.fun_kind ->
  this_class:Shallow_decl_defs.shallow_class option ->
  no_auto_likes:bool ->
  cannot_override:bool ->
  Provider_context.t ->
  Pos_or_decl.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty
