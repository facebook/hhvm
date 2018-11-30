(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module T = Tast

module type Env_S = sig
  type env
  val env_reactivity: env -> Typing_defs.reactivity
  val get_fun: env -> Typing_heap.Funs.key -> Typing_heap.Funs.t option
end

module Shared(Env: Env_S): sig
  val is_fun_call_returning_mutable: Env.env -> T.expr -> bool
end

val handle_assignment_mutability :
  Typing_env.env ->
  T.expr ->
  T.expr_ option ->
  Typing_env.env
val freeze_local : Pos.t -> Typing_env.env -> T.expr list -> Typing_env.env
val move_local : Pos.t -> Typing_env.env -> T.expr list -> Typing_env.env
val handle_value_in_return:
  function_returns_mutable: bool ->
  function_returns_void_for_rx: bool ->
  Typing_env.env ->
  Pos.t ->
  T.expr ->
  Typing_env.env
val is_move_or_mutable_call: ?allow_move: bool -> T.expr_ -> bool
