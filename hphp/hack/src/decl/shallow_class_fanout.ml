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
  let filenames = Naming_provider.get_files ctx deps in
  Relative_path.Set.fold filenames ~init:SSet.empty ~f:(fun file acc ->
      SSet.fold (get_classes_in_file file) ~init:acc ~f:(fun cid acc ->
          if DepSet.mem deps Dep.(make (Type cid)) then
            SSet.add acc cid
          else
            acc))

let get_minor_change_fanout
    ~(ctx : Provider_context.t)
    (class_name : string)
    (member_diff : ClassDiff.member_diff) : AffectedDeps.t =
  let mode = Provider_context.get_deps_mode ctx in
  let changed = DepSet.singleton (Dep.make (Dep.Type class_name)) in
  let acc = AffectedDeps.empty () in
  let acc = AffectedDeps.mark_changed acc changed in
  let acc = AffectedDeps.mark_as_needing_recheck acc changed in
  let { consts; typeconsts; props; sprops; methods; smethods; constructor } =
    member_diff
  in
  let changed_and_descendants =
    lazy (Typing_deps.add_extend_deps mode changed)
  in
  (* Recheck any file with a dependency on the provided member
     in the changed class and in each of its descendants,
     even if the member was overridden in a subclass. This deals with the case
     where adding, removing, or changing the abstract-ness of a member causes
     some subclass which inherits a member of that name from multiple parents
     to resolve the conflict in a different way than it did previously. *)
  let recheck_descendants_and_their_member_dependents acc member =
    let changed_and_descendants = Lazy.force changed_and_descendants in
    DepSet.fold changed_and_descendants ~init:acc ~f:(fun dep acc ->
        let acc =
          AffectedDeps.mark_as_needing_recheck acc (DepSet.singleton dep)
        in
        AffectedDeps.mark_all_dependents_as_needing_recheck_from_hash
          mode
          acc
          (Typing_deps.Dep.make_member_dep_from_type_dep dep member))
  in
  let add_member_fanout
      ~is_const (member : Dep.Member.t) (change : member_change) acc =
    (* Consts and typeconsts have their types copied into descendant classes in
       folded decl (rather than being stored in a separate heap as methods and
       properties are). As a result, when using a const, we register a
       dependency only upon the class type at the use site. In contrast, if we
       use a method or property B::f which was inherited from A, we register a
       dependency both upon B::f and A::f, which is what allows us to avoid the
       more expensive use of recheck_descendants_and_their_member_dependents
       here. Since we don't register a dependency upon the const at its class of
       origin in this way, we must always take the more expensive path for
       consts. *)
    if
      is_const || ClassDiff.method_or_property_change_affects_descendants change
    then
      recheck_descendants_and_their_member_dependents acc member
    else
      AffectedDeps.mark_all_dependents_as_needing_recheck_from_hash
        mode
        acc
        (Typing_deps.Dep.make_member_dep_from_type_dep
           (Typing_deps.Dep.make (Typing_deps.Dep.Type class_name))
           member)
  in
  let add_member_fanouts ~is_const changes make_member acc =
    SMap.fold changes ~init:acc ~f:(fun name ->
        add_member_fanout ~is_const (make_member name))
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
            recheck_descendants_and_their_member_dependents acc Dep.Member.all
          | _ -> acc
        in
        add_member_fanout ~is_const:true (Dep.Member.const name) change acc)
  in
  let acc =
    acc
    |> add_member_fanouts ~is_const:true typeconsts Dep.Member.const
    |> add_member_fanouts ~is_const:false props Dep.Member.prop
    |> add_member_fanouts ~is_const:false sprops Dep.Member.sprop
    |> add_member_fanouts ~is_const:false methods Dep.Member.method_
    |> add_member_fanouts ~is_const:false smethods Dep.Member.smethod
  in
  let acc =
    Option.value_map constructor ~default:acc ~f:(fun change ->
        add_member_fanout ~is_const:false Dep.Member.constructor change acc)
  in
  acc

let get_maximum_fanout (ctx : Provider_context.t) (class_name : string) =
  let mode = Provider_context.get_deps_mode ctx in
  AffectedDeps.get_maximum_fanout mode (Dep.make (Dep.Type class_name))

let get_fanout ~(ctx : Provider_context.t) (class_name, diff) : AffectedDeps.t =
  match diff with
  | Unchanged -> AffectedDeps.empty ()
  | Major_change _major_change -> get_maximum_fanout ctx class_name
  | Minor_change minor_change ->
    get_minor_change_fanout ~ctx class_name minor_change

let direct_references_cardinal mode class_name : int =
  Typing_deps.get_ideps mode (Dep.Type class_name) |> DepSet.cardinal

let descendants_cardinal mode class_name : int =
  (Typing_deps.add_extend_deps
     mode
     (DepSet.singleton @@ Dep.make @@ Dep.Type class_name)
  |> DepSet.cardinal)
  - 1

let children_cardinal mode class_name : int =
  Typing_deps.get_ideps mode (Dep.Extends class_name) |> DepSet.cardinal

let get_fanout_cardinal (fanout : AffectedDeps.t) : int =
  DepSet.cardinal fanout.AffectedDeps.needs_recheck

module Log = struct
  let do_log ctx ~fanout_cardinal =
    TypecheckerOptions.log_fanout ~fanout_cardinal
    @@ Provider_context.get_tcopt ctx

  let log_class_fanout
      ctx
      ((class_name : string), (diff : ClassDiff.t))
      (fanout : AffectedDeps.t) : unit =
    let fanout_cardinal = get_fanout_cardinal fanout in
    if do_log ~fanout_cardinal ctx then
      let mode = Provider_context.get_deps_mode ctx in
      HackEventLogger.Fanouts.log_class
        ~class_name
        ~class_diff:(ClassDiff.show diff)
        ~fanout_cardinal
        ~class_diff_category:(ClassDiff.to_category_json diff)
        ~direct_references_cardinal:(direct_references_cardinal mode class_name)
        ~descendants_cardinal:(descendants_cardinal mode class_name)
        ~children_cardinal:(children_cardinal mode class_name)

  let log_fanout
      ctx
      (changes : _ list)
      (fanout : AffectedDeps.t)
      ~max_class_fanout_cardinal : unit =
    let fanout_cardinal = get_fanout_cardinal fanout in
    if do_log ~fanout_cardinal ctx then
      HackEventLogger.Fanouts.log
        ~changes_cardinal:(List.length changes)
        ~max_class_fanout_cardinal
        ~fanout_cardinal
end

let add_fanout ~ctx (fanout_acc, max_class_fanout_cardinal) diff =
  let fanout = get_fanout ~ctx diff in
  Log.log_class_fanout ctx diff fanout;
  let fanout_acc = AffectedDeps.union fanout_acc fanout in
  let max_class_fanout_cardinal =
    Int.max max_class_fanout_cardinal (get_fanout_cardinal fanout)
  in
  (fanout_acc, max_class_fanout_cardinal)

let fanout_of_changes
    ~(ctx : Provider_context.t) (changes : (string * ClassDiff.t) list) :
    AffectedDeps.t =
  let (fanout, max_class_fanout_cardinal) =
    List.fold changes ~init:(AffectedDeps.empty (), 0) ~f:(add_fanout ~ctx)
  in
  Log.log_fanout ctx changes fanout ~max_class_fanout_cardinal;
  fanout
