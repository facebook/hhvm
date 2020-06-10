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
  let policied_id = "\\Policied" in
  let find_policy property =
    let attrs = property.A.cv_user_attributes in
    let is_policied attr = String.equal (snd attr.A.ua_name) policied_id in
    match List.filter attrs ~f:is_policied with
    | [] -> `No_policy
    | [attr] ->
      (match attr.A.ua_params with
      | [] -> `Policy
      | [(_, A.String purpose)] -> `Purpose purpose
      | _ -> fail "expected a string literal as a purpose argument.")
    | _ ->
      fail "a property can have only one 'Policied' or `Property` attribute."
  in
  let def psig_env = function
    | A.Class { A.c_name = (_, name); c_vars = properties; _ } ->
      let mk_policied_prop
          ({ A.cv_id = (_, pp_name); A.cv_type = (pp_type, _); _ } as prop) =
        match find_policy prop with
        | `No_policy -> None
        | `Policy -> Some { pp_name; pp_type; pp_purpose = None }
        | `Purpose purp -> Some { pp_name; pp_type; pp_purpose = Some purp }
      in
      let policied_props = List.filter_map properties ~f:mk_policied_prop in
      SMap.add name { psig_policied_properties = policied_props } psig_env
    | _ -> psig_env
  in
  List.fold ~f:def ~init:SMap.empty
