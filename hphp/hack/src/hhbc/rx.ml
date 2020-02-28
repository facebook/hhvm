(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Ast = Aast
module T = Aast

type conditional = bool

(* The possible Rx levels of a function or method *)
type t =
  | NonRx
  | RxLocal of conditional
  | RxShallow of conditional
  | Rx of conditional
  | Pure of conditional

let attr_is_pure ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaPure

let attr_is_rx ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaReactive

let attr_is_rx_shallow ast_attr =
  snd ast_attr.Ast.ua_name
  = Naming_special_names.UserAttributes.uaShallowReactive

let attr_is_rx_local ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaLocalReactive

let attr_is_non_rx ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaNonRx

let attr_is_rx_if_impl ast_attr =
  snd ast_attr.Ast.ua_name = Naming_special_names.UserAttributes.uaOnlyRxIfImpl

let attr_is_rx_as_args ast_attr =
  let name = snd ast_attr.Ast.ua_name in
  name = Naming_special_names.UserAttributes.uaAtMostRxAsArgs

let rx_level_from_ast ast_attrs =
  let pure = List.exists ast_attrs ~f:attr_is_pure in
  let rx = List.exists ast_attrs ~f:attr_is_rx in
  let non_rx = List.exists ast_attrs ~f:attr_is_non_rx in
  let rx_shallow = List.exists ast_attrs ~f:attr_is_rx_shallow in
  let rx_local = List.exists ast_attrs ~f:attr_is_rx_local in
  let cond =
    List.exists ast_attrs ~f:(fun a ->
        attr_is_rx_if_impl a || attr_is_rx_as_args a)
  in
  match (rx_local, rx_shallow, rx, pure) with
  | (true, false, false, false) -> Some (RxLocal cond)
  | (false, true, false, false) -> Some (RxShallow cond)
  | (false, false, true, false) -> Some (Rx cond)
  | (false, false, false, true) -> Some (Pure cond)
  | (false, false, false, false) when not cond ->
    if non_rx then
      Some NonRx
    else
      None
  | _ -> failwith "invalid combination of Rx attributes escaped the parser"

let rx_level_to_attr_string level =
  let f cond suffix =
    if cond then
      "conditional_" ^ suffix
    else
      suffix
  in
  match level with
  | NonRx -> None
  | RxLocal cond -> Some (f cond "rx_local")
  | RxShallow cond -> Some (f cond "rx_shallow")
  | Rx cond -> Some (f cond "rx")
  | Pure cond -> Some (f cond "pure")

let rx_level_from_attr_string s =
  match s with
  | "conditional_rx_local" -> Some (RxLocal true)
  | "conditional_rx_shallow" -> Some (RxShallow true)
  | "conditional_rx" -> Some (Rx true)
  | "conditional_pure" -> Some (Pure true)
  | "rx_local" -> Some (RxLocal false)
  | "rx_shallow" -> Some (RxShallow false)
  | "rx" -> Some (Rx false)
  | "pure" -> Some (Pure false)
  | _ -> None

let halves_of_is_enabled_body ast_body =
  let block = ast_body.T.fb_ast in
  match block with
  | [(_, T.If ((_, T.Id (_, const)), enabled, disabled))] ->
    if Naming_special_names.Rx.is_enabled const then
      match disabled with
      | []
      | [(_, T.Noop)] ->
        None
      | _ -> Some (enabled, disabled)
    else
      None
  | _ -> None
