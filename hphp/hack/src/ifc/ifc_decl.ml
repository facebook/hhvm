(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_core
open Ifc_types
module A = Aast
module T = Typing_defs

exception FlowDecl of string

let fail s = raise (FlowDecl s)

let collect_class_policy_sigs =
  let is_policied attrs =
    let is_flow attr = String.equal (snd attr.A.ua_name) "\\Policied" in
    match List.filter attrs ~f:is_flow with
    | [] -> false
    | [_] -> true
    | _ -> fail "Multiple 'Policied' attributes found."
  in
  let is_policied_property property =
    is_policied property.A.cv_user_attributes
  in
  let extract { A.cv_id; A.cv_type; _ } = (snd cv_id, fst cv_type) in
  let find_policied_properties properties =
    List.map (List.filter properties is_policied_property) extract
  in
  let def psig_env = function
    | A.Class { A.c_name = (_, name); c_vars = properties; _ } ->
      SMap.add
        name
        { psig_policied_properties = find_policied_properties properties }
        psig_env
    | _ -> psig_env
  in
  List.fold ~f:def ~init:SMap.empty
