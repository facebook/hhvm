(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
open String_utils
open Tast
module Env = Tast_env
module Cls = Typing_classes_heap
module ShapeMap = Aast.ShapeMap

let rec check_hint env (pos, hint) =
  match hint with
  | Aast.Happly ((_, x), hl) when Typing_env.is_typedef x ->
    begin match Typing_lazy_heap.get_typedef x with
    | Some {td_tparams; _} as _ty ->
      check_tparams env pos x td_tparams hl
    | None -> ()
    end
  | Aast.Happly ((_, x), hl) ->
    begin match Env.get_class env x with
    | None -> ()
    | Some class_ ->
      let tparams = Cls.tparams class_ in
      check_tparams env pos x tparams hl
    end;
    ()
  | Aast.Harray (ty1, ty2) ->
    Option.iter ty1 (check_hint env);
    Option.iter ty2 (check_hint env)
  | Aast.Hdarray (ty1, ty2) ->
    check_hint env ty1;
    check_hint env ty2
  | Aast.Hvarray_or_darray ty
  | Aast.Hvarray ty ->
    check_hint env ty
  | Aast.Htuple hl ->
    List.iter hl (check_hint env)
  | Aast.Hoption h
  | Aast.Hsoft h
  | Aast.Hlike h ->
    check_hint env h
  | Aast.Hfun (_, _, hl, _, _, variadic_hint, h, _) ->
    List.iter hl (check_hint env);
    check_hint env h;
    begin match variadic_hint with
    | Aast.Hvariadic (Some h) -> check_hint env h;
    | _ -> ()
    end
  | Aast.Hshape Aast.{ nsi_allows_unknown_fields=_; nsi_field_map } ->
    List.iter ~f:(fun v -> check_hint env v.Aast.sfi_hint) nsi_field_map
  | Aast.Haccess _ -> ()
  | Aast.Hany  | Aast.Hmixed | Aast.Hnonnull | Aast.Hprim _
  | Aast.Hthis | Aast.Habstr _  | Aast.Hdynamic | Aast.Hnothing -> ()

and check_tparams env p x tparams hl =
  let arity = List.length tparams in
  check_arity env p x arity (List.length hl);
  List.iter hl (check_hint env)

and check_arity env pos tname arity size =
  let tenv = Env.tast_env_as_typing_env env in
  if size = arity then () else
  if size = 0 && not (Typing_env.is_strict tenv)
  && not (TypecheckerOptions.experimental_feature_enabled (Env.get_tcopt env)
  TypecheckerOptions.experimental_generics_arity)
  then ()
  else
  let num_args = soi arity in
  Errors.type_arity pos tname num_args

let check_param env p =
  Option.iter p.param_hint (check_hint env)

let check_tparam env t =
  List.iter t.tp_constraints (fun (_, h) -> check_hint env h)
let handler = object
  inherit Tast_visitor.handler_base

  method! at_class_ env c =
    List.iter c.c_extends (check_hint env);
    List.iter c.c_implements (check_hint env);
    let c_tparam_list = c.c_tparams.c_tparam_list in
    List.iter c_tparam_list (fun t ->
      List.iter t.tp_constraints (fun (_, h) -> check_hint env h)
    );
    List.iter c.c_typeconsts (fun t ->
      Option.iter t.c_tconst_type (check_hint env);
      Option.iter t.c_tconst_constraint (check_hint env);
    );
    let check_var v =
      if TypecheckerOptions.typecheck_xhp_cvars
      (Env.get_tcopt env) then
      Option.iter v.cv_type (check_hint env) in
    List.iter c.c_vars check_var;
    List.iter c.c_static_vars check_var

  method! at_static_method env m =
    Option.iter m.m_ret (check_hint env);
    List.iter m.m_tparams (check_tparam env);
    List.iter m.m_params (check_param env)

  method! at_method_ env m =
    Option.iter m.m_ret (check_hint env);
    List.iter m.m_tparams (check_tparam env);
    List.iter m.m_params (check_param env)

  method! at_fun_ env f =
    Option.iter f.f_ret (check_hint env);
    List.iter f.f_tparams (check_tparam env);
    List.iter f.f_params (check_param env)

  method! at_expr env (_, e) =
    match e with
    | As (_, h, _) -> check_hint env h
    | Is (_, h) -> check_hint env h
    | New ((_, CI (_, cid)), targs, _, _, (p, _)) ->
      begin match Env.get_class env cid with
      | None -> ()
      | Some class_ ->
        let tparams_length = List.length (Cls.tparams class_) in
        let hargs_length = List.length targs in
        if (hargs_length <> tparams_length) && (hargs_length <> 0)
          then Errors.type_arity p cid (string_of_int tparams_length)
      end
    | _ -> ()

  method! at_typedef env t =
    Option.iter t.t_constraint (check_hint env);
    check_hint env t.t_kind

end
