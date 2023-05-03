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
      | Ast_defs.Pinout _ ->
        let pos = param.param_pos in
        if SSet.mem name SN.Members.as_set then
          Errors.add_error
            Nast_check_error.(to_user_error @@ Inout_params_special pos)
      | Ast_defs.Pnormal -> ());
  let inout =
    List.find params ~f:(fun x ->
        match x.param_callconv with
        | Ast_defs.Pinout _ -> true
        | Ast_defs.Pnormal -> false)
  in
  match inout with
  | Some param ->
    if
      Naming_attributes.mem2
        SN.UserAttributes.uaMemoize
        SN.UserAttributes.uaMemoizeLSB
        user_attributes
    then
      Errors.add_error
        Nast_check_error.(
          to_user_error
          @@ Inout_params_memoize { pos = p; param_pos = param.param_pos })
  | _ -> ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_def env fd =
      let f = fd.fd_fun in
      let (p, name) = fd.fd_name in
      check_param env f.f_params p f.f_user_attributes name

    method! at_method_ env m =
      let (p, name) = m.m_name in
      check_param env m.m_params p m.m_user_attributes name
  end
