(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast

module SN = Naming_special_names

let check_param params p user_attributes f_type name =
  let inout = List.find params (fun x -> x.param_callconv = Some Ast.Pinout) in
  let byref = List.find params (fun x -> x.param_is_reference) in
  List.iter params begin fun param ->
    match param.param_callconv with
    | None -> ()
    | Some Ast.Pinout ->
      let pos = param.param_pos in
      if f_type <> Ast.FSync then Errors.inout_params_outside_of_sync pos;
      if SSet.mem name SN.Members.as_set then Errors.inout_params_special pos;
      Option.iter byref ~f:(fun param ->
        Errors.inout_params_mix_byref pos param.param_pos);
  end;
  begin match inout with
  | Some param ->
    if Attributes.mem2 SN.UserAttributes.uaMemoize SN.UserAttributes.uaMemoize user_attributes
    then Errors.inout_params_memoize p param.param_pos
  | _ -> ()
  end


let handler = object
  inherit Nast_visitor.handler_base

  method! at_fun_ _env f =
    let (p, name) = f.f_name in
    let f_type = f.f_fun_kind in
    check_param f.f_params p f.f_user_attributes f_type name

  method! at_method_ _env m =
    let (p, name) = m.m_name in
    let f_type = m.m_fun_kind in
    check_param m.m_params p m.m_user_attributes f_type name

end
