(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ClassDiff
open Reordered_argument_collections
open Shallow_decl_defs

let diff_class_in_changed_file
    (old_classes : shallow_class option SMap.t)
    (new_classes : shallow_class option SMap.t)
    (class_name : string) : ClassDiff.t =
  let old_class_opt = SMap.find old_classes class_name in
  let new_class_opt = SMap.find new_classes class_name in
  match (old_class_opt, new_class_opt) with
  | (Some old_class, Some new_class) ->
    Shallow_class_diff.diff_class old_class new_class
  | (Some _, None)
  | (None, Some _)
  | (None, None) ->
    Major_change

let compute_class_diffs
    (ctx : Provider_context.t)
    ~(get_classes_in_file : Relative_path.t -> SSet.t)
    (changed_files : Relative_path.t list) : (string * ClassDiff.t) list =
  let possibly_changed_classes =
    List.fold changed_files ~init:SSet.empty ~f:(fun classes filename ->
        SSet.union classes (get_classes_in_file filename))
  in
  let old_classes =
    Shallow_classes_provider.get_old_batch ctx possibly_changed_classes
  in
  let new_classes =
    Shallow_classes_provider.get_batch ctx possibly_changed_classes
  in
  SSet.fold possibly_changed_classes ~init:[] ~f:(fun cid acc ->
      let diff = diff_class_in_changed_file old_classes new_classes cid in
      Decl_compare_utils.log_class_diff cid diff;
      if ClassDiff.equal diff Unchanged then
        acc
      else
        (cid, diff) :: acc)

let compute_class_fanout
    (ctx : Provider_context.t)
    ~(get_classes_in_file : Relative_path.t -> SSet.t)
    (changed_files : Relative_path.t list) : AffectedDeps.t =
  let file_count = List.length changed_files in
  Hh_logger.log "Detecting changes to classes in %d files:" file_count;

  let changes = compute_class_diffs ctx ~get_classes_in_file changed_files in
  let change_count = List.length changes in
  if List.is_empty changes then
    Hh_logger.log "No class changes detected"
  else
    Hh_logger.log "Computing fanout from %d changed classes" change_count;

  Shallow_class_fanout.fanout_of_changes ~get_classes_in_file changes
