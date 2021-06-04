(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix
open Typing_defs
open Decl_defs
module Inst = Decl_instantiate

exception Decl_heap_elems_bug

[@@@warning "-3"]

let wrap_not_found (child_class_name : string) find member =
  match find member (* TODO: t13396089 *) with
  | None ->
    let (origin, name) = member in
    Hh_logger.log
      "Decl_heap_elems_bug: could not find %s::%s (inherited by %s):\n%s"
      origin
      name
      child_class_name
      Stdlib.Printexc.(raw_backtrace_to_string @@ get_callstack 100);
    raise Decl_heap_elems_bug
  | Some m -> m

[@@@warning "+3"]

let rec apply_substs substs class_context (pos, ty) =
  match SMap.find_opt class_context substs with
  | None -> (pos, ty)
  | Some { sc_subst = subst; sc_class_context = next_class_context; _ } ->
    apply_substs substs next_class_context (pos, Inst.instantiate subst ty)

let element_to_class_elt
    (pty : (Pos_or_decl.t * decl_ty) lazy_t)
    {
      elt_flags = ce_flags;
      elt_origin = ce_origin;
      elt_visibility = ce_visibility;
      elt_deprecated = ce_deprecated;
    } =
  let (ce_pos, ce_type) =
    (lazy (fst @@ Lazy.force pty), lazy (snd @@ Lazy.force pty))
  in
  { ce_visibility; ce_origin; ce_type; ce_deprecated; ce_pos; ce_flags }

let to_class_type
    ( {
        dc_need_init;
        dc_members_fully_known;
        dc_abstract;
        dc_final;
        dc_const;
        dc_deferred_init_members;
        dc_kind;
        dc_is_xhp;
        dc_has_xhp_keyword;
        dc_is_disposable;
        dc_module;
        dc_name;
        dc_pos;
        dc_tparams;
        dc_where_constraints;
        dc_substs;
        dc_consts;
        dc_typeconsts;
        dc_props;
        dc_sprops;
        dc_methods;
        dc_smethods;
        dc_construct;
        dc_ancestors;
        dc_support_dynamic_type;
        dc_req_ancestors;
        dc_req_ancestors_extends;
        dc_extends;
        dc_sealed_whitelist;
        dc_xhp_attr_deps = _;
        dc_xhp_enum_values;
        dc_enum_type;
        dc_decl_errors;
        dc_condition_types = _;
      },
      (members : Decl_store.class_members option) ) =
  let find_in_local_or_heap find_in_local find_in_heap (origin, name) =
    match find_in_local name with
    | Some m -> m
    | None -> wrap_not_found dc_name find_in_heap (origin, name)
  in
  let find_in_local project_members x =
    members >>| project_members >>= SMap.find_opt x
  in
  let find_method =
    find_in_local_or_heap
      (find_in_local @@ fun m -> m.Decl_store.m_methods)
      Decl_store.((get ()).get_method)
  in
  let find_static_method =
    find_in_local_or_heap
      (find_in_local @@ fun m -> m.Decl_store.m_static_methods)
      Decl_store.((get ()).get_static_method)
  in
  let find_property x =
    let ty =
      find_in_local_or_heap
        (find_in_local @@ fun m -> m.Decl_store.m_properties)
        Decl_store.((get ()).get_prop)
        x
    in
    (get_pos ty, ty)
  in
  let find_static_property x =
    let ty =
      find_in_local_or_heap
        (find_in_local @@ fun m -> m.Decl_store.m_static_properties)
        Decl_store.((get ()).get_static_prop)
        x
    in
    (get_pos ty, ty)
  in
  let find_constructor class_ =
    find_in_local_or_heap
      (fun _ -> members >>= fun m -> m.Decl_store.m_constructor)
      (fun (class_, _) -> Decl_store.((get ()).get_constructor class_))
      (class_, Naming_special_names.Members.__construct)
  in
  let map_elements find elts =
    SMap.mapi
      begin
        fun name elt ->
        let pty =
          lazy
            ( (elt.elt_origin, name)
            |> find
            |> apply_substs dc_substs elt.elt_origin )
        in
        element_to_class_elt pty elt
      end
      elts
  in
  let fun_elt_to_ty fe = (fe.fe_pos, fe.fe_type) in
  let ft_map_elements find elts =
    map_elements (fun x -> find x |> fun_elt_to_ty) elts
  in
  let tc_construct =
    match dc_construct with
    | (None, consistent) -> (None, consistent)
    | (Some elt, consistent) ->
      let pty =
        lazy
          ( elt.elt_origin
          |> find_constructor
          |> fun_elt_to_ty
          |> apply_substs dc_substs elt.elt_origin )
      in
      let class_elt = element_to_class_elt pty elt in
      (Some class_elt, consistent)
  in
  {
    tc_need_init = dc_need_init;
    tc_members_fully_known = dc_members_fully_known;
    tc_abstract = dc_abstract;
    tc_final = dc_final;
    tc_const = dc_const;
    tc_deferred_init_members = dc_deferred_init_members;
    tc_kind = dc_kind;
    tc_is_xhp = dc_is_xhp;
    tc_has_xhp_keyword = dc_has_xhp_keyword;
    tc_is_disposable = dc_is_disposable;
    tc_module = dc_module;
    tc_name = dc_name;
    tc_pos = dc_pos;
    tc_tparams = dc_tparams;
    tc_where_constraints = dc_where_constraints;
    tc_consts = dc_consts;
    tc_typeconsts = dc_typeconsts;
    tc_props = map_elements find_property dc_props;
    tc_sprops = map_elements find_static_property dc_sprops;
    tc_methods = ft_map_elements find_method dc_methods;
    tc_smethods = ft_map_elements find_static_method dc_smethods;
    tc_construct;
    tc_ancestors = dc_ancestors;
    tc_support_dynamic_type = dc_support_dynamic_type;
    tc_req_ancestors = dc_req_ancestors;
    tc_req_ancestors_extends = dc_req_ancestors_extends;
    tc_extends = dc_extends;
    tc_enum_type = dc_enum_type;
    tc_sealed_whitelist = dc_sealed_whitelist;
    tc_xhp_enum_values = dc_xhp_enum_values;
    tc_decl_errors = dc_decl_errors;
  }
