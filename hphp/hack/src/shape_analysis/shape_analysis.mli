(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A program analysis to find shape like dicts and the static keys used in
    these dicts. *)

open Shape_analysis_types

exception Shape_analysis_exn of Error.t

val is_shape_like_dict : shape_result -> bool

val simplify : Typing_env_types.env -> constraint_ list -> shape_result list

val callable :
  mode ->
  A.id_ ->
  Tast_env.t ->
  Tast.fun_param list ->
  return:Tast.type_hint ->
  Tast.func_body ->
  decorated_constraints * Error.t list

val do_ : options -> Provider_context.t -> Tast.program -> unit

val show_shape_result : Typing_env_types.env -> shape_result -> string

val shape_results_using_hips : solve_entries

val shape_results_no_hips : solve_entries

val any_constraints_of_decorated_constraints :
  decorated_constraints ->
  (constraint_, inter_constraint_) HT.any_constraint_ list
