(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val expand_typedef :
  Typing_defs.expand_env ->
  Typing_env_types.env ->
  Typing_reason.t ->
  string ->
  Typing_defs.locl_ty list ->
  Typing_utils.expand_typedef_result

(** Expand a typedef, smashing abstraction and collecting a trail
  of where the typedefs come from.

  /!\ This only does something if passed a Tnewtype. Not sure if that's a bug.
  *)
val force_expand_typedef :
  ety_env:Typing_defs.expand_env ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  (Typing_env_types.env * Typing_error.t option)
  * Typing_defs.locl_ty
  * Pos_or_decl.t list
