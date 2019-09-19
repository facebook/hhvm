(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module TUtils = Typing_utils
module Reason = Typing_reason
module Env = Typing_env
open Typing_defs
open Typing_env_types

let check_constraint env ck ty ~cstr_ty =
  Typing_log.(
    log_with_level env "sub" 1 (fun () ->
        log_types
          (Reason.to_pos (fst ty))
          env
          [
            Log_head
              ( "Typing_generic_constraint.check_constraint",
                [Log_type ("ty", ty); Log_type ("cstr_ty", cstr_ty)] );
          ]));
  if not (Env.is_consistent env) then
    env
  else
    let (env, ety) = Env.expand_type env ty in
    let (env, ecstr_ty) = Env.expand_type env cstr_ty in
    (* using unify error here since the error itself is (almost) always superseded
     * by an Errors.try_ fallback using explain_constraint *)
    match ck with
    | Ast_defs.Constraint_as ->
      (* If ty is a Tvar, we don't want to unify that Tvar with
       * cstr_ty; we merely want the type itself to be added to
       * cstr_ty's list of unresolved types. Thus we pass the
       * expanded type. *)
      TUtils.sub_type env ety cstr_ty Errors.unify_error
    | Ast_defs.Constraint_eq ->
      (* An equality constraint is the same as two commuting `as`
       * constraints, i.e. X=Y is { X as Y, Y as X }. Thus, add
       * add both expansions to the environment. We don't expand
       * both sides of the equation simultaniously, to preserve an
       * easier convergence indication. *)
      let env = TUtils.sub_type env ecstr_ty ty Errors.unify_error in
      TUtils.sub_type env ety cstr_ty Errors.unify_error
    | Ast_defs.Constraint_super ->
      (* If cstr_ty is a Tvar, we don't want to unify that Tvar with
       * ty; we merely want the constraint itself to be added to the
       * ty's list of unresolved types. Thus we pass the expanded
       * constraint type. *)
      TUtils.sub_type env ecstr_ty ty Errors.unify_error

let add_check_constraint_todo (env : env) ~use_pos (pos, name) ck cstr_ty ty =
  Errors.try_
    (fun () -> check_constraint env ck ty ~cstr_ty)
    (fun l ->
      Errors.explain_constraint ~use_pos ~definition_pos:pos ~param_name:name l;
      env)

let add_check_where_constraint_todo
    ~in_class (env : env) ~use_pos ~definition_pos ck cstr_ty ty =
  Errors.try_
    (fun () -> check_constraint env ck ty ~cstr_ty)
    (fun l ->
      Errors.explain_where_constraint ~in_class ~use_pos ~definition_pos l;
      env)

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
  let env = check_constraint env ck ty ~cstr_ty in
  let (env, ty) = Env.expand_type env ty in
  let (env, cstr_ty) = Env.expand_type env cstr_ty in
  let rec flatten_unresolved_tys ty =
    match ty with
    | (_, Tunion tyl) ->
      List.fold tyl ~init:[] ~f:(fun acc ty -> flatten_unresolved_tys ty @ acc)
    | _ -> [ty]
  in
  let tyl = flatten_unresolved_tys ty @ flatten_unresolved_tys cstr_ty in
  match tyl with
  (* If they are both unresolved and empty for some reason, it's fine *)
  | [] -> env
  | ty :: tys ->
    List.fold tys ~init:env ~f:(fun env ty_ ->
        check_constraint env ck ty_ ~cstr_ty:ty)

let add_check_tconst_where_constraint_todo
    (env : env) ~use_pos ~definition_pos ck ty_from_env cstr_ty ty =
  Errors.try_
    (fun () ->
      let (env, ty) = ty_from_env env ty in
      let (env, cstr_ty) = ty_from_env env cstr_ty in
      match ck with
      | Ast_defs.Constraint_eq -> handle_eq_tconst_constraint env ck ty cstr_ty
      | _ -> check_constraint env ck ty ~cstr_ty)
    (fun l ->
      Errors.explain_tconst_where_constraint ~use_pos ~definition_pos l;
      env)
