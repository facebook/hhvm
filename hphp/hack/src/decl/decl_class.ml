(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Decl_defs
module Inst = Decl_instantiate
module SN = Naming_special_names

exception Decl_heap_elems_bug of string

(** Raise an exception when the class element can't be found.

Note that this exception can be raised in two modes:

1. A Provider_context.t was not available (e.g, Zoncolan execution) and the
   element was not in the member heaps. A bug in decling or invalidation has
   occurred, potentially due to files being changed on disk while Hack was
   decling. No [lazy_member_lookup_error] is available, because we didn't do
   a lazy member lookup.
2. A Provider_context.t was available (regular Hack execution with eviction),
   the element was not in the member heap, so we tried falling back to disk.
   However, the element could not be found in the origin class. This might be
   due to an inconsistent decl heap (e.g., due to files being changed on disk
   while Hack was decling) or because the file containing the element was
   changed while type checking. In this case, we have a
   [lazy_member_lookup_error] available.
*)
let raise_decl_heap_elems_bug
    ~(err : Decl_folded_class.lazy_member_lookup_error option)
    ~(child_class_name : string)
    ~(elt_origin : string)
    ~(member_name : string) =
  let data =
    Printf.sprintf
      "could not find %s::%s (inherited by %s) (%s)"
      elt_origin
      member_name
      child_class_name
      (Option.map ~f:Decl_folded_class.show_lazy_member_lookup_error err
      |> Option.value ~default:"no lazy member lookup performed")
  in
  Hh_logger.log
    "Decl_heap_elems_bug: %s\n%s"
    data
    (Exception.get_current_callstack_string 99 |> Exception.clean_stack);
  HackEventLogger.decl_consistency_bug ~data "Decl_heap_elems_bug";
  raise (Decl_heap_elems_bug data)

let unpack_member_lookup_result
    (type a)
    ~child_class_name
    ~elt_origin
    ~member_name
    (res : (a, Decl_folded_class.lazy_member_lookup_error) result option) : a =
  let res =
    match res with
    | None -> Error None
    | Some res -> Result.map_error ~f:(fun x -> Some x) res
  in
  match res with
  | Ok a -> a
  | Error err ->
    raise_decl_heap_elems_bug ~err ~child_class_name ~elt_origin ~member_name

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
      elt_sort_text = ce_sort_text;
    } =
  let (ce_pos, ce_type) =
    (lazy (fst @@ Lazy.force pty), lazy (snd @@ Lazy.force pty))
  in
  {
    ce_visibility;
    ce_origin;
    ce_type;
    ce_deprecated;
    ce_pos;
    ce_flags;
    ce_sort_text;
  }

let fun_elt_to_ty fe = (fe.fe_pos, fe.fe_type)

let find_method ctx ~child_class_name x =
  let (elt_origin, sm_name) = x in
  let fun_elt =
    match Decl_store.((get ()).get_method x) with
    | Some fe -> fe
    | None ->
      Option.map
        ctx
        ~f:
          (Decl_folded_class.method_decl_lazy
             ~sh:SharedMem.Uses
             ~is_static:false
             ~elt_origin
             ~sm_name)
      |> unpack_member_lookup_result
           ~child_class_name
           ~elt_origin
           ~member_name:sm_name
  in

  fun_elt_to_ty fun_elt

let find_static_method ctx ~child_class_name x =
  let (elt_origin, sm_name) = x in
  let fun_elt =
    match Decl_store.((get ()).get_static_method x) with
    | Some fe -> fe
    | None ->
      Option.map
        ctx
        ~f:
          (Decl_folded_class.method_decl_lazy
             ~sh:SharedMem.Uses
             ~is_static:true
             ~elt_origin
             ~sm_name)
      |> unpack_member_lookup_result
           ~child_class_name
           ~elt_origin
           ~member_name:sm_name
  in

  fun_elt_to_ty fun_elt

let find_property ctx ~child_class_name x =
  let (elt_origin, sp_name) = x in
  let ty =
    match Decl_store.((get ()).get_prop x) with
    | Some ty -> ty
    | None ->
      Option.map
        ctx
        ~f:
          (Decl_folded_class.prop_decl_lazy
             ~sh:SharedMem.Uses
             ~elt_origin
             ~sp_name)
      |> unpack_member_lookup_result
           ~child_class_name
           ~elt_origin
           ~member_name:sp_name
  in

  (get_pos ty, ty)

let find_static_property ctx ~child_class_name x =
  let (elt_origin, sp_name) = x in
  let ty =
    match Decl_store.((get ()).get_static_prop x) with
    | Some ty -> ty
    | None ->
      Option.map
        ctx
        ~f:
          (Decl_folded_class.static_prop_decl_lazy
             ~sh:SharedMem.Uses
             ~elt_origin
             ~sp_name)
      |> unpack_member_lookup_result
           ~child_class_name
           ~elt_origin
           ~member_name:sp_name
  in

  (get_pos ty, ty)

let find_constructor ctx ~child_class_name ~elt_origin =
  match Decl_store.((get ()).get_constructor elt_origin) with
  | Some fe -> fe
  | None ->
    Option.map
      ctx
      ~f:
        (Decl_folded_class.constructor_decl_lazy ~sh:SharedMem.Uses ~elt_origin)
    |> unpack_member_lookup_result
         ~child_class_name
         ~elt_origin
         ~member_name:SN.Members.__construct

let map_element dc_substs find name (elt : element) =
  let pty =
    lazy
      ((elt.elt_origin, name) |> find |> apply_substs dc_substs elt.elt_origin)
  in
  element_to_class_elt pty elt

let lookup_property_type_lazy
    (ctx : Provider_context.t option) (dc : Decl_defs.decl_class_type) =
  map_element
    dc.dc_substs
    (find_property ~child_class_name:dc.Decl_defs.dc_name ctx)

let lookup_static_property_type_lazy
    (ctx : Provider_context.t option) (dc : Decl_defs.decl_class_type) =
  map_element
    dc.dc_substs
    (find_static_property ~child_class_name:dc.Decl_defs.dc_name ctx)

let lookup_method_type_lazy
    (ctx : Provider_context.t option) (dc : Decl_defs.decl_class_type) =
  map_element
    dc.dc_substs
    (find_method ctx ~child_class_name:dc.Decl_defs.dc_name)

let lookup_static_method_type_lazy
    (ctx : Provider_context.t option) (dc : Decl_defs.decl_class_type) =
  map_element
    dc.dc_substs
    (find_static_method ctx ~child_class_name:dc.Decl_defs.dc_name)

let lookup_constructor_lazy
    (ctx : Provider_context.t option)
    ~(child_class_name : string)
    (dc_substs : Decl_defs.subst_context SMap.t)
    (dc_construct : Decl_defs.element option * Typing_defs.consistent_kind) :
    Typing_defs.class_elt option * Typing_defs.consistent_kind =
  match dc_construct with
  | (None, consistent) -> (None, consistent)
  | (Some elt, consistent) ->
    let pty =
      lazy
        (find_constructor ctx ~child_class_name ~elt_origin:elt.elt_origin
        |> fun_elt_to_ty
        |> apply_substs dc_substs elt.elt_origin)
    in
    let class_elt = element_to_class_elt pty elt in
    (Some class_elt, consistent)
