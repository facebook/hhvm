(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs
open Decl_defs
module Inst = Decl_instantiate

exception Decl_heap_elems_bug

let wrap_not_found find x =
  try
    find x
    (* TODO: t13396089 *)
  with Not_found -> raise Decl_heap_elems_bug

let rec apply_substs substs class_context ty =
  match SMap.get class_context substs with
  | None -> ty
  | Some { sc_subst = subst; sc_class_context = next_class_context; _ } ->
    apply_substs substs next_class_context (Inst.instantiate subst ty)

let element_to_class_elt ce_type {
  elt_final = ce_final;
  elt_synthesized = ce_synthesized;
  elt_override = ce_override;
  elt_abstract = _;
  elt_is_xhp_attr = ce_is_xhp_attr;
  elt_origin = ce_origin;
  elt_visibility = ce_visibility;
} =
  {
    ce_final;
    ce_is_xhp_attr;
    ce_override;
    ce_synthesized;
    ce_visibility;
    ce_origin;
    ce_type;
  }

let to_class_type {
  dc_need_init;
  dc_members_fully_known;
  dc_abstract;
  dc_final;
  dc_deferred_init_members;
  dc_kind;
  dc_name;
  dc_pos;
  dc_tparams;
  dc_substs;
  dc_consts;
  dc_typeconsts;
  dc_props;
  dc_sprops;
  dc_methods;
  dc_smethods;
  dc_construct;
  dc_ancestors;
  dc_req_ancestors;
  dc_req_ancestors_extends;
  dc_extends;
  dc_enum_type;
} =
  let map_elements find elts = SMap.mapi begin fun name elt ->
    let ty = lazy begin
      let elem =  wrap_not_found find (elt.elt_origin, name) in
      apply_substs dc_substs elt.elt_origin @@ elem
    end in
    element_to_class_elt ty elt
  end elts in

  let ft_to_ty ft = Reason.Rwitness ft.ft_pos, Tfun ft in
  let ft_map_elements find elts =
    map_elements (fun x -> find x |> ft_to_ty) elts in
  let tc_construct = match dc_construct with
    | None, consistent -> None, consistent
    | Some elt, consistent ->
      let ty = lazy begin
        elt.elt_origin |>
        wrap_not_found Decl_heap.Constructors.find_unsafe |>
        ft_to_ty |>
        apply_substs dc_substs elt.elt_origin
      end in
      let class_elt = element_to_class_elt ty elt in
      Some class_elt, consistent
  in
  {
    tc_need_init = dc_need_init;
    tc_members_fully_known = dc_members_fully_known;
    tc_abstract = dc_abstract;
    tc_final = dc_final;
    tc_deferred_init_members = dc_deferred_init_members;
    tc_kind = dc_kind;
    tc_name = dc_name;
    tc_pos = dc_pos;
    tc_tparams = dc_tparams;
    tc_consts = dc_consts;
    tc_typeconsts = dc_typeconsts;
    tc_props = map_elements Decl_heap.Props.find_unsafe dc_props ;
    tc_sprops = map_elements Decl_heap.StaticProps.find_unsafe dc_sprops;
    tc_methods = ft_map_elements Decl_heap.Methods.find_unsafe dc_methods;
    tc_smethods =
      ft_map_elements Decl_heap.StaticMethods.find_unsafe dc_smethods;
    tc_construct;
    tc_ancestors = dc_ancestors;
    tc_req_ancestors = dc_req_ancestors;
    tc_req_ancestors_extends = dc_req_ancestors_extends;
    tc_extends = dc_extends;
    tc_enum_type = dc_enum_type;
  }
