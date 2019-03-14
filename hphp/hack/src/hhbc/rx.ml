(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module T = Tast

(* The possible Rx levels of a function or method *)
type t =
  | NonRx
  | ConditionalRxLocal
  | ConditionalRxShallow
  | ConditionalRx
  | RxLocal
  | RxShallow
  | Rx

let attr_is_rx ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaReactive

let attr_is_rx_tast ast_attr =
  snd ast_attr.T.ua_name = Naming_special_names.UserAttributes.uaReactive

let any_attr_is_rx ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx

let any_attr_is_rx_tast ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_tast

let attr_is_rx_shallow ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaShallowReactive

let attr_is_rx_shallow_tast ast_attr =
  snd ast_attr.T.ua_name = Naming_special_names.UserAttributes.uaShallowReactive

let any_attr_is_rx_shallow ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_shallow

let any_attr_is_rx_shallow_tast ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_shallow_tast

let attr_is_rx_local ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaLocalReactive

let attr_is_rx_local_tast ast_attr =
  snd ast_attr.T.ua_name = Naming_special_names.UserAttributes.uaLocalReactive

let any_attr_is_rx_local ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_local

let any_attr_is_rx_local_tast ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_local_tast

let attr_is_non_rx ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaNonRx

let attr_is_non_rx_tast ast_attr =
  snd ast_attr.T.ua_name = Naming_special_names.UserAttributes.uaNonRx

let any_attr_is_non_rx ast_attrs =
  List.exists ast_attrs ~f:attr_is_non_rx

let any_attr_is_non_rx_tast ast_attrs =
  List.exists ast_attrs ~f:attr_is_non_rx_tast

let attr_is_rx_if_impl ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaOnlyRxIfImpl

let attr_is_rx_if_impl_tast ast_attr =
  snd ast_attr.T.ua_name = Naming_special_names.UserAttributes.uaOnlyRxIfImpl

let any_attr_is_rx_if_impl ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_if_impl

let any_attr_is_rx_if_impl_tast ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_if_impl_tast

let attr_is_rx_as_args ast_attr =
  let name = snd ast_attr.Ast.ua_name in
    name = Naming_special_names.UserAttributes.uaAtMostRxAsArgs

let attr_is_rx_as_args_tast ast_attr =
  snd ast_attr.T.ua_name = Naming_special_names.UserAttributes.uaAtMostRxAsArgs

let any_attr_is_rx_as_args ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_as_args

let any_attr_is_rx_as_args_tast ast_attrs =
  List.exists ast_attrs ~f:attr_is_rx_as_args_tast

let rx_level_from_ast ast_attrs =
  let rx = any_attr_is_rx ast_attrs in
  let non_rx = any_attr_is_non_rx ast_attrs in
  let rx_shallow = any_attr_is_rx_shallow ast_attrs in
  let rx_local = any_attr_is_rx_local ast_attrs in
  let rx_conditional = any_attr_is_rx_if_impl ast_attrs
    || any_attr_is_rx_as_args ast_attrs in
  match (rx_conditional, non_rx, rx_local, rx_shallow, rx) with
    | (false, false, false, false, false) -> None
    | (false, true,  false, false, false) -> Some NonRx
    | (true,  false, true,  false, false) -> Some ConditionalRxLocal
    | (true,  false, false, true,  false) -> Some ConditionalRxShallow
    | (true,  false, false, false, true ) -> Some ConditionalRx
    | (false, false, true,  false, false) -> Some RxLocal
    | (false, false, false, true,  false) -> Some RxShallow
    | (false, false, false, false, true ) -> Some Rx
    | _ -> failwith "invalid combination of Rx attributes escaped the parser"

let rx_level_from_ast_tast ast_attrs =
  let rx = any_attr_is_rx_tast ast_attrs in
  let non_rx = any_attr_is_non_rx_tast ast_attrs in
  let rx_shallow = any_attr_is_rx_shallow_tast ast_attrs in
  let rx_local = any_attr_is_rx_local_tast ast_attrs in
  let rx_conditional = any_attr_is_rx_if_impl_tast ast_attrs
    || any_attr_is_rx_as_args_tast ast_attrs in
  match (rx_conditional, non_rx, rx_local, rx_shallow, rx) with
    | (false, false, false, false, false) -> None
    | (false, true,  false, false, false) -> Some NonRx
    | (true,  false, true,  false, false) -> Some ConditionalRxLocal
    | (true,  false, false, true,  false) -> Some ConditionalRxShallow
    | (true,  false, false, false, true ) -> Some ConditionalRx
    | (false, false, true,  false, false) -> Some RxLocal
    | (false, false, false, true,  false) -> Some RxShallow
    | (false, false, false, false, true ) -> Some Rx
    | _ -> failwith "invalid combination of Rx attributes escaped the parser"

let rx_level_to_attr_string level = match level with
  | NonRx                -> None
  | ConditionalRxLocal   -> Some "conditional_rx_local"
  | ConditionalRxShallow -> Some "conditional_rx_shallow"
  | ConditionalRx        -> Some "conditional_rx"
  | RxLocal              -> Some "rx_local"
  | RxShallow            -> Some "rx_shallow"
  | Rx                   -> Some "rx"

let rx_level_from_attr_string s = match s with
  | "conditional_rx_local"   -> Some ConditionalRxLocal
  | "conditional_rx_shallow" -> Some ConditionalRxShallow
  | "conditional_rx"         -> Some ConditionalRx
  | "rx_local"               -> Some RxLocal
  | "rx_shallow"             -> Some RxShallow
  | "rx"                     -> Some Rx
  | _                        -> None

let halves_of_is_enabled_body namespace ast_body =
  match ast_body with
  | (_, Ast.If ((_, Ast.Id const), enabled, disabled)) :: [] ->
    let fq_const =
      Namespaces.elaborate_id namespace Namespaces.ElaborateConst const in
    if snd fq_const <> Naming_special_names.Rx.is_enabled then None else
    begin match disabled with
    | [] | [ _, Ast.Noop ] -> None
    | _ -> Some (enabled, disabled)
    end
  | _ -> None

let halves_of_is_enabled_body_tast namespace ast_body =
  let block = ast_body.T.fb_ast in
  match block with
  | (_, T.If ((_, T.Id const), enabled, disabled)) :: [] ->
    let fq_const =
      Namespaces.elaborate_id namespace Namespaces.ElaborateConst const in
    if snd fq_const <> Naming_special_names.Rx.is_enabled
    then None
    else
      begin
        match disabled with
        | []
        | [ _, T.Noop ] -> None
        | _ -> Some (enabled, disabled)
      end
  | _ -> None
