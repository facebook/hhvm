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
open Tast
open Ast_defs
open Typing_defs
module Cls = Folded_class
module Env = Tast_env
module SN = Naming_special_names

let warning_kind = Typing_warning.Equality_check

let error_codes = Typing_warning_utils.codes warning_kind

(** Warns when comparing two expression whose types are disjoint *)

let enum_base_type env cid =
  match Decl_provider.get_class (Typing_env.get_ctx env) cid with
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    (env, None)
  | Decl_entry.Found cls ->
    (match Cls.enum_type cls with
    | None -> (env, None)
    | Some te ->
      let ((env, ty_err_opt), ty) =
        Typing_phase.localize_no_subst env ~ignore_errors:true te.te_base
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      (env, Some ty))

let opaque_enum_expander =
  object
    inherit Type_mapper.tvar_expanding_type_mapper as super

    method! on_tnewtype env r cid tyl cstr =
      match get_node cstr with
      | Tprim Tarraykey when Typing_env.is_enum env cid ->
        let (env, cstr') = enum_base_type env cid in
        begin
          match cstr' with
          | None -> (env, mk (r, Tnewtype (cid, tyl, cstr)))
          | Some ty ->
            (match get_node ty with
            | Tprim Tarraykey -> (env, mk (r, Tnewtype (cid, tyl, cstr)))
            | _ -> (env, ty))
        end
      | _ -> super#on_tnewtype env r cid tyl cstr
  end

let add_warning env ~as_lint pos kind ty1 ty2 =
  Typing_warning_utils.add_for_migration
    (Env.get_tcopt env)
    ~as_lint:
      (if as_lint then
        Some None
      else
        None)
    (pos, warning_kind, { Typing_warning.Equality_check.kind; ty1; ty2 })

let error_if_inequatable env ty1 ty2 err =
  let expand_tydef =
    Typing_tdef.force_expand_typedef ~ety_env:empty_expand_env
  in
  let expand_enum = opaque_enum_expander#on_type in
  (* Break all type abstractions *)
  let expand env ty =
    let env = Env.tast_env_as_typing_env env in
    let (env, ty) = expand_enum env ty in
    let ((env, _), ty, _) = expand_tydef env ty in
    (Tast_env.typing_env_as_tast_env env, ty)
  in
  let (env, ety1) = expand env ty1 in
  let (env, ety2) = expand env ty2 in
  match Env.is_disjoint ~is_dynamic_call:false env ety1 ety2 with
  | Env.NonDisjoint -> ()
  | Env.Disjoint -> err (Env.print_ty env ty1) (Env.print_ty env ty2)
  | Env.DisjointIgnoringDynamic (ty1, ty2) ->
    err (Env.print_ty env ty1) (Env.print_ty env ty2)

let ensure_valid_equality_check env ~as_lint p bop e1 e2 =
  let ty1 = get_type e1 in
  let ty2 = get_type e2 in
  error_if_inequatable
    env
    ty1
    ty2
    (add_warning
       env
       ~as_lint
       p
       (Typing_warning.Equality_check.Equality (equal_bop bop Diff2)))

let ensure_valid_contains_check env ~as_lint p trv_val_ty val_ty =
  error_if_inequatable
    env
    trv_val_ty
    val_ty
    (add_warning env ~as_lint p Typing_warning.Equality_check.Contains)

let ensure_valid_contains_key_check env ~as_lint p trv_key_ty key_ty =
  error_if_inequatable
    env
    trv_key_ty
    key_ty
    (add_warning env ~as_lint p Typing_warning.Equality_check.Contains_key)

let handler ~as_lint =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Binop { bop = (Diff | Diff2 | Eqeqeq | Eqeq) as bop; lhs; rhs })
        ->
        ensure_valid_equality_check env ~as_lint p bop lhs rhs
      | ( _,
          p,
          Call { func = (_, _, Id (_, id)); targs = [(tv1, _); (tv2, _)]; _ } )
        when String.equal id SN.HH.contains ->
        ensure_valid_contains_check env ~as_lint p tv1 tv2
      | ( _,
          p,
          Call { func = (_, _, Id (_, id)); targs = [(tk1, _); (tk2, _); _]; _ }
        )
        when String.equal id SN.HH.contains_key ->
        ensure_valid_contains_key_check env ~as_lint p tk1 tk2
      | _ -> ()
  end
