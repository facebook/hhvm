(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_defs
open Shallow_decl_defs
open Typing_defs

module LSTable = Lazy_string_table
module Reason = Typing_reason
module SN = Naming_special_names

type ancestor_caches = {
  ancestors : decl ty LSTable.t; (** Types of parents, interfaces, and traits *)
  parents_and_traits : unit LSTable.t; (** Names of parents and traits only *)
  members_fully_known : bool Lazy.t;
  req_ancestor_names : unit LSTable.t;
}

let all_ancestors lin =
  lin
  |> Sequence.map ~f:begin fun mro ->
    let { mro_name; mro_type_args; _ } = mro in
    let pos =
      match Shallow_classes_heap.get mro_name with
      | None -> Pos.none
      | Some c -> fst c.sc_name
    in
    let ty = Reason.Rhint pos, Tapply ((pos, mro_name), mro_type_args) in
    mro_name, ty
  end

let parents_and_traits lin =
  lin
  |> Sequence.filter ~f:(fun mro -> not mro.mro_consts_only)
  |> Sequence.map ~f:(fun mro -> mro.mro_name, ())

let members_fully_known lin =
  lazy (Sequence.for_all lin ~f:(fun mro -> not mro.mro_class_not_found))

let req_ancestor_names class_name =
  Decl_linearize.get_linearization class_name
  |> Sequence.filter ~f:(fun mro -> mro.mro_synthesized)
  |> Sequence.map ~f:(fun mro -> mro.mro_name, ())

let is_canonical _ = true
let merge ~earlier ~later:_ = earlier

let make class_name =
  let lin =
    Decl_linearize.(get_linearization ~kind:Ancestor_types) class_name
    (* Drop the requested class; we only want its ancestors. *)
    |> (fun lin -> Sequence.drop_eagerly lin 1)
  in
  {
    ancestors =
      LSTable.make (all_ancestors lin) ~is_canonical ~merge;
    parents_and_traits =
      LSTable.make (parents_and_traits lin) ~is_canonical ~merge;
    members_fully_known = members_fully_known lin;
    req_ancestor_names =
      LSTable.make (req_ancestor_names class_name) ~is_canonical ~merge;
  }
