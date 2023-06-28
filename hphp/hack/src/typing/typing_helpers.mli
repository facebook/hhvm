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
    coerce: Typing_logic.coercion_direction option;
  }

  val make :
    ?coerce:Typing_logic.coercion_direction option ->
    Pos.t ->
    Typing_reason.ureason ->
    Typing_defs.locl_ty ->
    t

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

(** During the decl phase we can, for global inference, add "improved type hints".
   That is we can say that some missing type hints are in fact global tyvars.
   In that case to get the real type hint we must merge the type hint present
   in the ast with the one we created during the decl phase. This function does
   exactly this for the return type, the parameters and the variadic parameters.
  *)
val merge_decl_header_with_hints :
  params:Nast.fun_param list ->
  ret:Nast.type_hint ->
  Typing_env_types.env ->
  Typing_defs.decl_ty option * Typing_defs.decl_ty option list
