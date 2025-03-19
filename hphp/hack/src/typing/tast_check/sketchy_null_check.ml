(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ast_defs
open Aast
open Typing_defs
module Env = Tast_env

let warning_kind = Typing_warning.Sketchy_null_check

let error_codes = Typing_warning_utils.codes warning_kind

let get_lvar_name = function
  | Lvar (_, id) -> Some (Local_id.get_name id)
  | _ -> None

let rec is_abstract_or_unknown env ty =
  let (_, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tany _
  | Tdynamic
  | Tvar _
  | Tgeneric _
  | Tnewtype _
  | Tdependent _ ->
    true
  | Tunion tyl -> List.exists tyl ~f:(is_abstract_or_unknown env)
  | Tintersection tyl -> List.for_all tyl ~f:(is_abstract_or_unknown env)
  | _ -> false

let sketchy_null_check env ~as_lint (ty, p, e) kind =
  if
    Env.is_sub_type_for_union env (Typing_make_type.null Reason.none) ty
    || is_abstract_or_unknown env ty
  then
    let name = get_lvar_name e in
    Tast_utils.(
      let (env, nonnull_ty) = Env.intersect_with_nonnull env (get_pos ty) ty in
      match truthiness env nonnull_ty with
      | Possibly_falsy ->
        Typing_warning_utils.add_for_migration
          (Env.get_tcopt env)
          ~as_lint:
            (if as_lint then
              Some None
            else
              None)
          ( p,
            warning_kind,
            {
              Typing_warning.Sketchy_null_check.name;
              kind;
              ty = Tast_env.print_ty env ty;
            } )
      | Always_truthy
      | Always_falsy
      | Unknown ->
        ())

let handler ~as_lint =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, _, x) =
      match x with
      | Eif (e, None, _) ->
        sketchy_null_check
          env
          ~as_lint
          e
          Typing_warning.Sketchy_null_check.Coalesce
      | Unop (Unot, e)
      | Binop { bop = Eqeq; lhs = (_, _, Null); rhs = e }
      | Binop { bop = Eqeq; lhs = e; rhs = (_, _, Null) } ->
        sketchy_null_check env ~as_lint e Typing_warning.Sketchy_null_check.Eq
      | Eif (e, Some _, _) ->
        sketchy_null_check env ~as_lint e Typing_warning.Sketchy_null_check.Neq
      | Binop { bop = Ampamp | Barbar; lhs = e1; rhs = e2 } ->
        sketchy_null_check env ~as_lint e1 Typing_warning.Sketchy_null_check.Neq;
        sketchy_null_check env ~as_lint e2 Typing_warning.Sketchy_null_check.Neq
      | _ -> ()

    method! at_stmt env x =
      match snd x with
      | If (e, _, _)
      | Do (_, e)
      | While (e, _)
      | For (_, Some e, _, _) ->
        sketchy_null_check env ~as_lint e Typing_warning.Sketchy_null_check.Neq
      | _ -> ()
  end
