(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module TUtils = Typing_utils
module N = Nast
module Reason = Typing_reason
module Env = Typing_env

let check_constraint env ck cstr_ty ty =
  let env, ety = Env.expand_type env ty in
  let env, ecstr_ty = Env.expand_type env cstr_ty in
  match ck with
  | Ast.Constraint_as ->
      (* If ty is a Tvar, we don't want to unify that Tvar with
       * cstr_ty; we merely want the type itself to be added to
       * cstr_ty's list of unresolved types. Thus we pass the
       * expanded type. *)
      TUtils.sub_type env ety cstr_ty
  | Ast.Constraint_eq ->
      (* An equality constraint is the same as two commuting `as`
       * constraints, i.e. X=Y is { X as Y, Y as X }. Thus, add
       * add both expansions to the environment. We don't expand
       * both sides of the equation simultaniously, to preserve an
       * easier convergence indication. *)
      TUtils.sub_type (TUtils.sub_type env ecstr_ty ty) ety cstr_ty
  | Ast.Constraint_super ->
      (* If cstr_ty is a Tvar, we don't want to unify that Tvar with
       * ty; we merely want the constraint itself to be added to the
       * ty's list of unresolved types. Thus we pass the expanded
       * constraint type. *)
      TUtils.sub_type env ecstr_ty ty

let add_check_constraint_todo (env_now:Env.env) reason generic ck cstr_ty ty =
  Env.add_todo env_now begin fun (env:Env.env) ->
    Errors.try_
      (fun () ->
        check_constraint env ck cstr_ty ty)
      (fun l ->
       Reason.explain_generic_constraint env.Env.pos reason generic l;
       env
      )
  end

let add_check_where_constraint_todo (env_now:Env.env) pos ck cstr_ty ty =
  Env.add_todo env_now begin fun (env:Env.env) ->
    Errors.try_
      (fun () ->
        check_constraint env ck cstr_ty ty)
      (fun l ->
       Errors.explain_where_constraint env.Env.pos pos l;
       env
      )
  end
