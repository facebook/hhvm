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
    ty: Typing_defs.locl_possibly_enforced_ty;
  }

  val make : Pos.t -> Typing_reason.ureason -> Typing_defs.locl_ty -> t

  val make_and_allow_coercion :
    Pos.t -> Typing_reason.ureason -> Typing_defs.locl_possibly_enforced_ty -> t

  val make_and_allow_coercion_opt :
    Typing_env_types.env ->
    Pos.t ->
    Typing_reason.ureason ->
    Typing_defs.locl_possibly_enforced_ty ->
    t option
end

val set_tyvars_variance_in_callable :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty list ->
  Tast.fun_variadicity ->
  Typing_env_types.env

val has_accept_disposable_attribute : ('a, 'b, 'c, 'd) Aast.fun_param -> bool

val all_record_fields :
  Typing_env_types.env ->
  Decl_provider.record_def_decl ->
  (Typing_defs.pos_id * Typing_defs.record_field_req) SMap.t

val add_decl_errors : Errors.t option -> unit

val with_timeout :
  Typing_env_types.env ->
  Pos.t * string ->
  do_:(Typing_env_types.env -> 'b) ->
  'b option

val reify_kind : Aast.reify_kind -> Aast.reify_kind
