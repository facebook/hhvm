(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module T = Tast

val handle_assignment_mutability :
  Typing_env.env ->
  T.expr ->
  T.expr_ option ->
  Typing_env.env
val freeze_local : Pos.t -> Typing_env.env -> T.expr list -> Typing_env.env
val move_local : Pos.t -> Typing_env.env -> T.expr list -> Typing_env.env
val check_rx_mutable_arguments : Pos.t -> Typing_env.env -> T.expr list -> unit
val enforce_mutable_call : Typing_env.env -> T.expr -> unit
val enforce_mutable_constructor_call:
  Typing_env.env -> 'a Typing_defs.ty -> T.expr list -> unit
val handle_value_in_return:
  function_returns_mutable: bool ->
  function_returns_void_for_rx: bool ->
  Typing_env.env ->
  Pos.t ->
  T.expr ->
  Typing_env.env
val check_unset_target: Typing_env.env -> T.expr -> unit
val check_conditional_operator:
  T.expr ->
  T.expr ->
  unit
