(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module ExpectedTy : sig
  type t = private {
    pos: Pos.t;
    reason: Typing_reason.ureason;
    ty: Typing_defs.locl_ty;
    coerce: Typing_logic.coercion_direction option;
    ignore_readonly: bool;
  }

  val make :
    ?coerce:Typing_logic.coercion_direction option ->
    ?ignore_readonly:bool ->
    Pos.t ->
    Typing_reason.ureason ->
    Typing_defs.locl_ty ->
    t
end

val set_tyvars_variance_in_callable :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty option list ->
  Typing_env_types.env

val has_accept_disposable_attribute : ('a, 'b) Aast.fun_param -> bool

val add_decl_errors :
  env:Typing_env_types.env -> Decl_defs.decl_error list -> unit

val with_timeout :
  Typing_env_types.env ->
  Pos.t * string ->
  (Typing_env_types.env -> 'b) ->
  'b option

val reify_kind : Aast.reify_kind -> Aast.reify_kind

(** Convert a function signature hint (method or toplevel function) into a bunch
  of decl_tys. *)
val hint_fun_decl :
  params:Nast.fun_param list ->
  ret:Nast.type_hint ->
  Typing_env_types.env ->
  Typing_defs.decl_ty option * Typing_defs.decl_ty option list

val refine_and_simplify_intersection :
  hint_first:bool ->
  Typing_env_types.env ->
  is_class:bool ->
  Typing_reason.t ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val make_simplify_typed_expr :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty ->
  (Typing_defs.locl_ty, Tast.saved_env) Aast_defs.expr_ ->
  Typing_env_types.env * Tast.expr
