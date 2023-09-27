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
module Cls = Decl_provider.Class
module Env = Tast_env
module MakeType = Typing_make_type
module SN = Naming_special_names

let enum_base_type env cid =
  match Decl_provider.get_class (Typing_env.get_ctx env) cid with
  | None -> (env, None)
  | Some cls ->
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

let is_nothing env ty =
  let nothing = MakeType.nothing Reason.Rnone in
  Tast_env.is_sub_type env ty nothing

let error_if_inequatable env ty1 ty2 err =
  let expand_tydef =
    Typing_tdef.force_expand_typedef ~ety_env:empty_expand_env
  in
  let expand_enum = opaque_enum_expander#on_type in
  (* Break all type abstractions *)
  let expand env ty =
    let (env, ty) = expand_enum env ty in
    expand_tydef env ty
  in
  let typing_env = Env.tast_env_as_typing_env env in
  let ((typing_env, _), ety1, _) = expand typing_env ty1 in
  let ((typing_env, _), ety2, _) = expand typing_env ty2 in
  if is_nothing env ety1 || is_nothing env ety2 then
    ()
  else if Typing_subtype.is_type_disjoint typing_env ety1 ety2 then
    err (Env.print_ty env ty1) (Env.print_ty env ty2)

let ensure_valid_equality_check env p bop e1 e2 =
  let ty1 = get_type e1 in
  let ty2 = get_type e2 in
  error_if_inequatable
    env
    ty1
    ty2
    (Lints_errors.non_equatable_comparison p (equal_bop bop Diff2))

let ensure_valid_contains_check env p trv_val_ty val_ty =
  error_if_inequatable
    env
    trv_val_ty
    val_ty
    (Lints_errors.invalid_contains_check p)

let ensure_valid_contains_key_check env p trv_key_ty key_ty =
  error_if_inequatable
    env
    trv_key_ty
    key_ty
    (Lints_errors.invalid_contains_key_check p)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Binop { bop = (Diff | Diff2 | Eqeqeq | Eqeq) as bop; lhs; rhs })
        ->
        ensure_valid_equality_check env p bop lhs rhs
      | ( _,
          p,
          Call { func = (_, _, Id (_, id)); targs = [(tv1, _); (tv2, _)]; _ } )
        when String.equal id SN.HH.contains ->
        ensure_valid_contains_check env p tv1 tv2
      | ( _,
          p,
          Call { func = (_, _, Id (_, id)); targs = [(tk1, _); (tk2, _); _]; _ }
        )
        when String.equal id SN.HH.contains_key ->
        ensure_valid_contains_key_check env p tk1 tk2
      | _ -> ()
  end
