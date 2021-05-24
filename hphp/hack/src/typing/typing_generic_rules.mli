(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val apply_rules :
  ?ignore_type_structure:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  (Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty) ->
  Typing_env_types.env * Typing_defs.locl_ty

val apply_rules_with_err :
  ?ignore_type_structure:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  (Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env
  * Typing_defs.locl_ty
  * (Typing_defs.locl_ty, Typing_defs.locl_ty * Typing_defs.locl_ty) result) ->
  Typing_env_types.env
  * Typing_defs.locl_ty
  * (Typing_defs.locl_ty * Typing_defs.locl_ty) option

val apply_rules_with_index_value_errs :
  ?ignore_type_structure:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  (Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env
  * Typing_defs.locl_ty
  * (Typing_defs.locl_ty, Typing_defs.locl_ty * Typing_defs.locl_ty) result
  * (Typing_defs.locl_ty, Typing_defs.locl_ty * Typing_defs.locl_ty) result) ->
  Typing_env_types.env
  * Typing_defs.locl_ty
  * (Typing_defs.locl_ty * Typing_defs.locl_ty) option
  * (Typing_defs.locl_ty * Typing_defs.locl_ty) option
