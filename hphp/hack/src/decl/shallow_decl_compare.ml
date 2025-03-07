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
module VersionedSSet = Decl_compare.VersionedSSet
module Dep = Typing_deps.Dep

let diff_class_in_changed_file
    (package_info : PackageInfo.t)
    (old_classes : shallow_class option SMap.t)
    (new_classes : shallow_class option SMap.t)
    (class_name : string) : ClassDiff.t option =
  let old_class_opt = SMap.find old_classes class_name in
  let new_class_opt = SMap.find new_classes class_name in
  match (old_class_opt, new_class_opt) with
  | (Some old_class, Some new_class) ->
    Shallow_class_diff.diff_class package_info old_class new_class
  | (None, None) -> Some (Major_change (MajorChange.Unknown Neither_found))
  | (None, Some _) ->
    Some (Major_change (MajorChange.Unknown Old_decl_not_found))
  | (Some _, None) ->
    Some (Major_change (MajorChange.Unknown New_decl_not_found))

let compute_class_diffs
    (ctx : Provider_context.t) ~during_init ~(class_names : VersionedSSet.diff)
    : (string * ClassDiff.t) list =
  let { VersionedSSet.added; kept; removed } = class_names in
  let acc = [] in
  let acc =
    SSet.fold added ~init:acc ~f:(fun name acc ->
        (name, Major_change MajorChange.Added) :: acc)
  in
  let acc =
    SSet.fold removed ~init:acc ~f:(fun name acc ->
        (name, Major_change MajorChange.Removed) :: acc)
  in
  let old_classes =
    Old_shallow_classes_provider.get_old_batch ctx ~during_init kept
  in
  let new_classes =
    SSet.fold kept ~init:SMap.empty ~f:(fun name acc ->
        SMap.add acc ~key:name ~data:(Decl_provider.get_shallow_class ctx name))
  in
  let package_info = Provider_context.get_package_info ctx in
  SSet.fold kept ~init:acc ~f:(fun cid acc ->
      let diff =
        diff_class_in_changed_file package_info old_classes new_classes cid
      in
      match diff with
      | Some diff -> (cid, diff) :: acc
      | None -> acc)

let log_changes (changes : (string * ClassDiff.t) list) : unit =
  let change_count = List.length changes in
  if List.is_empty changes then
    Hh_logger.log "No class changes detected"
  else
    Hh_logger.log "Found %d changed classes:" change_count;
  let max = 1000 in
  Hh_logger.log_lazy
  @@ lazy
       Hh_json.(
         json_to_multiline
         @@ JSON_Object
              [
                ( "diffs",
                  JSON_Object
                    (List.take changes max
                    |> List.map
                         ~f:
                           (Tuple2.map_snd ~f:(fun diff ->
                                string_ @@ ClassDiff.show diff))) );
                ("truncated", bool_ (change_count > max));
              ]);
  ()

let compute_changes
    (ctx : Provider_context.t) ~during_init ~(class_names : VersionedSSet.diff)
    : Shallow_class_fanout.changed_class list =
  Hh_logger.log
    "Detecting changes to %d classes"
    (VersionedSSet.diff_cardinal class_names);

  let changes = compute_class_diffs ctx ~during_init ~class_names in
  log_changes changes;
  List.map changes ~f:(fun (class_name, diff) ->
      let class_dep = Dep.make (Dep.Type class_name) in
      let descendant_deps =
        Typing_deps.add_extend_deps
          (Provider_context.get_deps_mode ctx)
          (Typing_deps.DepSet.singleton class_dep)
      in
      {
        Shallow_class_fanout.name = class_name;
        diff;
        dep = class_dep;
        descendant_deps;
      })
