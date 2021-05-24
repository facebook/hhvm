(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Typing_defs

val array_get :
  array_pos:pos ->
  expr_pos:pos ->
  ?lhs_of_null_coalesce:is_variadic ->
  is_variadic ->
  Typing_env_types.env ->
  locl_ty ->
  pos * Nast.expr_ ->
  locl_ty ->
  Typing_env_types.env * locl_ty * (locl_ty * locl_ty) option

val assign_array_append_with_err :
  array_pos:pos ->
  expr_pos:pos ->
  Reason.ureason ->
  Typing_env_types.env ->
  locl_ty ->
  locl_ty ->
  Typing_env_types.env * locl_ty * (locl_ty * locl_ty) option

val assign_array_get_with_err :
  array_pos:pos ->
  expr_pos:pos ->
  Reason.ureason ->
  Typing_env_types.env ->
  locl_ty ->
  pos * (pos, Nast.func_body_ann, unit, unit) expr_ ->
  locl_ty ->
  locl_ty ->
  Typing_env_types.env
  * locl_ty
  * (locl_ty * locl_ty) option
  * (locl_ty * locl_ty) option
