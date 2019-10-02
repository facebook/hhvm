(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val obj_get :
  obj_pos:Ast_defs.pos ->
  is_method:bool ->
  nullsafe:Ast_defs.pos option ->
  coerce_from_ty:
    (Ast_defs.pos * Typing_reason.ureason * Typing_defs.locl_ty) option ->
  ?explicit_tparams:Aast.targ list ->
  ?pos_params:'a ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Nast.class_id_ ->
  Ast_defs.pos * string ->
  Typing_env_types.env
  * (Typing_reason.t * Typing_defs.locl_phase Typing_defs.ty_)

val obj_get_ :
  inst_meth:bool ->
  is_method:bool ->
  nullsafe:Ast_defs.pos option ->
  obj_pos:Ast_defs.pos ->
  pos_params:'a option ->
  coerce_from_ty:
    (Ast_defs.pos * Typing_reason.ureason * Typing_defs.locl_ty) option ->
  ?explicit_tparams:Aast.targ list ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Nast.class_id_ ->
  Ast_defs.pos * string ->
  (Typing_defs.locl_ty -> Typing_defs.locl_ty) ->
  Typing_env_types.env
  * (Typing_reason.t * Typing_defs.locl_phase Typing_defs.ty_)

val smember_not_found :
  Ast_defs.pos ->
  is_const:bool ->
  is_method:bool ->
  Decl_provider.Class.t ->
  string ->
  unit
