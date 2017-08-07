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
open Typing_defs
open Core

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
       let env = TUtils.sub_type env ecstr_ty ty in
       TUtils.sub_type env ety cstr_ty
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
      ), true
  end

let add_check_where_constraint_todo (env_now:Env.env) pos ck cstr_ty ty =
  Env.add_todo env_now begin fun (env:Env.env) ->
    Errors.try_
      (fun () ->
        check_constraint env ck cstr_ty ty)
      (fun l ->
       Errors.explain_where_constraint env.Env.pos pos l;
       env
      ), true
  end

(*
  For where clauses containing type accesses, we can't just handle equality
  constraints the normal way. Given two unresolved types(which are exactly the
  type returned by generics when checking where constraints), unifying them will
  only cause the types to grow. Thus (int) will unify with (string). But this is
  unsound when it comes to generic type accesses, since type constants do not
  follow variance rules. Thus, the reasonable thing to do is to check whether
  all types within the unresolved types unify with each other.

  For example, for the constraint:
  function foo<T1 as Box, T2>(T1 $x) : T2 where T1::T = T2::T {
    $x->set($y->get());
  }
  We should expect the where clause to actually check if T1 and T2
  are equal, not unify them into the same type.
*)
let handle_eq_tconst_constraint env ck ty cstr_ty =
  (* First check that the bigger types work *)
  let env = check_constraint env ck ty cstr_ty in
  let env, ty = Env.expand_type env ty in
  let env, cstr_ty = Env.expand_type env cstr_ty in
  let rec flatten_unresolved_tys ty =
    match ty with
    | _, Tunresolved tyl ->
      List.fold tyl ~init: []
      ~f: (fun acc ty -> (flatten_unresolved_tys ty) @ acc)
    | _ ->
      [ty] in
  let tyl =
    (flatten_unresolved_tys) ty @ (flatten_unresolved_tys cstr_ty) in
  match tyl with
  (* If they are both unresolved and empty for some reason, it's fine *)
  | [] -> env
  | ty::tys ->
  List.fold tys ~init: env
  ~f: begin fun env ty_ ->
    check_constraint env ck ty ty_
  end

let add_check_tconst_where_constraint_todo
  (env_now:Env.env) pos ck ty_from_env cstr_ty ty =
  Env.add_todo env_now begin fun (env: Env.env) ->
    Errors.try_
      (fun () ->
        let env, ty = ty_from_env env ty in
        let env, cstr_ty = ty_from_env env cstr_ty in
        match ck with
        | Ast.Constraint_eq ->
          handle_eq_tconst_constraint env ck ty cstr_ty
        | _ ->
          check_constraint env ck ty cstr_ty
      )
      (fun l ->
        Errors.explain_tconst_where_constraint env.Env.pos pos l;
       env
      ), true
  end
