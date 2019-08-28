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

val non : env -> Reason.t -> locl ty -> approx:Utils.approx-> env * locl ty
val intersect : env -> r:Reason.t -> locl ty -> locl ty -> env * locl ty
val intersect_list : env -> Reason.t -> locl ty list -> env * locl ty
val simplify_intersections : env -> ?on_tyvar:(env -> Reason.t -> int -> env * locl ty)
  -> locl ty -> env * locl ty
