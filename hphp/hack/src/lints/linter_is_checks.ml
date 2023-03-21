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
open Typing_defs
open Utils
module Cls = Decl_provider.Class

let trivial_check
    pos
    env
    lhs_ty
    rhs_ty
    ~always_nonnull
    ~never_null
    ~always_subtype
    ~never_subtype =
  let (env, lhs_ty) = Tast_env.expand_type env lhs_ty in
  let (env, rhs_ty) = Tast_env.expand_type env rhs_ty in
  match (get_node lhs_ty, get_node rhs_ty) with
  | (_, Tnonnull) when Tast_utils.type_non_nullable env lhs_ty ->
    always_nonnull pos (Tast_env.print_ty env lhs_ty)
  | (_, Tprim Tnull) when Tast_utils.type_non_nullable env lhs_ty ->
    never_null pos (Tast_env.print_ty env lhs_ty)
  | (Tclass ((_, lhs_cn), _, _), Tclass ((_, rhs_cn), _, _)) ->
    let lhs_c = Decl_provider.get_class (Tast_env.get_ctx env) lhs_cn in
    let rhs_c = Decl_provider.get_class (Tast_env.get_ctx env) rhs_cn in
    begin
      match (lhs_c, rhs_c) with
      | (Some lhs_cls, Some _)
        when String.equal lhs_cn rhs_cn || Cls.has_ancestor lhs_cls rhs_cn ->
        always_subtype pos (strip_ns lhs_cn) (strip_ns rhs_cn)
      (* Cheap test, should be subsumed by disjointness test below *)
      | (Some lhs_cls, Some rhs_cls)
        when let lhs_kind = Cls.kind lhs_cls in
             let rhs_kind = Cls.kind rhs_cls in
             Ast_defs.is_c_class lhs_kind
             && Ast_defs.is_c_class rhs_kind
             && (not (String.equal lhs_cn rhs_cn))
             && not (Cls.has_ancestor rhs_cls lhs_cn) ->
        never_subtype pos (strip_ns lhs_cn) (strip_ns rhs_cn)
      | _ ->
        let tenv = Tast_env.tast_env_as_typing_env env in
        if Typing_utils.is_type_disjoint tenv lhs_ty rhs_ty then
          never_subtype pos (strip_ns lhs_cn) (strip_ns rhs_cn)
    end
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Is ((lhs_ty, _, _), hint)) ->
        let hint_ty = Tast_env.hint_to_ty env hint in
        let (env, hint_ty) =
          Tast_env.localize_no_subst env ~ignore_errors:true hint_ty
        in
        trivial_check
          p
          env
          lhs_ty
          hint_ty
          ~always_nonnull:(fun pos ty ->
            Lints_errors.invalid_null_check pos true ty)
          ~never_null:(fun pos ty ->
            Lints_errors.invalid_null_check pos false ty)
          ~always_subtype:Lints_errors.is_always_true
          ~never_subtype:Lints_errors.is_always_false
      | (_, p, As ((lhs_ty, lhs_pos, lhs_expr), hint, false)) ->
        let hint_ty = Tast_env.hint_to_ty env hint in
        let (env, hint_ty) =
          Tast_env.localize_no_subst env ~ignore_errors:true hint_ty
        in
        let can_be_captured = Aast_utils.can_be_captured lhs_expr in
        let always_nonnull p =
          Lints_errors.redundant_nonnull_assertion
            ~can_be_captured
            ~as_pos:p
            ~child_expr_pos:lhs_pos
        in
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
          ~always_nonnull
            (* D21997525: $x as null is not particularly interesting or common,
             * so we don't warn against it. Only $x is null seems useful.
             *)
          ~never_null:(fun _ _ -> ())
          ~always_subtype
          ~never_subtype:Lints_errors.as_always_fails
      | _ -> ()
  end
