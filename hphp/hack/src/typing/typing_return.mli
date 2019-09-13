(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_env_types

(** Typing code concerned with return types. *)

val make_info :
  Ast_defs.fun_kind ->
  Nast.user_attribute list ->
  env ->
  is_explicit:bool ->
  Typing_defs.locl Typing_defs.ty ->
  Typing_defs.decl Typing_defs.ty option ->
  Typing_env_return_info.t

val async_suggest_return :
  Ast_defs.fun_kind -> 'a * Aast.hint_ -> Ast_defs.pos -> unit

val implicit_return :
  env ->
  Ast_defs.pos ->
  expected:Typing_defs.locl Typing_defs.ty ->
  actual:Typing_defs.locl Typing_defs.ty ->
  env

val wrap_awaitable :
  env ->
  Ast_defs.pos ->
  Typing_defs.locl Typing_defs.ty ->
  Typing_defs.locl Typing_defs.ty
(** For async functions, wrap Awaitable<_> around the return type *)

val make_return_type :
  (env ->
  Typing_defs.decl Typing_defs.ty ->
  env * Typing_defs.locl Typing_defs.ty) ->
  env ->
  Typing_defs.decl Typing_defs.ty ->
  env * Typing_defs.locl Typing_defs.ty

val strip_awaitable :
  Ast_defs.fun_kind ->
  env ->
  Typing_defs.locl Typing_defs.possibly_enforced_ty ->
  Typing_defs.locl Typing_defs.possibly_enforced_ty
(** For async functions, strip Awaitable<_> from the return type *)

val force_awaitable :
  env ->
  Ast_defs.pos ->
  Typing_defs.locl Typing_defs.ty ->
  env * Typing_defs.locl Typing_defs.ty

val make_default_return :
  is_method:bool ->
  is_infer_missing_on:bool ->
  env ->
  Ast_defs.pos * string ->
  env * (Typing_reason.t * Typing_defs.locl Typing_defs.ty_)
(** If there is no return type annotation on method, assume `void` for the
special functions `__construct`, otherwise we can either
introduce a new fresh variable when infer missing is on or assume type Tany *)
