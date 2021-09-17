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

let type_of_mro_element mro =
  let { mro_name; mro_type_args; mro_use_pos; mro_ty_pos; _ } = mro in
  mk (Reason.Rhint mro_ty_pos, Tapply ((mro_use_pos, mro_name), mro_type_args))

let all_ancestors ~lin_ancestors_drop_one =
  Sequence.map lin_ancestors_drop_one ~f:(fun mro ->
      (mro.mro_name, type_of_mro_element mro))

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
