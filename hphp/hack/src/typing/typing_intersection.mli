(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types
module Reason = Typing_reason
module Utils = Typing_utils

val negate_type :
  env -> Reason.t -> locl_ty -> approx:Utils.approx -> env * locl_ty

val intersect : env -> r:Reason.t -> locl_ty -> locl_ty -> env * locl_ty

val intersect_list : env -> Reason.t -> locl_ty list -> env * locl_ty

val simplify_intersections :
  env ->
  ?on_tyvar:(env -> Reason.t -> Tvid.t -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

val intersect_i :
  env -> Typing_reason.t -> internal_type -> locl_ty -> env * internal_type

val destruct_inter_list :
  env -> locl_ty list -> env * (locl_ty list * Reason.t option)
