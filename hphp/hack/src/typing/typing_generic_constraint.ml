(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Typing_defs

module TUtils = Typing_utils
module N = Nast
module Reason = Typing_reason
module Env = Typing_env

(* Given a list of generic parameter declarations of type [locl tparam list]
 * add entries for each generic parameter to env.lenv.tpenv, with lower
 * bounds for `super` constraints, upper bounds for `as` constraints, and
 * both bounds for `eq` constraints. *)
let add_tparams_bounds env (tparams: locl tparam list) =
  let add_bound env (_, (_, name), cstrl) =
    List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
      Env.add_constraint env name ck ty) in
  List.fold_left tparams ~f:add_bound ~init: env

let check_constraint env ck cstr_ty ty =
  let env, ety = Env.expand_type env ty in
  let env, ecstr_ty = Env.expand_type env cstr_ty in
  match snd ecstr_ty, snd ety with
  | _, Tany ->
      (* This branch is only reached when we have an unbound type variable,
       * when this is the case, the constraint should always succeed.
       *)
      env
  | Tany, _ -> fst (TUtils.unify env cstr_ty ty)
  | (Tmixed | Tarraykind _ | Tprim _ | Toption _ | Tvar _
    | Tabstract (_, _) | Tclass (_, _) | Ttuple _ | Tanon (_, _) | Tfun _
    | Tunresolved _ | Tobject | Tshape _
    ), _ -> begin
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
      end

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
