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
    ~(defs : FileInfo.names Relative_path.Map.t)
    ~(fetch_old_decls :
       string list -> Shallow_decl_defs.shallow_class option SMap.t) :
    (string * ClassDiff.t) list =
  let all_defs =
    Relative_path.Map.fold defs ~init:FileInfo.empty_names ~f:(fun _ ->
        FileInfo.merge_names)
  in
  let possibly_changed_classes = all_defs.FileInfo.n_classes in
  let old_classes =
    Shallow_classes_provider.get_old_batch
      ctx
      possibly_changed_classes
      ~fetch_old_decls
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
    ~(defs : FileInfo.names Relative_path.Map.t)
    ~(fetch_old_decls :
       string list -> Shallow_decl_defs.shallow_class option SMap.t)
    (changed_files : Relative_path.t list) : AffectedDeps.t =
  let file_count = List.length changed_files in
  Hh_logger.log "Detecting changes to classes in %d files:" file_count;

  let changes = compute_class_diffs ctx ~defs ~fetch_old_decls in
  let change_count = List.length changes in
  if List.is_empty changes then
    Hh_logger.log "No class changes detected"
  else
    Hh_logger.log "Computing fanout from %d changed classes" change_count;

  Shallow_class_fanout.fanout_of_changes ~ctx changes
