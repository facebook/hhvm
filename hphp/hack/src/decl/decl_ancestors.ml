(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_defs
open Typing_defs
module Reason = Typing_reason
module SN = Naming_special_names

let type_of_mro_element mro =
  let { mro_name; mro_type_args; mro_use_pos; mro_ty_pos; _ } = mro in
  mk (Reason.Rhint mro_ty_pos, Tapply ((mro_use_pos, mro_name), mro_type_args))

let all_ancestors ~lin_ancestors_drop_one =
  Sequence.map lin_ancestors_drop_one ~f:(fun mro ->
      (mro.mro_name, type_of_mro_element mro))

let members_fully_known ~lin_ancestors_drop_one =
  lazy
    (Sequence.for_all lin_ancestors_drop_one ~f:(fun mro ->
         not (is_set mro_class_not_found mro.mro_flags)))

let req_ancestor_names ~lin_members =
  lin_members
  |> Sequence.filter ~f:(fun mro ->
         is_set mro_via_req_extends mro.mro_flags
         || is_set mro_via_req_impl mro.mro_flags)
  |> Sequence.map ~f:(fun mro -> (mro.mro_name, ()))

let all_requirements ~lin_members =
  lin_members
  |> Sequence.filter ~f:(fun mro ->
         not (is_set mro_xhp_attrs_only mro.mro_flags))
  |> Sequence.filter_map ~f:(fun mro ->
         Option.map mro.mro_required_at ~f:(fun pos ->
             (pos, type_of_mro_element mro)))

let is_disposable ~lin_members =
  (* Precisely which ancestors need we traverse to see if they're disposable?
  * We need to look at things via req_extends and req_impl; they're present
  in the member linearization but not the ancestor linearization.
  * It doesn't matter whether we travese the implicit "stringish" interface
  that is implicitly on classes that implement toString, since stringish
  isn't disposable; this is present on ancestor but not member linearization.
  * It doesn't matter whether we traverse enum include ancestors which are
  present in member but not ancestor lienarization, since they only relate
  to enums and don't factor into disposability.
  * We must not look at use_xhp_attr ancestors because using XHP attrs only
  brings in the attrs from a class, not its disposability; xhp_attrs ancestors
  are present in the member linearization but not the ancestor linearization.
  * Summary: the member linearization is the most complete thing to work off,
  and is suitable so long as we filter out xhp-attrs.
  *)
  lazy
    (Sequence.exists lin_members ~f:(fun mro ->
         (not (Decl_defs.is_set Decl_defs.mro_xhp_attrs_only mro.mro_flags))
         && ( String.equal mro.mro_name SN.Classes.cIDisposable
            || String.equal mro.mro_name SN.Classes.cIAsyncDisposable )))
