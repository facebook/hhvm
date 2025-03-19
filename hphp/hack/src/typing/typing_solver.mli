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
val is_sub_type : env -> locl_ty -> locl_ty -> bool * Typing_error.t option

(* Force solve all remaining unsolved type variables *)
val solve_all_unsolved_tyvars : env -> env * Typing_error.t option

val expand_type_and_solve :
  env ->
  ?default:locl_ty ->
  ?freshen:bool ->
  description_of_expected:string ->
  Pos.t ->
  locl_ty ->
  (env * Typing_error.t option) * locl_ty

val expand_type_and_solve_eq :
  env -> locl_ty -> (env * Typing_error.t option) * locl_ty

(** Expand an already-solved type variable to its solution.
    If the type variable is not yet solved, but one of its lower bounds is dynamic,
    i.e. dynamic, t1, ..., tn <: #0 <: u1, ..., um,
    then solve the type variable to #0 := ~#1 for fresh type variable #1, where
    t1, ..., tn <: #1 <: u1, ..., um.
    This is used in the strip_dynamic functions in Typing_dynamic_utils before matching the type on ~t
  *)
val expand_type_for_strip_dynamic : env -> locl_ty -> env * locl_ty

val expand_type_and_narrow :
  env ->
  ?default:locl_ty ->
  ?allow_nothing:bool ->
  ?force_solve:bool ->
  description_of_expected:string ->
  (env -> locl_ty -> env * locl_ty option) ->
  Pos.t ->
  locl_ty ->
  (env * Typing_error.t option) * locl_ty

val solve_to_equal_bound_or_wrt_variance :
  env -> Reason.t -> Tvid.t -> env * Typing_error.t option

val close_tyvars_and_solve : env -> env * Typing_error.t option

val bind : env -> Tvid.t -> locl_ty -> env * Typing_error.t option

val try_bind_to_equal_bound : env -> Tvid.t -> env * Typing_error.t option
