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

(* Helper methods *)
let check_conditionally_reactive_annotation_params p params ~is_method =
  match params with
  | [_, Class_const (_, (_, "class"))] -> ()
  | _ -> Errors.conditionally_reactive_annotation_invalid_arguments ~is_method p

let check_conditionally_reactive_annotations is_reactive p method_name user_attributes =
  let rec check l seen =
  match l with
  | [] -> ()
  | { ua_name = (_, name); ua_params } :: xs when name = SN.UserAttributes.uaOnlyRxIfImpl ->
      if seen
      then Errors.multiple_conditionally_reactive_annotations p method_name
      else if is_reactive
      then check_conditionally_reactive_annotation_params ~is_method:true p ua_params;
      check xs true
  | _ :: xs -> check xs seen in
  check user_attributes false

let check_maybe_rx_attributes_on_params is_reactive parent_attrs params =
  let parent_only_rx_if_args =
    Attributes.find SN.UserAttributes.uaAtMostRxAsArgs parent_attrs in
  let check_param seen_atmost_rx_as_rxfunc p =
    let only_rx_if_rxfunc_attr =
      Attributes.find SN.UserAttributes.uaAtMostRxAsFunc p.param_user_attributes in
    let only_rx_if_impl_attr =
      Attributes.find SN.UserAttributes.uaOnlyRxIfImpl p.param_user_attributes in
    match only_rx_if_rxfunc_attr, only_rx_if_impl_attr with
    | Some { ua_name = (p, _); _ }, _ ->
      if parent_only_rx_if_args = None || not is_reactive
      then Errors.atmost_rx_as_rxfunc_invalid_location p;
      true
    | _, Some { ua_name = (p, _); ua_params; _ } ->
      if parent_only_rx_if_args = None || not is_reactive
      then Errors.atmost_rx_as_rxfunc_invalid_location p
      else check_conditionally_reactive_annotation_params ~is_method:false p ua_params;
      true
    | _ ->  seen_atmost_rx_as_rxfunc in
  let has_param_with_atmost_rx_as_rxfunc =
    List.fold_left params ~init:false ~f:check_param in
  match parent_only_rx_if_args, has_param_with_atmost_rx_as_rxfunc with
  | Some { ua_name = (p, _); _ }, false ->
    Errors.no_atmost_rx_as_rxfunc_for_rx_if_args p
  | _ -> ()


(* context used in the iter_with_state visitor *)
type ctx = { rx_is_enabled_allowed : bool; }

let default_context = { rx_is_enabled_allowed = false; }

let visitor = object(this)
  inherit [ctx] Nast_visitor.iter_with_state as super

  method! on_expr (env, ctx) (p, e) =
    match e with
    | Id (pos, const) ->
      let const = Utils.add_ns const in
      if const = SN.Rx.is_enabled && not ctx.rx_is_enabled_allowed
      then
      Errors.rx_is_enabled_invalid_location pos
    | _ -> ();
    super#on_expr (env, ctx) (p, e)

  method! on_fun_ (env, ctx) f =
    let nb = f.f_body.fb_ast in
    match nb with
    | [If ((_, Id (_, c) as id), then_stmt, else_stmt)] ->
      (*
        (* this is the only case when HH\Rx\IS_ENABLED can appear in
           function body, other occurences are considered errors *)
        {
          if (HH\Rx\IS_ENABLED) {}
          else {}
        }
      *)
      if c = SN.Rx.is_enabled then begin
        this#on_expr (env, { rx_is_enabled_allowed = true }) id;
        super#on_block (env, ctx) then_stmt;
        super#on_block (env, ctx) else_stmt
      end
    | _ -> super#on_fun_ (env, ctx) f

end

let handler = object
  inherit Nast_visitor.handler_base

  method! at_fun_ env f =
    visitor#on_fun_ (env, default_context) f;
    check_maybe_rx_attributes_on_params env.Nast_visitor.is_reactive f.f_user_attributes f.f_params;

  method! at_method_ _env m =
    let ua = m.m_user_attributes in
    let (p, name) = m.m_name in
    let is_reactive = Nast_visitor.fun_is_reactive ua in
    check_conditionally_reactive_annotations is_reactive p name ua;
    check_maybe_rx_attributes_on_params is_reactive ua m.m_params;

end
