(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module T = Tast

val handle_assignment_mutability : Typing_env.env -> T.expr -> T.expr -> Typing_env.env
val expr_is_mutable : Typing_env.env -> T.expr -> bool
val freeze_local : Pos.t -> Typing_env.env -> T.expr list -> Typing_env.env
val enforce_mutable_call : Typing_env.env -> T.expr -> unit
