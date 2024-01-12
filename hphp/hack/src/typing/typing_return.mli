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
  Pos.t ->
  Ast_defs.fun_kind ->
  Nast.user_attribute list ->
  env ->
  Typing_defs.locl_ty ->
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
  ety_env:Typing_defs.expand_env ->
  this_class:Decl_provider.Class.t option ->
  ?is_toplevel:bool ->
  (* Wrap return type with supportdyn, used when checking SDT functions and methods *)
  supportdyn:bool ->
  env ->
  (* Position of return type hint, or function name, if absent *)
  hint_pos:Ast_defs.pos ->
  (* Explicit type from source code *)
  explicit:Typing_defs.decl_ty option ->
  (* A type to use if the explicit type isn't present.
   * e.g. implicit void (for constructors), or contextual type (for lambdas)
   *)
  default:Typing_defs.locl_ty option ->
  env * Typing_defs.locl_ty

(** For async functions, strip Awaitable<_> from the return type *)
val strip_awaitable :
  Ast_defs.fun_kind -> env -> Typing_defs.locl_ty -> Typing_defs.locl_ty

val fun_implicit_return :
  env -> Ast_defs.pos -> Typing_defs.locl_ty -> Ast_defs.fun_kind -> env

val check_inout_return : Ast_defs.pos -> env -> env
