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
open Typing_deps

let class_names_from_deps ~ctx ~get_classes_in_file deps =
  let mode = Provider_context.get_deps_mode ctx in
  let filenames = Naming_provider.ByHash.get_files ctx deps in
  Relative_path.Set.fold filenames ~init:SSet.empty ~f:(fun file acc ->
      SSet.fold (get_classes_in_file file) ~init:acc ~f:(fun cid acc ->
          if DepSet.mem deps Dep.(make (hash_mode mode) (Type cid)) then
            SSet.add acc cid
          else
            acc))

let add_minor_change_fanout
    ~(ctx : Provider_context.t)
    ~(get_classes_in_file : Relative_path.t -> SSet.t)
    (acc : AffectedDeps.t)
    (class_name : string)
    (minor_change : ClassDiff.minor_change) : AffectedDeps.t =
  let mode = Provider_context.get_deps_mode ctx in
  let dep = Dep.make (hash_mode mode) (Dep.Type class_name) in
  let changed = DepSet.singleton mode dep in
  let acc = AffectedDeps.mark_changed acc changed in
  let acc = AffectedDeps.mark_as_needing_recheck acc changed in
  let {
    mro_positions_changed;
    member_diff =
      { consts; typeconsts; props; sprops; methods; smethods; constructor };
  } =
    minor_change
  in
  let acc =
    (* If positions affecting the linearization have changed, we need to update
       positions in the linearization of this class and all its descendants.
       We mark those linearizations as invalidated here. We don't need to
       recheck the fanout of the invalidated classes--since only positions
       changed, there will be no change in the fanout except in the positions
       in error messages, and we recheck all files with errors anyway. *)
    if mro_positions_changed then
      let changed_and_descendants = Typing_deps.add_extend_deps mode changed in
      AffectedDeps.mark_mro_invalidated acc changed_and_descendants
    else
      acc
  in
  let changed_and_descendant_class_names =
    lazy
      begin
        let changed_and_descendants =
          Typing_deps.add_extend_deps mode changed
        in
        class_names_from_deps ~ctx ~get_classes_in_file changed_and_descendants
      end
  in
  (* Recheck any files with a reference to the member returned by make_dep,
     even if the member was overridden in a subclass. This deals with the case
     where adding, removing, or changing the abstract-ness of a member causes
     some subclass which inherits a member of that name from multiple parents
     to resolve the conflict in a different way than it did previously. *)
  let recheck_descendants_and_their_member_dependents acc make_dep =
    SSet.fold
      (Lazy.force changed_and_descendant_class_names)
      ~init:acc
      ~f:(fun cid acc ->
        let dep = Dep.make (hash_mode mode) (Dep.Type cid) in
        let needs_recheck =
          Typing_deps.add_typing_deps mode (DepSet.singleton mode dep)
        in
        let acc = AffectedDeps.mark_as_needing_recheck acc needs_recheck in
        AffectedDeps.mark_all_dependents_as_needing_recheck
          mode
          acc
          (make_dep cid))
  in
  let add_member_fanout acc change make_dep =
    if not (ClassDiff.change_affects_descendants change) then
      AffectedDeps.mark_all_dependents_as_needing_recheck
        mode
        acc
        (make_dep class_name)
    else
      recheck_descendants_and_their_member_dependents acc make_dep
  in
  let add_member_fanouts acc changes make_dep =
    SMap.fold changes ~init:acc ~f:(fun member_id change acc ->
        add_member_fanout acc change (make_dep member_id))
  in
  let acc =
    SMap.fold consts ~init:acc ~f:(fun name change acc ->
        let acc =
          (* If a const has been added or removed in an enum type, we must recheck
             all switch statements which need to have a case for each variant
             (exhaustive switch statements add an AllMembers dependency).
             We don't bother to test whether the class is an enum type because
             non-enum classes will have no AllMembers dependents anyway. *)
          match change with
          | Added
          | Removed ->
            AffectedDeps.mark_all_dependents_as_needing_recheck
              mode
              acc
              (Dep.AllMembers class_name)
          | _ -> acc
        in
        add_member_fanout acc change (fun cid -> Dep.Const (cid, name)))
  in
  let acc =
    add_member_fanouts acc typeconsts (fun mid cid -> Dep.Const (cid, mid))
  in
  let acc = add_member_fanouts acc props (fun mid cid -> Dep.Prop (cid, mid)) in
  let acc =
    add_member_fanouts acc sprops (fun mid cid -> Dep.Prop (cid, mid))
  in
  let acc =
    add_member_fanouts acc methods (fun mid cid -> Dep.Method (cid, mid))
  in
  let acc =
    add_member_fanouts acc smethods (fun mid cid -> Dep.Method (cid, mid))
  in
  let acc =
    Option.value_map constructor ~default:acc ~f:(fun change ->
        add_member_fanout acc change (fun cid -> Dep.Constructor cid))
  in
  acc

let add_maximum_fanout
    (ctx : Provider_context.t) (acc : AffectedDeps.t) (class_name : string) =
  let mode = Provider_context.get_deps_mode ctx in
  AffectedDeps.add_maximum_fanout
    mode
    acc
    (Dep.make (hash_mode mode) (Dep.Type class_name))

let add_fanout
    ~(ctx : Provider_context.t)
    ~(get_classes_in_file : Relative_path.t -> SSet.t)
    (acc : AffectedDeps.t)
    (class_name, diff) : AffectedDeps.t =
  match diff with
  | Unchanged -> acc
  | Major_change -> add_maximum_fanout ctx acc class_name
  | Minor_change minor_change ->
    add_minor_change_fanout
      ~ctx
      ~get_classes_in_file
      acc
      class_name
      minor_change

let fanout_of_changes
    ~(ctx : Provider_context.t)
    ~(get_classes_in_file : Relative_path.t -> SSet.t)
    (changes : (string * ClassDiff.t) list) : AffectedDeps.t =
  let mode = Provider_context.get_deps_mode ctx in
  List.fold
    changes
    ~init:(AffectedDeps.empty mode)
    ~f:(add_fanout ~ctx ~get_classes_in_file)
