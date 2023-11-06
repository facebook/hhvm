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
module TUtils = Typing_utils
module T = Typing_defs

let nothing_ty = Typing_make_type.nothing Typing_reason.Rnone

(* To handle typechecking against placeholder, e.g., `... as C<_>`, we convert
   the generic the placeholder is elaborated to into a type variable so that
   the subtyping query can be discharged. *)
let replace_placeholders_with_tvars env ty =
  let replace_placeholder env ty =
    match T.get_node ty with
    | T.Tgeneric (name, []) when String.contains name '#' ->
      Typing_env.fresh_type env Pos.none
    | _ -> (env, ty)
  in
  match T.get_node ty with
  | T.Tclass (id, exact, targs) ->
    let (env, targs) = List.fold_map ~f:replace_placeholder ~init:env targs in
    (env, T.mk (Typing_reason.Rnone, T.Tclass (id, exact, targs)))
  | _ -> (env, ty)

let trivial_check pos env lhs_ty rhs_ty ~always_subtype ~never_subtype =
  let (env, lhs_ty) = Env.expand_type env lhs_ty in
  let (env, rhs_ty) = Env.expand_type env rhs_ty in
  let tenv = Env.tast_env_as_typing_env env in
  let tenv = Typing_env.open_tyvars tenv Pos.none in
  let (tenv, rhs_ty) = replace_placeholders_with_tvars tenv rhs_ty in
  if Env.is_sub_type env lhs_ty nothing_ty then
    (* If we have a nothing in our hands, there was a bigger problem
       originating from earlier in the program. Don't flag it here, as it is
       merely a symptom. *)
    ()
  else
    (* We can't just use the `is_subtype` API which will discharge the
       propositions with the fresh type variables. Instead, we use `sub_type`
       which feedback the propositions against unconstrained type variables as
       assumptions. *)
    let callback = Typing_error.Reasons_callback.unify_error_at Pos.none in
    let (tenv, err_opt1) = TUtils.sub_type tenv lhs_ty rhs_ty (Some callback) in
    let (tenv, err_opt2) = Typing_solver.close_tyvars_and_solve tenv in
    let err_opt = Option.merge err_opt1 err_opt2 ~f:Typing_error.both in
    let env = Env.typing_env_as_tast_env tenv in
    let (env, lhs_ty) = Env.expand_type env lhs_ty in
    let (env, rhs_ty) = Env.expand_type env rhs_ty in
    let print_ty = Env.print_ty env in
    if Option.is_none err_opt then
      always_subtype pos (print_ty lhs_ty) (print_ty rhs_ty)
    else if TUtils.is_type_disjoint tenv lhs_ty rhs_ty then
      never_subtype pos (print_ty lhs_ty) (print_ty rhs_ty)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      let check_status = Env.get_check_status env in
      function
      | (_, p, Is ((lhs_ty, _, _), hint)) ->
        let (env, hint_ty) = Env.localize_hint_for_refinement env hint in
        trivial_check
          p
          env
          lhs_ty
          hint_ty
          ~always_subtype:(Lints_errors.is_always_true ~check_status)
          ~never_subtype:(Lints_errors.is_always_false ~check_status)
      | ( _,
          p,
          As
            {
              expr = (lhs_ty, lhs_pos, lhs_expr);
              hint;
              is_nullable = false;
              enforce_deep = _;
            } ) ->
        let (env, hint_ty) = Env.localize_hint_for_refinement env hint in
        let can_be_captured = Aast_utils.can_be_captured lhs_expr in
        let always_subtype p =
          Lints_errors.as_always_succeeds
            ~check_status
            ~can_be_captured
            ~as_pos:p
            ~child_expr_pos:lhs_pos
        in
        trivial_check
          p
          env
          lhs_ty
          hint_ty
          ~always_subtype
          ~never_subtype:(Lints_errors.as_always_fails ~check_status)
      | _ -> ()
  end
