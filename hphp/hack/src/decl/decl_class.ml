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

let fun_elt_to_ty fe = (fe.fe_pos, fe.fe_type)

let find_method class_name x =
  wrap_not_found class_name Decl_store.((get ()).get_method) x |> fun_elt_to_ty

let find_static_method class_name x =
  wrap_not_found class_name Decl_store.((get ()).get_static_method) x
  |> fun_elt_to_ty

let find_property class_name x =
  let ty = wrap_not_found class_name Decl_store.((get ()).get_prop) x in
  (get_pos ty, ty)

let find_static_property class_name x =
  let ty = wrap_not_found class_name Decl_store.((get ()).get_static_prop) x in
  (get_pos ty, ty)

let find_constructor class_name =
  wrap_not_found
    class_name
    (fun (class_name, _) -> Decl_store.((get ()).get_constructor class_name))
    (class_name, Naming_special_names.Members.__construct)

let map_element dc_substs find name elt =
  let pty =
    lazy
      ((elt.elt_origin, name) |> find |> apply_substs dc_substs elt.elt_origin)
  in
  element_to_class_elt pty elt

let map_property dc = map_element dc.dc_substs (find_property dc.dc_name)

let map_static_property dc =
  map_element dc.dc_substs (find_static_property dc.dc_name)

let map_method dc = map_element dc.dc_substs (find_method dc.dc_name)

let map_static_method dc =
  map_element dc.dc_substs (find_static_method dc.dc_name)

let map_constructor dc_substs dc_construct =
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
