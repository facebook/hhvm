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

(* Everything done in this file should eventually be merged in Hack's
   regular decl phase. Right now it is more convenient to keep things
   simple and separate here. *)

exception FlowDecl of string

let fail s = raise (FlowDecl s)

let policied_id = "\\Policied"

let infer_flows_id = "\\InferFlows"

let make_callable_name cls_name_opt name =
  match cls_name_opt with
  | None -> name
  | Some cls_name -> cls_name ^ "#" ^ name

let get_attr attr attrs =
  let is_attr a = String.equal (snd a.A.ua_name) attr in
  match List.filter ~f:is_attr attrs with
  | [] -> None
  | [a] -> Some a
  | _ -> fail ("multiple '" ^ attr ^ "' attributes found")

let callable_decl attrs =
  let fd_kind =
    if Option.is_some (get_attr infer_flows_id attrs) then
      FDInferFlows
    else
      FDPublic
  in
  { fd_kind }

let collect_sigs =
  let find_policy property =
    match get_attr policied_id property.A.cv_user_attributes with
    | None -> `No_policy
    | Some attr ->
      (match attr.A.ua_params with
      | [] -> `Policy
      | [(_, A.String purpose)] -> `Purpose purpose
      | _ -> fail "expected a string literal as a purpose argument.")
  in
  let def decl_env = function
    | A.Class
        { A.c_name = (_, name); c_vars = properties; c_methods; c_tparams; _ }
      ->
      let mk_policied_prop
          ({ A.cv_id = (_, pp_name); A.cv_type = (pp_type, _); _ } as prop) =
        match find_policy prop with
        | `No_policy -> None
        | `Policy -> Some { pp_name; pp_type; pp_purpose = None }
        | `Purpose purp -> Some { pp_name; pp_type; pp_purpose = Some purp }
      in
      let cd_policied_properties =
        List.filter_map ~f:mk_policied_prop properties
      in
      let cd_tparam_variance =
        List.map ~f:(fun tp -> tp.A.tp_variance) c_tparams.A.c_tparam_list
      in
      let class_decl = { cd_policied_properties; cd_tparam_variance } in
      let de_class = SMap.add name class_decl decl_env.de_class in
      let de_fun =
        let meth de_fun m =
          let callable_name = make_callable_name (Some name) (snd m.A.m_name) in
          let decl = callable_decl m.A.m_user_attributes in
          SMap.add callable_name decl de_fun
        in
        List.fold ~init:decl_env.de_fun ~f:meth c_methods
      in
      { de_class; de_fun }
    | A.Fun { A.f_name = (_, name); f_user_attributes = attrs; _ } ->
      let callable_name = make_callable_name None name in
      let decl = callable_decl attrs in
      let de_fun = SMap.add callable_name decl decl_env.de_fun in
      { decl_env with de_fun }
    | _ -> decl_env
  in
  List.fold ~f:def ~init:{ de_class = SMap.empty; de_fun = SMap.empty }
