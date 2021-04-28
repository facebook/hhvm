(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val make_subst :
  'a Typing_defs.tparam list ->
  Typing_defs.decl_ty list ->
  Decl_subst.decl_subst

val instantiate :
  Decl_subst.decl_subst -> Typing_defs.decl_ty -> Typing_defs.decl_ty

val instantiate_ce :
  Decl_subst.decl_subst -> Typing_defs.class_elt -> Typing_defs.class_elt

val instantiate_cc :
  Decl_subst.decl_subst -> Typing_defs.class_const -> Typing_defs.class_const

val instantiate_typeconst :
  Decl_subst.decl_subst -> Typing_defs.typeconst -> Typing_defs.typeconst

val instantiate_typeconst_type :
  Decl_subst.decl_subst ->
  Typing_defs.typeconst_type ->
  Typing_defs.typeconst_type
