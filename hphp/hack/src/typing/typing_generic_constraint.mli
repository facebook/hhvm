(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Add a constraint to the environment.
    Raise an error if any inconsistency is detected. *)
val check_constraint :
  Typing_env_types.env ->
  Ast_defs.constraint_kind ->
  Typing_defs.locl_ty ->
  cstr_ty:Typing_defs.locl_ty ->
  Errors.Reasons_callback.t ->
  Typing_env_types.env

(** Add an [as] or [super] constraint to the environment.
    Raise an error if any inconsistency is detected. *)
val check_tparams_constraint :
  Typing_env_types.env ->
  use_pos:Pos.t ->
  Ast_defs.constraint_kind ->
  cstr_ty:Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env

(** Add a [where] constraint to the environment.
    Raise an error if any inconsistency is detected. *)
val check_where_constraint :
  in_class:bool ->
  Typing_env_types.env ->
  use_pos:Pos.t ->
  definition_pos:Pos_or_decl.t ->
  Ast_defs.constraint_kind ->
  cstr_ty:Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env
