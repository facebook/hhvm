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

(** Returns the possibly enforced return type along with some other information.
    The position parameter is used for error generation. *)
val make_info :
  Pos.t ->
  Ast_defs.fun_kind ->
  Nast.user_attribute list ->
  env ->
  is_explicit:bool ->
  Typing_defs.locl_ty ->
  Typing_defs.decl_ty option ->
  Typing_env_return_info.t

val implicit_return :
  env ->
  Ast_defs.pos ->
  expected:Typing_defs.locl_ty ->
  actual:Typing_defs.locl_ty ->
  hint_pos:Pos_or_decl.t option ->
  is_async:bool ->
  env

val make_return_type :
  (env -> Typing_defs.decl_ty -> env * Typing_defs.locl_ty) ->
  env ->
  Typing_defs.decl_ty ->
  env * Typing_defs.locl_ty

(** For async functions, strip Awaitable<_> from the return type *)
val strip_awaitable :
  Ast_defs.fun_kind ->
  env ->
  Typing_defs.locl_possibly_enforced_ty ->
  Typing_defs.locl_possibly_enforced_ty

val make_fresh_return_type : env -> Ast_defs.pos -> env * Typing_defs.locl_ty

(** Force the return type of a function to adhere to the fun_kind specified in
    the env *)
val force_return_kind :
  ?is_toplevel:bool ->
  env ->
  Ast_defs.pos ->
  Typing_defs.locl_ty ->
  env * Typing_defs.locl_ty

(** If there is no return type annotation on method, assume `void` for the
special functions `__construct`, otherwise we can assume type Tany *)
val make_default_return :
  is_method:bool -> env -> Ast_defs.pos * string -> Typing_defs.locl_ty

val fun_implicit_return :
  env -> Ast_defs.pos -> Typing_defs.locl_ty -> Ast_defs.fun_kind -> env

val check_inout_return : Ast_defs.pos -> env -> env
