(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val member_type :
  Typing_env_types.env -> Typing_defs.class_elt -> Typing_defs.decl_ty

(** Check an enum declaration of the form

     enum E : <ty_exp> as <ty_constraint>

  or

     class E extends Enum<ty_exp>

  where the absence of `<ty_constraint>` is assumed to default to arraykey.
  [name_pos] is the position of `E`.

  Check that `<ty_exp>` is int or string, and that

    ty_exp <: ty_constraint <: arraykey

  Also that each type constant is of type `ty_exp`. *)
val enum_class_check :
  Typing_env_types.env ->
  name_pos:Pos.t ->
  Folded_class.t ->
  Nast.class_const list ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env
