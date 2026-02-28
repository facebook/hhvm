(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A program analysis to find locations of upcasts to dynamic type *)

open Refactor_sd_types

exception Refactor_sd_exn of string

val do_ : string -> options -> Provider_context.t -> Tast.program -> unit

val simplify :
  Typing_env_types.env -> constraint_ list -> refactor_sd_result list

val callable :
  element_info ->
  Tast_env.t ->
  Tast.fun_param list ->
  Tast.func_body ->
  constraint_ list

(** Relationship with shape_analysis: is_shape_like_dict *)
val contains_upcast : refactor_sd_result -> bool

(** Relationship with shape_analysis: show_shape_result *)
val show_refactor_sd_result :
  Typing_env_types.env -> refactor_sd_result -> string
