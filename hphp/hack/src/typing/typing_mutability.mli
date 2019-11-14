(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type Env_S = sig
  type env

  val env_reactivity : env -> Typing_defs.reactivity

  val get_fun : env -> Decl_provider.fun_key -> Decl_provider.fun_decl option
end

module Shared (Env : Env_S) : sig
  val is_fun_call_returning_mutable : Env.env -> Tast.expr -> bool
end

val handle_assignment_mutability :
  Typing_env_types.env -> Tast.expr -> Tast.expr_ option -> Typing_env_types.env

val freeze_local :
  Pos.t -> Typing_env_types.env -> Tast.expr list -> Typing_env_types.env

val move_local :
  Pos.t -> Typing_env_types.env -> Tast.expr list -> Typing_env_types.env

val handle_value_in_return :
  function_returns_mutable:bool ->
  function_returns_void_for_rx:bool ->
  Typing_env_types.env ->
  Pos.t ->
  Tast.expr ->
  Typing_env_types.env

val is_move_or_mutable_call : ?allow_move:bool -> Tast.expr_ -> bool
