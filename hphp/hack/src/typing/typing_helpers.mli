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
  }

  val make :
    ?coerce:Typing_logic.coercion_direction option ->
    Pos.t ->
    Typing_reason.ureason ->
    Typing_defs.locl_ty ->
    t

  val make_and_allow_coercion :
    Pos.t -> Typing_reason.ureason -> Typing_defs.locl_ty -> t

  val make_and_allow_coercion_opt :
    Typing_env_types.env ->
    Pos.t ->
    Typing_reason.ureason ->
    Typing_defs.locl_ty ->
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

(** Convert a function signature hint (method or toplevel function) into a bunch
  of decl_tys. *)
val hint_fun_decl :
  params:Nast.fun_param list ->
  ret:Nast.type_hint ->
  Typing_env_types.env ->
  Typing_defs.decl_ty option * Typing_defs.decl_ty option list

(** Stuff about type provenance? *)
module Prov : sig
  val update :
    Typing_defs.locl_ty ->
    f:(Typing_reason.t -> Typing_reason.t) ->
    env:Typing_env_types.env ->
    Typing_defs.locl_ty

  val flow : from:Typing_reason.t -> into:Typing_reason.t -> Typing_reason.t

  val rev : Typing_reason.t -> Typing_reason.t

  val prj_fn_arg :
    Typing_reason.t ->
    idx_sub:int ->
    idx_super:int ->
    var:Ast_defs.variance ->
    Typing_reason.t

  val prj_fn_ret : Typing_reason.t -> Typing_reason.t

  val prj_union_left : Typing_reason.t -> Typing_reason.t

  val prj_union_right : Typing_reason.t -> Typing_reason.t

  val prj_intersection_left : Typing_reason.t -> Typing_reason.t

  val prj_intersection_right : Typing_reason.t -> Typing_reason.t

  val prj_class :
    Typing_reason.t ->
    nm:string ->
    idx:int ->
    var:Ast_defs.variance ->
    Typing_reason.t

  val prj_newtype :
    Typing_reason.t ->
    nm:string ->
    idx:int ->
    var:Ast_defs.variance ->
    Typing_reason.t

  val prj_shape :
    Typing_reason.t ->
    fld_nm:string ->
    fld_kind_sub:Typing_reason.field_kind ->
    fld_kind_super:Typing_reason.field_kind ->
    Typing_reason.t
end

val refine_and_simplify_intersection :
  hint_first:bool ->
  Typing_env_types.env ->
  is_class:bool ->
  Typing_reason.t ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty
