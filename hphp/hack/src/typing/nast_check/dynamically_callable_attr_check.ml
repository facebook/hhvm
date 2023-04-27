(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Hh_prelude
open Aast
module SN = Naming_special_names

let has_reified_generics tparaml =
  List.exists
    ~f:(fun tparam ->
      match tparam.tp_reified with
      | Erased -> false
      | SoftReified
      | Reified ->
        true)
    tparaml

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_method_ _ m =
      let (pos, _) = m.m_name in
      let vis = m.m_visibility in
      let attr = m.m_user_attributes in
      let check_reified_callable p =
        if has_reified_generics m.m_tparams then
          Errors.add_nast_check_error
          @@ Nast_check_error.Dynamically_callable_reified p
      in
      match
        ( Naming_attributes.mem_pos SN.UserAttributes.uaDynamicallyCallable attr,
          vis )
      with
      | (Some p, Public)
      | (Some p, Internal) ->
        check_reified_callable p
      | (Some p, _) ->
        let vis =
          match vis with
          | Public -> Naming_error.Vpublic
          | Private -> Naming_error.Vprivate
          | Protected -> Naming_error.Vprotected
          | Internal -> Naming_error.Vinternal
        in
        Errors.add_error
          Naming_error.(
            to_user_error
            @@ Illegal_use_of_dynamically_callable
                 { attr_pos = p; meth_pos = pos; vis });
        check_reified_callable p
      | _ -> ()

    method! at_fun_def _ fd =
      let attrs = fd.fd_fun.f_user_attributes in
      match
        Naming_attributes.mem_pos SN.UserAttributes.uaDynamicallyCallable attrs
      with
      | Some p ->
        if has_reified_generics fd.fd_tparams then
          Errors.add_nast_check_error
          @@ Nast_check_error.Dynamically_callable_reified p;
        ()
      | _ -> ()
  end
