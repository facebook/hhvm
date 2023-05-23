(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
module Env = Tast_env

let nothing_ty = Typing_make_type.nothing Typing_reason.Rnone

let trivial_check pos env lhs_ty rhs_ty ~always_subtype ~never_subtype =
  let (env, lhs_ty) = Env.expand_type env lhs_ty in
  let (env, rhs_ty) = Env.expand_type env rhs_ty in
  let tenv = Env.tast_env_as_typing_env env in
  let print_ty = Env.print_ty env in
  if Env.is_sub_type env lhs_ty nothing_ty then
    (* If we have a nothing in our hands, there was a bigger problem
       originating from earlier in the program. Don't flag it here, as it is
       merely a symptom. *)
    ()
  else if Env.is_sub_type env lhs_ty rhs_ty then
    always_subtype pos (print_ty lhs_ty) (print_ty rhs_ty)
  else if Typing_utils.is_type_disjoint tenv lhs_ty rhs_ty then
    never_subtype pos (print_ty lhs_ty) (print_ty rhs_ty)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Is ((lhs_ty, _, _), hint)) ->
        let hint_ty = Env.hint_to_ty env hint in
        let (env, hint_ty) =
          Env.localize_no_subst env ~ignore_errors:true hint_ty
        in
        trivial_check
          p
          env
          lhs_ty
          hint_ty
          ~always_subtype:Lints_errors.is_always_true
          ~never_subtype:Lints_errors.is_always_false
      | (_, p, As ((lhs_ty, lhs_pos, lhs_expr), hint, false)) ->
        let hint_ty = Env.hint_to_ty env hint in
        let (env, hint_ty) =
          Env.localize_no_subst env ~ignore_errors:true hint_ty
        in
        let can_be_captured = Aast_utils.can_be_captured lhs_expr in
        let always_subtype p =
          Lints_errors.as_always_succeeds
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
          ~never_subtype:Lints_errors.as_always_fails
      | _ -> ()
  end
