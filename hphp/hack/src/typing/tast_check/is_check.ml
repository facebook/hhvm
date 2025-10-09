(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module Env = Tast_env
module T = Typing_defs

let warning_kind = Typing_warning.Is_as_always

let error_codes = Typing_warning_utils.codes warning_kind

let nothing_ty = Typing_make_type.nothing Typing_reason.none

(* To handle typechecking against placeholder, e.g., `... as C<_>`, we convert
   the generic the placeholder is elaborated to into a type variable so that
   the subtyping query can be discharged. *)
let replace_placeholders_with_tvars env pos ty =
  let replace_placeholder env ty =
    match T.get_node ty with
    | T.Tgeneric name when String.contains name '#' ->
      Typing_env.fresh_type env pos
    | _ -> (env, ty)
  in
  match T.get_node ty with
  | T.Tclass (id, exact, targs) ->
    let (env, targs) = List.fold_map ~f:replace_placeholder ~init:env targs in
    (env, T.mk (Typing_reason.none, T.Tclass (id, exact, targs)))
  | _ -> (env, ty)

let is_like_null env ty =
  Tast_env.is_sub_type
    env
    ty
    (Typing_make_type.locl_like
       Typing_reason.none
       (Typing_make_type.null Typing_reason.none))

let is_as_warning ~as_lint kind pos lhs_ty rhs_ty env =
  Typing_warning_utils.add_for_migration
    (Env.get_tcopt env)
    ~as_lint
    ( pos,
      Typing_warning.Is_as_always,
      {
        Typing_warning.Is_as_always.kind;
        lhs_ty = Env.print_ty env lhs_ty;
        rhs_ty = Env.print_ty env rhs_ty;
      } )

let coalesce_warning kind pos lhs_ty _rhs_ty env =
  Env.add_warning
    env
    Typing_warning.
      ( pos,
        Null_coalesce_always,
        {
          Null_coalesce_always.kind;
          lhs_pos = Typing_reason.to_pos (Typing_defs.get_reason lhs_ty);
          lhs_ty = Env.print_ty env lhs_ty;
        } )

let trivial_check
    pos
    (env : Env.env)
    lhs_ty
    rhs_ty
    ~(always :
       Pos.t -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> Env.env -> unit)
    ~(never :
       Pos.t -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> Env.env -> unit) :
    unit =
  if Env.is_sub_type env lhs_ty nothing_ty then
    (* If we have a nothing in our hands, there was a bigger problem
       originating from earlier in the program. Don't flag it here, as it is
       merely a symptom. *)
    ()
  else
    let lhs_ty =
      (* Don't strip the like if LHS is ~null because this commonly leads to
         false positives. *)
      if is_like_null env lhs_ty then
        lhs_ty
      else
        Env.strip_dynamic env lhs_ty
    in
    let (env, lhs_ty) = Env.expand_type env lhs_ty in
    match Env.is_disjoint ~is_dynamic_call:false env lhs_ty rhs_ty with
    | Env.Disjoint -> never pos lhs_ty rhs_ty env
    | DisjointIgnoringDynamic (lhs_ty, rhs_ty) -> never pos lhs_ty rhs_ty env
    | NonDisjoint ->
      (* We can't just use the `is_subtype` API which will discharge the
         propositions with the fresh type variables. Instead, we use `sub_type`
         which feedback the propositions against unconstrained type variables as
         assumptions. *)
      let Equal = Tast_env.eq_typing_env in
      let ((env, _), lhs_ty) =
        Typing_solver.expand_type_and_solve
          env
          ~description_of_expected:"a value"
          Pos.none
          lhs_ty
      in
      let env = Typing_env.open_tyvars env pos in
      let (env, rhs_ty) = replace_placeholders_with_tvars env pos rhs_ty in
      let callback = Typing_error.Reasons_callback.unify_error_at pos in
      let (env, err_opt1) =
        Typing_utils.sub_type env lhs_ty rhs_ty (Some callback)
      in
      let (env, err_opt2) = Typing_solver.close_tyvars_and_solve env in
      let err_opt = Option.merge err_opt1 err_opt2 ~f:Typing_error.both in
      if Option.is_none err_opt then always pos lhs_ty rhs_ty env

let handler ~as_lint =
  object
    inherit Tast_visitor.handler_base

    method! at_expr (env : Env.env) expr =
      let as_lint =
        if as_lint then
          Some (Some (Env.get_check_status env))
        else
          None
      in
      match expr with
      | (_, p, Is ((lhs_ty, _, _), hint)) ->
        let (env, hint_ty) = Env.localize_hint_for_refinement env hint in
        let open Typing_warning.Is_as_always in
        trivial_check
          p
          env
          lhs_ty
          hint_ty
          ~always:(is_as_warning ~as_lint Is_is_always_true)
          ~never:(is_as_warning ~as_lint Is_is_always_false)
      | ( _,
          p,
          As
            {
              expr = (lhs_ty, lhs_pos, lhs_expr);
              hint;
              is_nullable;
              enforce_deep = _;
            } ) ->
        let (env, hint_ty) = Env.localize_hint_for_refinement env hint in
        let open Typing_warning.Is_as_always in
        trivial_check
          p
          env
          lhs_ty
          hint_ty
          ~always:
            (is_as_warning
               ~as_lint
               (As_always_succeeds
                  {
                    Typing_warning.can_be_captured =
                      Aast_utils.can_be_captured lhs_expr;
                    original_pos = p;
                    replacement_pos = lhs_pos;
                  }))
          ~never:(is_as_warning ~as_lint (As_always_fails { is_nullable }))
      | ( _,
          p,
          Binop
            {
              bop = Ast_defs.QuestionQuestion;
              lhs = (lhs_ty, _, lhs_expr);
              rhs = _;
            } ) -> begin
        let open Typing_warning.Null_coalesce_always in
        match lhs_expr with
        | Array_get _ -> ()
        | _ ->
          trivial_check
            p
            env
            lhs_ty
            (Typing_make_type.null (Typing_reason.witness p))
            ~always:(coalesce_warning Null_coalesce_always_null)
            ~never:(coalesce_warning Null_coalesce_never_null)
      end
      | _ -> ()
  end
