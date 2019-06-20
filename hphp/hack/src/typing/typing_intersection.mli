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

val intersect_list : Env.env -> Reason.t -> locl ty list -> Env.env * locl ty
