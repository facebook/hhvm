(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Typing_env
open Typing_defs
open Typing_env_types

(* Non-side-effecting test for subtypes.
    result = true implies ty1 <: ty2
    result = false implies NOT ty1 <: ty2 OR we don't know
*)
val is_sub_type : env -> locl_ty -> locl_ty -> bool

val non_null : env -> Pos_or_decl.t -> locl_ty -> env * locl_ty

(* Force solve all remaining unsolved type variables *)
val solve_all_unsolved_tyvars : env -> env

val expand_type_and_solve :
  env ->
  ?freshen:bool ->
  description_of_expected:string ->
  Pos.t ->
  locl_ty ->
  env * locl_ty

val expand_type_and_solve_eq : env -> locl_ty -> env * locl_ty

val expand_type_and_narrow :
  env ->
  ?default:locl_ty ->
  description_of_expected:string ->
  (env -> locl_ty -> env * locl_ty option) ->
  Pos.t ->
  locl_ty ->
  env * locl_ty

val solve_to_equal_bound_or_wrt_variance : env -> Reason.t -> int -> env

val close_tyvars_and_solve : env -> env

val solve_all_unsolved_tyvars_gi : env -> env

val bind : env -> Ident.t -> locl_ty -> env

val try_bind_to_equal_bound : env -> Ident.t -> env
