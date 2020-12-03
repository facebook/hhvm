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
module LSTable = Lazy_string_table
module Reason = Typing_reason
module SN = Naming_special_names

type ancestor_caches = {
  ancestors: decl_ty LSTable.t;  (** Types of parents, interfaces, and traits *)
  parents_and_traits: unit LSTable.t;  (** Names of parents and traits only *)
  members_fully_known: bool Lazy.t;
  req_ancestor_names: unit LSTable.t;
  all_requirements: (Pos.t * decl_ty) Sequence.t;
  is_disposable: bool Lazy.t;
}

type class_name = string

let type_of_mro_element mro =
  let { mro_name; mro_type_args; mro_use_pos; mro_ty_pos; _ } = mro in
  mk (Reason.Rhint mro_ty_pos, Tapply ((mro_use_pos, mro_name), mro_type_args))

let all_ancestors lin =
  Sequence.map lin ~f:(fun mro -> (mro.mro_name, type_of_mro_element mro))

let parents_and_traits lin =
  lin
  |> Sequence.filter ~f:(fun mro -> not (is_set mro_consts_only mro.mro_flags))
  |> Sequence.map ~f:(fun mro -> (mro.mro_name, ()))

let members_fully_known lin =
  lazy
    (Sequence.for_all lin ~f:(fun mro ->
         not (is_set mro_class_not_found mro.mro_flags)))

let req_ancestor_names lin_members =
  lin_members
  |> Sequence.filter ~f:(fun mro ->
         is_set mro_via_req_extends mro.mro_flags
         || is_set mro_via_req_impl mro.mro_flags)
  |> Sequence.map ~f:(fun mro -> (mro.mro_name, ()))

let all_requirements lin_members =
  lin_members
  |> Sequence.filter ~f:(fun mro ->
         not (is_set mro_xhp_attrs_only mro.mro_flags))
  |> Sequence.filter_map ~f:(fun mro ->
         Option.map mro.mro_required_at (fun pos ->
             (pos, type_of_mro_element mro)))

let is_disposable lin_members =
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

let is_canonical _ = true

let merge ~earlier ~later:_ = earlier

let make ctx class_name =
  let lin =
    Decl_linearize.get_linearization ctx (class_name, Decl_defs.Ancestor_types)
    (* Drop the requested class; we only want its ancestors. *)
    |> fun lin -> Sequence.drop_eagerly lin 1
  in
  let lin_members =
    Decl_linearize.get_linearization
      ctx
      (class_name, Decl_defs.Member_resolution)
  in
  {
    ancestors = LSTable.make (all_ancestors lin) ~is_canonical ~merge;
    parents_and_traits =
      LSTable.make (parents_and_traits lin) ~is_canonical ~merge;
    members_fully_known = members_fully_known lin;
    req_ancestor_names =
      LSTable.make (req_ancestor_names lin_members) ~is_canonical ~merge;
    all_requirements = all_requirements lin_members;
    is_disposable = is_disposable lin_members;
  }
