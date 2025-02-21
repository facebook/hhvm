(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Instantiation : sig
  type t = private {
    this: Typing_defs.locl_ty option;
    subst: Typing_defs.locl_ty SMap.t;
  }

  val is_empty : t -> bool
end

(** Given two types ty1 and ty2, assuming ty2 is an instantiation
    of ty1, return the type substitution to apply to ty1 to obtain ty2.
    This won't fail if ty2 is not an instantiation of ty1 but instead will return
    a best effort substitution. *)
val derive_instantiation :
  Typing_env_types.env ->
  Typing_defs.decl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Instantiation.t
