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
module SN = Naming_special_names

let check_param _env params p user_attributes name =
  List.iter params ~f:(fun param ->
      match param.param_callconv with
      | Ast_defs.Pinout ->
        let pos = param.param_pos in
        if SSet.mem name SN.Members.as_set then Errors.inout_params_special pos
      | Ast_defs.Pnormal -> ());
  let inout =
    List.find params ~f:(fun x ->
        Ast_defs.equal_param_kind x.param_callconv Ast_defs.Pinout)
  in
  match inout with
  | Some param ->
    if
      Naming_attributes.mem2
        SN.UserAttributes.uaMemoize
        SN.UserAttributes.uaMemoizeLSB
        user_attributes
    then
      Errors.inout_params_memoize p param.param_pos
  | _ -> ()

let check_callconv_expr ((_, p, _) as e) =
  let rec check_callconv_expr_helper (_, _, expr_) =
    match expr_ with
    | Lvar (_, x)
      when not
             (String.equal (Local_id.to_string x) SN.SpecialIdents.this
             || String.equal
                  (Local_id.to_string x)
                  SN.SpecialIdents.dollardollar) ->
      ()
    | Array_get (e2, Some _) -> check_callconv_expr_helper e2
    | _ -> Errors.inout_argument_bad_expr p
  in
  check_callconv_expr_helper e

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ env f =
      let (p, name) = f.f_name in
      check_param env f.f_params p f.f_user_attributes name

    method! at_method_ env m =
      let (p, name) = m.m_name in
      check_param env m.m_params p m.m_user_attributes name

    method! at_expr _ (_, _, e) =
      match e with
      | Call (_fn, _targs, args, _unpacked_arg) ->
        List.iter
          ~f:(function
            | (Ast_defs.Pnormal, _) -> ()
            | (Ast_defs.Pinout, e) -> check_callconv_expr e)
          args
      | _ -> ()
  end
