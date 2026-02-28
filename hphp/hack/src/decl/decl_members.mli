(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This modules transforms member types from Shallow_decl_defs like
    `shallow_method` into member types from Typing_defs like `fun_elt`.
    It may perform pessimization along the way. *)

module Make : functor (Provider : Decl_enforceability.ShallowProvider) -> sig
  (** May pessimize the property. *)
  val build_property :
    ctx:Provider.t ->
    this_class:Shallow_decl_defs.shallow_class option ->
    Shallow_decl_defs.shallow_prop ->
    Typing_defs.decl_ty

  (** May pessimize the method. *)
  val build_method :
    ctx:Provider.t ->
    this_class:Shallow_decl_defs.shallow_class option ->
    no_auto_likes:bool ->
    Shallow_decl_defs.shallow_method ->
    Typing_defs.fun_elt

  val build_constructor :
    Shallow_decl_defs.shallow_method -> Typing_defs.fun_elt
end
