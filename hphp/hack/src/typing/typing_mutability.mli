(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module T = Tast

val handle_assignment_mutability : Typing_env.env -> T.expr -> T.expr -> Typing_env.env
val freeze_local : Pos.t -> Typing_env.env -> T.expr list -> Typing_env.env
val enforce_mutable_call : Typing_env.env -> T.expr -> unit
val check_function_return_value:
  function_returns_mutable: bool -> Typing_env.env -> Pos.t -> T.expr -> unit
