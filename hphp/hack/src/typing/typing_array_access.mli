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

val pessimise_type :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val pessimised_tup_assign :
  pos ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

(* Typing of array-get like expressions; [ty1] is the type of the expression
   into which we are indexing (the 'collection'), [e2] is the index expression
   and [ty2] is the type of that expression.

   We return:
   1) the (modified) typing environment,
   2) the type of the resulting expression (i.e. the type of the element we are 'getting')
   3) the actual and expected type of the indexed expression, indicating a type mismatch (if any)
   4) the actual and expected type of the indexing expression, indicating a type mismatch (if any)
   and an optional type mismatch giving the actual vs expected type of the

   The function has an error side-effect
*)
val array_get :
  array_pos:pos ->
  expr_pos:pos ->
  expr_ty:locl_ty ->
  ?lhs_of_null_coalesce:is_variadic ->
  ?ignore_error:bool ->
  is_variadic ->
  Typing_env_types.env ->
  locl_ty ->
  ('a, 'b) Aast.expr ->
  locl_ty ->
  Typing_env_types.env
  * (locl_ty * (locl_ty * locl_ty) option * (locl_ty * locl_ty) option)

(* Check an array append expression returning the modified typing environment,
   the resulting type of the lhs expression, an optional subtyping errors for
   the lhs expression and an optional subtyping error for the expression to be
   appended *)
val assign_array_append :
  array_pos:pos ->
  expr_pos:pos ->
  Reason.ureason ->
  Typing_env_types.env ->
  locl_ty ->
  locl_ty ->
  Typing_env_types.env
  * (locl_ty * (locl_ty * locl_ty) option * (locl_ty * locl_ty) option)

(* Check an array append expression returning the modified typing environment,
   the resulting type of the lhs expression, an optional subtyping errors for
   the indexed expression, an optional subtyping error for the indexing expression
   , and an optional subtyping error for the rhs expression *)
val assign_array_get :
  array_pos:pos ->
  expr_pos:pos ->
  Reason.ureason ->
  Typing_env_types.env ->
  locl_ty ->
  Nast.expr ->
  locl_ty ->
  locl_ty ->
  Typing_env_types.env
  * (locl_ty
    * (locl_ty * locl_ty) option
    * (locl_ty * locl_ty) option
    * (locl_ty * locl_ty) option)
