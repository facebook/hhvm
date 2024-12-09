(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module TUtils = Typing_utils
module Env = Typing_env
open Typing_defs
open Typing_env_types

let check_constraint
    env ck ty ~cstr_ty (on_error : Typing_error.Reasons_callback.t option) =
  Typing_log.(
    log_with_level env "sub" ~level:1 (fun () ->
        log_types
          (get_pos ty)
          env
          [
            Log_head
              ( "Typing_generic_constraint.check_constraint",
                [Log_type ("ty", ty); Log_type ("cstr_ty", cstr_ty)] );
          ]));
  if not (Env.is_consistent env) then
    (env, None)
  else
    let (env, ety) = Env.expand_type env ty in
    let (env, ecstr_ty) = Env.expand_type env cstr_ty in
    match ck with
    | Ast_defs.Constraint_as ->
      TUtils.sub_type ~class_sub_classname:false env ety cstr_ty on_error
    | Ast_defs.Constraint_eq ->
      (* An equality constraint is the same as two commuting `as`
       * constraints, i.e. X=Y is { X as Y, Y as X }. Thus, add
       * add both expansions to the environment. We don't expand
       * both sides of the equation simultaniously, to preserve an
       * easier convergence indication. *)
      let (env, e1) =
        TUtils.sub_type ~class_sub_classname:false env ecstr_ty ty on_error
      in
      let (env, e2) =
        TUtils.sub_type ~class_sub_classname:false env ety cstr_ty on_error
      in
      (env, Option.merge e1 e2 ~f:Typing_error.both)
    | Ast_defs.Constraint_super ->
      TUtils.sub_type ~class_sub_classname:false env ecstr_ty ty on_error

let check_tparams_constraint (env : env) ~use_pos ck ~cstr_ty ty =
  check_constraint env ck ty ~cstr_ty
  @@ Some (Typing_error.Reasons_callback.explain_constraint use_pos)

let check_where_constraint
    ~in_class (env : env) ~use_pos ~definition_pos ck ~cstr_ty ty =
  check_constraint env ck ty ~cstr_ty
  @@ Some
       (Typing_error.Reasons_callback.explain_where_constraint
          use_pos
          ~in_class
          ~decl_pos:definition_pos)
