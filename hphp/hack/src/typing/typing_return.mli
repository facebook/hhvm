(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Typing_env

(** Typing code concerned with return types. *)

val make_info:
  Ast_defs.fun_kind ->
  Nast.user_attribute list ->
  Env.env ->
  is_explicit:bool ->
  Typing_defs.locl Typing_defs.ty ->
  Typing_defs.decl Typing_defs.ty option -> Typing_env_return_info.t

val suggest_return:
  Env.env ->
  Ast_defs.pos ->
  Typing_defs.locl Typing_defs.ty ->
  (int -> bool) -> unit

val async_suggest_return:
  Ast_defs.fun_kind ->
  'a * Aast.hint_ ->
  Ast_defs.pos -> unit

val implicit_return:
  Env.env ->
  Ast_defs.pos ->
  expected:Typing_defs.locl Typing_defs.ty ->
  actual:Typing_defs.locl Typing_defs.ty -> Env.env

(** For async functions, wrap Awaitable<_> around the return type *)
val wrap_awaitable:
  Env.env ->
  Ast_defs.pos ->
  Typing_defs.locl Typing_defs.ty -> Typing_defs.locl Typing_defs.ty

val make_return_type:
  (Env.env -> Typing_defs.decl Typing_defs.ty -> Env.env * Typing_defs.locl Typing_defs.ty) ->
  Env.env ->
  Typing_defs.decl Typing_defs.ty -> Env.env * Typing_defs.locl Typing_defs.ty

(** For async functions, strip Awaitable<_> from the return type *)
val strip_awaitable:
  Ast_defs.fun_kind ->
  Env.env ->
  Typing_defs.locl Typing_defs.ty -> Typing_defs.locl Typing_defs.ty

val strip_awaitable_decl:
  Env.env ->
  Typing_defs.decl Typing_defs.ty -> Typing_defs.decl Typing_defs.ty

val force_awaitable:
  Env.env ->
  Ast_defs.pos ->
  Typing_defs.locl Typing_defs.ty ->
  Env.env * Typing_defs.locl Typing_defs.ty

(** If there is no return type annotation on method, assume `void` for the
special function `__construct`, otherwise Tany *)
val make_default_return:
  Env.env ->
  Ast_defs.pos * string -> Typing_reason.t * 'a Typing_defs.ty_
