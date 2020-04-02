(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Decl_defs
module Inst = Decl_instantiate

exception Decl_heap_elems_bug

let wrap_not_found child_class_name elem_name find x =
  try find x (* TODO: t13396089 *)
  with Not_found ->
    Hh_logger.log
      "Decl_heap_elems_bug: could not find %s (inherited by %s):\n%s"
      (elem_name ())
      child_class_name
      (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100));
    raise Decl_heap_elems_bug

let rec apply_substs substs class_context (pos, ty) =
  match SMap.find_opt class_context substs with
  | None -> (pos, ty)
  | Some { sc_subst = subst; sc_class_context = next_class_context; _ } ->
    apply_substs substs next_class_context (pos, Inst.instantiate subst ty)

let element_to_class_elt
    (ce_pos, ce_type)
    {
      elt_final = final;
      elt_synthesized = synthesized;
      elt_override = override;
      elt_lsb = lsb;
      elt_memoizelsb = memoizelsb;
      elt_abstract = abstract;
      elt_dynamicallycallable = dynamicallycallable;
      elt_xhp_attr = xhp_attr;
      elt_const = const;
      elt_lateinit = lateinit;
      elt_origin = ce_origin;
      elt_visibility = ce_visibility;
      elt_reactivity = _;
      elt_fixme_codes = _;
      elt_deprecated = ce_deprecated;
    } =
  {
    ce_visibility;
    ce_origin;
    ce_type;
    ce_deprecated;
    ce_pos;
    ce_flags =
      make_ce_flags
        ~xhp_attr
        ~abstract
        ~final
        ~const
        ~synthesized
        ~lateinit
        ~override
        ~lsb
        ~memoizelsb
        ~dynamicallycallable;
  }

let to_class_type
    {
      dc_need_init;
      dc_members_fully_known;
      dc_abstract;
      dc_final;
      dc_const;
      dc_ppl;
      dc_deferred_init_members;
      dc_kind;
      dc_is_xhp;
      dc_has_xhp_keyword;
      dc_is_disposable;
      dc_name;
      dc_pos;
      dc_tparams;
      dc_where_constraints;
      dc_substs;
      dc_consts;
      dc_typeconsts;
      dc_pu_enums;
      dc_props;
      dc_sprops;
      dc_methods;
      dc_smethods;
      dc_construct;
      dc_ancestors;
      dc_req_ancestors;
      dc_req_ancestors_extends;
      dc_extends;
      dc_sealed_whitelist;
      dc_xhp_attr_deps = _;
      dc_enum_type;
      dc_decl_errors;
      dc_condition_types = _;
    } =
  let map_elements find elts =
    SMap.mapi
      begin
        fun name elt ->
        let (pos, ty) =
          let pos_and_ty =
            lazy
              begin
                let elem_name () =
                  Printf.sprintf "%s::%s" elt.elt_origin name
                in
                let elem =
                  wrap_not_found dc_name elem_name find (elt.elt_origin, name)
                in
                apply_substs dc_substs elt.elt_origin @@ elem
              end
          in
          let pos = lazy (fst (Lazy.force pos_and_ty)) in
          let ty = lazy (snd (Lazy.force pos_and_ty)) in
          (pos, ty)
        in
        element_to_class_elt (pos, ty) elt
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
      let (pos, ty) =
        let pos_and_ty =
          lazy
            begin
              let name = Naming_special_names.Members.__construct in
              let elem_name () = Printf.sprintf "%s::%s" elt.elt_origin name in
              elt.elt_origin
              |> wrap_not_found
                   dc_name
                   elem_name
                   Decl_heap.Constructors.find_unsafe
              |> fun_elt_to_ty
              |> apply_substs dc_substs elt.elt_origin
            end
        in
        let pos = lazy (fst (Lazy.force pos_and_ty)) in
        let ty = lazy (snd (Lazy.force pos_and_ty)) in
        (pos, ty)
      in
      let class_elt = element_to_class_elt (pos, ty) elt in
      (Some class_elt, consistent)
  in
  {
    tc_need_init = dc_need_init;
    tc_members_fully_known = dc_members_fully_known;
    tc_abstract = dc_abstract;
    tc_final = dc_final;
    tc_const = dc_const;
    tc_ppl = dc_ppl;
    tc_deferred_init_members = dc_deferred_init_members;
    tc_kind = dc_kind;
    tc_is_xhp = dc_is_xhp;
    tc_has_xhp_keyword = dc_has_xhp_keyword;
    tc_is_disposable = dc_is_disposable;
    tc_name = dc_name;
    tc_pos = dc_pos;
    tc_tparams = dc_tparams;
    tc_where_constraints = dc_where_constraints;
    tc_consts = dc_consts;
    tc_typeconsts = dc_typeconsts;
    tc_props =
      map_elements
        (fun x ->
          let ty = Decl_heap.Props.find_unsafe x in
          (get_pos ty, ty))
        dc_props;
    tc_pu_enums = dc_pu_enums;
    tc_sprops =
      map_elements
        (fun x ->
          let ty = Decl_heap.StaticProps.find_unsafe x in
          (get_pos ty, ty))
        dc_sprops;
    tc_methods = ft_map_elements Decl_heap.Methods.find_unsafe dc_methods;
    tc_smethods =
      ft_map_elements Decl_heap.StaticMethods.find_unsafe dc_smethods;
    tc_construct;
    tc_ancestors = dc_ancestors;
    tc_req_ancestors = dc_req_ancestors;
    tc_req_ancestors_extends = dc_req_ancestors_extends;
    tc_extends = dc_extends;
    tc_enum_type = dc_enum_type;
    tc_sealed_whitelist = dc_sealed_whitelist;
    tc_decl_errors = dc_decl_errors;
  }
