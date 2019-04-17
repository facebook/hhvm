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

let all_ancestors class_name =
  Decl_linearize.(get_linearization ~kind:Ancestor_types) class_name
  (* Drop the requested class; we only want its ancestors. *)
  |> (fun lin -> Sequence.drop_eagerly lin 1)
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

let is_canonical _ = true
let merge ~earlier ~later:_ = earlier

let ancestors_cache class_name =
  LSTable.make (all_ancestors class_name) ~is_canonical ~merge
