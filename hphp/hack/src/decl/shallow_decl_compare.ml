(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections
open Shallow_decl_defs
open Typing_deps

let is_changed
    (old_classes : shallow_class option SMap.t)
    (new_classes : shallow_class option SMap.t)
    (class_name : string) : bool =
  let old_class_opt = SMap.find_unsafe old_classes class_name in
  let new_class_opt = SMap.find_unsafe new_classes class_name in
  match (old_class_opt, new_class_opt) with
  | (Some old_class, Some new_class) -> old_class <> new_class
  | (Some _, None)
  | (None, Some _)
  | (None, None) ->
    true

let find_changed_classes
    ~(get_classes_in_file : Relative_path.t -> SSet.t)
    (changed_files : Relative_path.t list) : string list =
  let possibly_changed_classes =
    List.fold changed_files ~init:SSet.empty ~f:(fun classes filename ->
        SSet.union classes (get_classes_in_file filename))
  in
  let old_classes =
    Shallow_classes_heap.get_old_batch possibly_changed_classes
  in
  let new_classes = Shallow_classes_heap.get_batch possibly_changed_classes in
  SSet.fold possibly_changed_classes ~init:[] ~f:(fun cid acc ->
      let changed = is_changed old_classes new_classes cid in
      if not changed then
        acc
      else
        cid :: acc)

let compute_class_fanout
    ~(get_classes_in_file : Relative_path.t -> SSet.t)
    (changed_files : Relative_path.t list) : DepSet.t * DepSet.t * DepSet.t =
  let file_count = List.length changed_files in
  Hh_logger.log "Detecting changes to classes in %d files:" file_count;

  let changes = find_changed_classes ~get_classes_in_file changed_files in
  let change_count = List.length changes in
  if List.is_empty changes then
    Hh_logger.log "No class changes detected"
  else
    Hh_logger.log "Computing fanout from %d changed classes" change_count;

  let changed_class_deps =
    List.fold changes ~init:DepSet.empty ~f:(fun acc cid ->
        DepSet.add acc (Dep.make (Dep.Class cid)))
  in
  let mro_invalidated = Typing_deps.add_extend_deps changed_class_deps in
  let to_recheck = Typing_deps.add_typing_deps mro_invalidated in
  (changed_class_deps, mro_invalidated, to_recheck)
