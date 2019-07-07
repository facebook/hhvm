(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module Env = Typing_env
module Reason = Typing_reason
module Utils = Typing_utils

val non : Env.env -> Reason.t -> locl ty -> approx:Utils.approx-> Env.env * locl ty
val intersect : Env.env -> r:Reason.t -> locl ty -> locl ty -> Env.env * locl ty
val intersect_list : Env.env -> Reason.t -> locl ty list -> Env.env * locl ty
val simplify_intersections : Env.env -> ?on_tyvar:(Env.env -> Reason.t -> int -> Env.env * locl ty)
  -> locl ty -> Env.env * locl ty
