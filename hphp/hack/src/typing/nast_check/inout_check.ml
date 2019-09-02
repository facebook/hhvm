(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast
open Nast_check_env
module SN = Naming_special_names

let check_param _env params p user_attributes f_type name =
  let byref = List.find params (fun x -> x.param_is_reference) in
  List.iter params (fun param ->
      match param.param_callconv with
      | Some Ast_defs.Pinout ->
        let pos = param.param_pos in
        if f_type <> Ast_defs.FSync then
          Errors.inout_params_outside_of_sync pos;
        if SSet.mem name SN.Members.as_set then Errors.inout_params_special pos;
        Option.iter byref ~f:(fun p ->
            Errors.inout_params_mix_byref pos p.param_pos)
      | None -> ());
  let inout =
    List.find params (fun x -> x.param_callconv = Some Ast_defs.Pinout)
  in
  match inout with
  | Some param ->
    if
      Attributes.mem2
        SN.UserAttributes.uaMemoize
        SN.UserAttributes.uaMemoizeLSB
        user_attributes
    then
      Errors.inout_params_memoize p param.param_pos
  | _ -> ()

let is_dynamic_call func_expr =
  match func_expr with
  (* regular function call, e.g. func() *)
  | Id _ -> false
  (* instance method call, e.g. $x->method() *)
  | Obj_get (_, (_, Id _), _) -> false
  (* static method call, e.g. Foo::method() *)
  | Class_const (_, _) -> false
  (* everything else *)
  | _ -> true

let check_call_expr env func_expr func_args =
  List.iter func_args (fun (arg_pos, arg) ->
      match arg with
      | Unop (Ast_defs.Uref, _)
        when TypecheckerOptions.disallow_byref_calls env.tcopt ->
        Errors.byref_call arg_pos
      | Unop (Ast_defs.Uref, (_, (Class_get _ | Obj_get _))) ->
        Errors.byref_on_property arg_pos
      | Unop (Ast_defs.Uref, _) when is_dynamic_call func_expr ->
        if TypecheckerOptions.disallow_byref_dynamic_calls env.tcopt then
          Errors.byref_dynamic_call arg_pos
      | _ -> ())

let check_callconv_expr e =
  let rec check_callconv_expr_helper e1 =
    match snd e1 with
    | Lvar (_, x)
      when not
             ( Local_id.to_string x = SN.SpecialIdents.this
             || Local_id.to_string x = SN.SpecialIdents.dollardollar ) ->
      ()
    | Array_get (e2, Some _) -> check_callconv_expr_helper e2
    | _ -> Errors.inout_argument_bad_expr (fst e)
  in
  check_callconv_expr_helper e

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ env f =
      let (p, name) = f.f_name in
      let f_type = f.f_fun_kind in
      check_param env f.f_params p f.f_user_attributes f_type name

    method! at_method_ env m =
      let (p, name) = m.m_name in
      let f_type = m.m_fun_kind in
      check_param env m.m_params p m.m_user_attributes f_type name

    method! at_expr env (_, e) =
      match e with
      | Call (_, (_, func_expr), _, func_args, _) ->
        check_call_expr env func_expr func_args
      | Callconv (_, e) -> check_callconv_expr e
      | _ -> ()
  end
