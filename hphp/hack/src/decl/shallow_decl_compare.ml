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

let diff_class_in_changed_file
    ctx
    (package_info : PackageInfo.t)
    (old_classes : shallow_class option SMap.t)
    (new_classes : shallow_class option SMap.t)
    (class_name : string) : ClassDiff.t option =
  let old_class_opt = SMap.find old_classes class_name in
  let new_class_opt = SMap.find new_classes class_name in
  match (old_class_opt, new_class_opt) with
  | (Some old_class, Some new_class) ->
    Shallow_class_diff.diff_class ctx package_info old_class new_class
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
        diff_class_in_changed_file ctx package_info old_classes new_classes cid
      in
      match diff with
      | Some diff -> (cid, diff) :: acc
      | None -> acc)

let untag_removed_members_in_depgraph
    ~ctx (changes : (string * ClassDiff.t) list) =
  let removed_member_deps =
    List.fold
      changes
      ~init:(Typing_deps.DepSet.make ())
      ~f:(fun deps_acc (class_name, diff) ->
        match diff with
        | ClassDiff.(Major_change MajorChange.(Unknown _ | Added | Removed)) ->
          deps_acc
        | ClassDiff.(Major_change (MajorChange.Modified (_, member_diff)))
        | ClassDiff.Minor_change member_diff ->
          let class_dep =
            Typing_deps.Dep.make (Typing_deps.Dep.Type class_name)
          in
          let {
            ClassDiff.consts;
            typeconsts;
            props;
            sprops;
            methods;
            smethods;
            constructor;
          } =
            member_diff
          in
          let acc_removed_member member change deps_acc =
            match change with
            | ClassDiff.Added
            | ClassDiff.Modified
            | ClassDiff.Changed_inheritance
            | ClassDiff.Private_change_not_in_trait ->
              deps_acc
            | ClassDiff.Removed ->
              let member_dep =
                Typing_deps.Dep.make_member_dep_from_type_dep class_dep member
              in
              Typing_deps.DepSet.add deps_acc member_dep
          in
          let acc_removed_members make_member members deps_acc =
            SMap.fold
              members
              ~init:deps_acc
              ~f:(fun member_name change deps_acc ->
                acc_removed_member (make_member member_name) change deps_acc)
          in
          Option.fold constructor ~init:deps_acc ~f:(fun deps_acc change ->
              acc_removed_member
                Typing_deps.Dep.Member.constructor
                change
                deps_acc)
          |> acc_removed_members Typing_deps.Dep.Member.const consts
          |> acc_removed_members Typing_deps.Dep.Member.const typeconsts
          |> acc_removed_members Typing_deps.Dep.Member.prop props
          |> acc_removed_members Typing_deps.Dep.Member.sprop sprops
          |> acc_removed_members Typing_deps.Dep.Member.method_ methods
          |> acc_removed_members Typing_deps.Dep.Member.smethod smethods)
  in
  Typing_deps.remove_declared_tags
    (Provider_context.get_deps_mode ctx)
    removed_member_deps

let log_changes (changes : (string * ClassDiff.t) list) : unit =
  let change_count = List.length changes in
  if List.is_empty changes then
    Hh_logger.log "No class changes detected"
  else
    Hh_logger.log "Computing fanout from %d changed classes" change_count;
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

let compute_class_fanout
    (ctx : Provider_context.t)
    ~during_init
    ~(class_names : VersionedSSet.diff)
    (changed_files : Relative_path.t list) : Fanout.t =
  let file_count = List.length changed_files in
  Hh_logger.log "Detecting changes to classes in %d files:" file_count;

  let changes = compute_class_diffs ctx ~during_init ~class_names in
  log_changes changes;

  let fanout = Shallow_class_fanout.fanout_of_changes ~ctx changes in
  if TypecheckerOptions.optimized_member_fanout (Provider_context.get_tcopt ctx)
  then
    untag_removed_members_in_depgraph ~ctx changes;
  fanout
