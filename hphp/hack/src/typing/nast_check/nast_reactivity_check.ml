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
open Nast_check_env
module SN = Naming_special_names

(* Helper methods *)
let check_conditionally_reactive_annotation_params p params ~is_method =
  match params with
  | [(_, Class_const (_, (_, "class")))] -> ()
  | _ -> Errors.conditionally_reactive_annotation_invalid_arguments ~is_method p

let check_conditionally_reactive_annotations
    is_reactive p method_name user_attributes =
  let rec check l seen =
    match l with
    | [] -> ()
    | { ua_name = (_, name); ua_params } :: xs
      when String.equal name SN.UserAttributes.uaOnlyRxIfImpl ->
      if seen then
        Errors.multiple_conditionally_reactive_annotations p method_name
      else if is_reactive then
        check_conditionally_reactive_annotation_params
          ~is_method:true
          p
          ua_params;
      check xs true
    | _ :: xs -> check xs seen
  in
  check user_attributes false

let check_maybe_rx_attributes_on_params is_reactive parent_attrs params =
  let parent_only_rx_if_args =
    Naming_attributes.find SN.UserAttributes.uaAtMostRxAsArgs parent_attrs
  in
  let check_param seen_atmost_rx_as_rxfunc p =
    let only_rx_if_rxfunc_attr =
      Naming_attributes.find
        SN.UserAttributes.uaAtMostRxAsFunc
        p.param_user_attributes
    in
    let only_rx_if_impl_attr =
      Naming_attributes.find
        SN.UserAttributes.uaOnlyRxIfImpl
        p.param_user_attributes
    in
    match (only_rx_if_rxfunc_attr, only_rx_if_impl_attr) with
    | (Some { ua_name = (p, _); _ }, _) ->
      if Option.is_none parent_only_rx_if_args || not is_reactive then
        Errors.atmost_rx_as_rxfunc_invalid_location p;
      true
    | (_, Some { ua_name = (p, _); ua_params; _ }) ->
      if Option.is_none parent_only_rx_if_args || not is_reactive then
        Errors.atmost_rx_as_rxfunc_invalid_location p
      else
        check_conditionally_reactive_annotation_params
          ~is_method:false
          p
          ua_params;
      true
    | _ -> seen_atmost_rx_as_rxfunc
  in
  let has_param_with_atmost_rx_as_rxfunc =
    List.fold_left params ~init:false ~f:check_param
  in
  match (parent_only_rx_if_args, has_param_with_atmost_rx_as_rxfunc) with
  | (Some { ua_name = (p, _); _ }, false) ->
    Errors.no_atmost_rx_as_rxfunc_for_rx_if_args p
  | _ -> ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ env f =
      check_maybe_rx_attributes_on_params
        env.is_reactive
        f.f_user_attributes
        f.f_params

    method! at_expr env (_, e) =
      match e with
      | Id (pos, const) ->
        if SN.Rx.is_enabled const && not env.rx_is_enabled_allowed then
          Errors.rx_is_enabled_invalid_location pos
      | Call (_, (p, Id (_, cn)), _, _, _)
        when String.equal cn SN.Rx.move && not env.rx_move_allowed ->
        Errors.rx_move_invalid_location p
      | _ -> ()

    method! at_method_ _env m =
      let ua = m.m_user_attributes in
      let (p, name) = m.m_name in
      let is_reactive = fun_is_reactive ua in
      check_conditionally_reactive_annotations is_reactive p name ua;
      check_maybe_rx_attributes_on_params is_reactive ua m.m_params
  end
