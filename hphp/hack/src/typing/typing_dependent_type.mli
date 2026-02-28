(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module ExprDepTy : sig
  type dep =
    | Dep_This
    | Dep_Cls of string
    | Dep_Expr of Expression_id.t

  val from_cid :
    Typing_env_types.env ->
    Typing_reason.t ->
    Nast.class_id_ ->
    Typing_reason.t * dep

  val make :
    Typing_env_types.env ->
    cid:Nast.class_id_ ->
    Typing_defs.locl_ty ->
    Typing_env_types.env * Typing_defs.locl_ty

  val make_with_dep_kind :
    Typing_env_types.env ->
    Typing_reason.t * dep ->
    Typing_defs.locl_ty ->
    Typing_env_types.env * Typing_defs.locl_ty
end
