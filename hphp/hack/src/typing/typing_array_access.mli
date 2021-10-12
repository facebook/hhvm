(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Typing_defs

val maybe_pessimise_type :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

(* Check an array get expression returning the modified typing environment,
   the accessed type, an optional subtyping errors for the indexed expression
   and an optional subtyping error for the indexing expression *)
val array_get :
  array_pos:pos ->
  expr_pos:pos ->
  ?lhs_of_null_coalesce:is_variadic ->
  is_variadic ->
  Typing_env_types.env ->
  locl_ty ->
  Nast.expr ->
  locl_ty ->
  Typing_env_types.env
  * locl_ty
  * (locl_ty * locl_ty) option
  * (locl_ty * locl_ty) option

(* Check an array append expression returning the modified typing environment,
   the resulting type of the lhs expression, an optional subtyping errors for
   the lhs expression and an optional subtyping error for the expression to be
   appended *)
val assign_array_append_with_err :
  array_pos:pos ->
  expr_pos:pos ->
  Reason.ureason ->
  Typing_env_types.env ->
  locl_ty ->
  locl_ty ->
  Typing_env_types.env
  * locl_ty
  * (locl_ty * locl_ty) option
  * (locl_ty * locl_ty) option

(* Check an array append expression returning the modified typing environment,
   the resulting type of the lhs expression, an optional subtyping errors for
   the indexed expression, an optional subtyping error for the indexing expression
   , and an optional subtyping error for the rhs expression *)
val assign_array_get_with_err :
  array_pos:pos ->
  expr_pos:pos ->
  Reason.ureason ->
  Typing_env_types.env ->
  locl_ty ->
  Nast.expr ->
  locl_ty ->
  locl_ty ->
  Typing_env_types.env
  * locl_ty
  * (locl_ty * locl_ty) option
  * (locl_ty * locl_ty) option
  * (locl_ty * locl_ty) option
