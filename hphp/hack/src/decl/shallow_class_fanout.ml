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

let include_fanout_of_dep (mode : Mode.t) (dep : Dep.t) (deps : DepSet.t) :
    DepSet.t =
  let fanout = Typing_deps.get_ideps_from_hash mode dep in
  DepSet.union fanout deps

let acc_member_fanout ctx class_dep member change fanout_acc =
  match change with
  | Private_change_not_in_trait -> fanout_acc
  | Added
  | Removed
  | Changed_inheritance
  | Modified ->
    Typing_deps.get_member_fanout
      (Provider_context.get_deps_mode ctx)
      ~class_dep
      member
      fanout_acc

let acc_member_fanouts ctx class_dep make_member changes fanout_acc =
  SMap.fold changes ~init:fanout_acc ~f:(fun name change fanout_acc ->
      acc_member_fanout ctx class_dep (make_member name) change fanout_acc)

let get_minor_change_fanout
    ~(ctx : Provider_context.t)
    (class_dep : Dep.t)
    (member_diff : ClassDiff.member_diff) : Fanout.t =
  let { consts; typeconsts; props; sprops; methods; smethods; constructor } =
    member_diff
  in
  let acc_fanout = acc_member_fanout ctx class_dep in
  let acc_fanouts make_member changes fanout_acc =
    acc_member_fanouts ctx class_dep make_member changes fanout_acc
  in
  let to_recheck =
    DepSet.singleton class_dep
    |> acc_fanouts Dep.Member.const consts
    |> acc_fanouts Dep.Member.const typeconsts
    |> acc_fanouts Dep.Member.prop props
    |> acc_fanouts Dep.Member.sprop sprops
    |> acc_fanouts Dep.Member.method_ methods
    |> acc_fanouts Dep.Member.smethod smethods
  in
  let to_recheck =
    Option.fold constructor ~init:to_recheck ~f:(fun to_recheck change ->
        acc_fanout Dep.Member.constructor change to_recheck)
  in
  let to_recheck =
    if
      SMap.exists consts ~f:(fun _name change ->
          method_or_property_change_affects_descendants change)
    then
      acc_fanout Dep.Member.all Modified to_recheck
    else
      to_recheck
  in
  {
    Fanout.changed = DepSet.singleton class_dep;
    to_recheck;
    to_recheck_if_errors = DepSet.make ();
  }

let get_minor_change_fanout_legacy
    ~(ctx : Provider_context.t)
    (class_dep : Dep.t)
    (member_diff : ClassDiff.member_diff) : Fanout.t =
  let mode = Provider_context.get_deps_mode ctx in
  let changed = DepSet.singleton class_dep in
  let acc = DepSet.singleton class_dep in
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
  let recheck_descendants_and_their_member_dependents (acc : DepSet.t) member :
      DepSet.t =
    let changed_and_descendants = Lazy.force changed_and_descendants in
    DepSet.fold changed_and_descendants ~init:acc ~f:(fun dep acc ->
        DepSet.add acc dep
        |> include_fanout_of_dep
             mode
             (Typing_deps.Dep.make_member_dep_from_type_dep dep member))
  in
  let add_member_fanout
      ~is_const
      (member : Dep.Member.t)
      (change : member_change)
      (acc : DepSet.t) =
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
      include_fanout_of_dep
        mode
        (Dep.make_member_dep_from_type_dep class_dep member)
        acc
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
  {
    Fanout.changed = DepSet.singleton class_dep;
    to_recheck = acc;
    to_recheck_if_errors = DepSet.make ();
  }

(** This is all descendants + anything that depends on any descendants. *)

let get_all_deps (ctx : Provider_context.t) (class_dep : Dep.t) : DepSet.t =
  let mode = Provider_context.get_deps_mode ctx in
  Typing_deps.add_all_deps mode @@ DepSet.singleton class_dep

let get_maximum_fanout (ctx : Provider_context.t) (class_dep : Dep.t) : Fanout.t
    =
  {
    Fanout.changed = DepSet.singleton class_dep;
    Fanout.to_recheck = get_all_deps ctx class_dep;
    Fanout.to_recheck_if_errors = DepSet.make ();
  }

(** Get the fanout of an added parent. *)
let get_added_parent_fanout
    ctx class_dep classish_kind added_parent_name fanout_acc : Fanout.t =
  (* This function is based on reasoning about how basic decl facts are altered
   * by adding a parent.
   * The kinds of facts we consider are:
   * - subtyping relationships: t1 <: t2
   * - member types: X::m : t
   * Adding a parent Y to a type X should:
   * - Add new subtyping relationships, but not remove any.
   *   And therefore there should be no new subtyping errors.
   * - For each member m in Y, possibly change the type of X::m.
   *   The fanout should therefore include the fanout of X::m.
   *
   * So conclusion: fanout = union over m in Y of fanout(X::m)
   * Which is what this function implements *)
  match Decl_provider.get_class ctx added_parent_name with
  | None -> fanout_acc
  | Some (cls : Decl_provider.class_decl) ->
    let { Fanout.changed; to_recheck; to_recheck_if_errors } = fanout_acc in
    let acc_fanout member = acc_member_fanout ctx class_dep member Added in
    let acc_fanouts make_member members =
      acc_member_fanouts
        ctx
        class_dep
        make_member
        (List.map members ~f:(fun m -> (fst m, Added)) |> SMap.of_list)
    in
    let consts =
      Decl_provider.Class.consts cls
      |> List.filter ~f:(fun (name, _) ->
             not @@ String.equal name Naming_special_names.Members.mClass)
    in
    let deps_acc =
      to_recheck
      |> acc_fanouts Dep.Member.method_ (Decl_provider.Class.methods cls)
      |> acc_fanouts Dep.Member.smethod (Decl_provider.Class.smethods cls)
      |> acc_fanouts Dep.Member.prop (Decl_provider.Class.props cls)
      |> acc_fanouts Dep.Member.sprop (Decl_provider.Class.sprops cls)
      |> acc_fanouts Dep.Member.const consts
      |> acc_fanouts Dep.Member.const (Decl_provider.Class.typeconsts cls)
    in
    let deps_acc =
      let (construct, _consistent_kind) = Decl_provider.Class.construct cls in
      Option.fold construct ~init:deps_acc ~f:(fun deps_acc _construct ->
          acc_fanout Dep.Member.constructor deps_acc)
    in
    let deps_acc =
      if Ast_defs.is_c_enum classish_kind && (not @@ List.is_empty consts) then
        acc_fanout Dep.Member.all deps_acc
      else
        deps_acc
    in
    {
      Fanout.changed = DepSet.add changed class_dep;
      to_recheck = deps_acc;
      to_recheck_if_errors =
        DepSet.union to_recheck_if_errors (get_all_deps ctx class_dep);
    }

let get_added_parents_fanout
    ctx class_dep classish_kind added_parents member_diff : Fanout.t =
  let deps = get_minor_change_fanout ~ctx class_dep member_diff in
  let deps =
    SSet.fold
      added_parents
      ~init:deps
      ~f:(get_added_parent_fanout ctx class_dep classish_kind)
  in
  deps

exception Do_max_fanout

let get_parent_changes_fanout
    ctx
    class_dep
    classish_kind
    (parent_changes : ClassDiff.parent_changes)
    (member_diff : ClassDiff.member_diff) : Fanout.t =
  let {
    ClassDiff.extends_changes;
    implements_changes;
    req_extends_changes;
    req_implements_changes;
    req_class_changes;
    uses_changes;
    xhp_attr_changes;
  } =
    parent_changes
  in
  try
    (* If any parent was modified or removed, then some subtyping relationships are invalidated
     * and we must return max fanout.
     * Otherwise if we're only dealing with added parents, we can calculate a smaller fanout.
     * Let's detect removed/modified parents sooner rather than later, by going over the list
     * once first, and collect all the added parents at the same occasion. *)
    let added_parents =
      List.fold
        [
          extends_changes;
          implements_changes;
          req_extends_changes;
          req_implements_changes;
          req_class_changes;
          uses_changes;
          xhp_attr_changes;
        ]
        ~init:SSet.empty
        ~f:(fun added_parents changes ->
          Option.fold
            changes
            ~init:added_parents
            ~f:(fun added_parents (change : _ ClassDiff.NamedItemsListChange.t)
               ->
              let {
                ClassDiff.NamedItemsListChange.per_name_changes;
                order_change;
              } =
                change
              in
              if order_change then
                (* Any change order can invalidate subtyping relationships
                 * Due to the way we handle multiple instantiation inheritence. *)
                raise Do_max_fanout
              else
                SMap.fold
                  per_name_changes
                  ~init:added_parents
                  ~f:(fun name change added_parents ->
                    match change with
                    | ClassDiff.ValueChange.(Modified _ | Removed) ->
                      raise Do_max_fanout
                    | ClassDiff.ValueChange.Added -> SSet.add added_parents name)))
    in
    get_added_parents_fanout
      ctx
      class_dep
      classish_kind
      added_parents
      member_diff
  with
  | Do_max_fanout -> get_maximum_fanout ctx class_dep

let get_shell_change_fanout
    ctx
    class_dep
    (shell_change : ClassDiff.class_shell_change)
    (member_diff : ClassDiff.member_diff) : Fanout.t =
  let {
    ClassDiff.classish_kind;
    parent_changes;
    type_parameters_change;
    kind_change;
    final_change;
    abstract_change;
    is_xhp_change;
    internal_change;
    has_xhp_keyword_change;
    support_dynamic_type_change;
    module_change;
    xhp_enum_values_change;
    user_attributes_changes;
    enum_type_change;
  } =
    shell_change
  in
  if
    Option.is_some type_parameters_change
    || Option.is_some kind_change
    || Option.is_some final_change
    || Option.is_some abstract_change
    || Option.is_some is_xhp_change
    || Option.is_some internal_change
    || Option.is_some has_xhp_keyword_change
    || Option.is_some support_dynamic_type_change
    || Option.is_some module_change
    || xhp_enum_values_change
    || Option.is_some user_attributes_changes
    || Option.is_some enum_type_change
  then
    get_maximum_fanout ctx class_dep
  else
    match parent_changes with
    | Some parent_changes ->
      get_parent_changes_fanout
        ctx
        class_dep
        classish_kind
        parent_changes
        member_diff
    | None ->
      (* If we get here, we have a major change but don't know what caused it.
       * Let's play safe and return maximum fanout.
       * This case would be observable in our telemetry anyway. *)
      get_maximum_fanout ctx class_dep

let get_major_change_fanout ctx class_dep (change : ClassDiff.MajorChange.t) :
    Fanout.t =
  match change with
  | ClassDiff.MajorChange.(Added | Removed | Unknown) ->
    get_maximum_fanout ctx class_dep
  | ClassDiff.MajorChange.Modified (shell_change, member_diff) ->
    get_shell_change_fanout ctx class_dep shell_change member_diff

let get_fanout ~(ctx : Provider_context.t) (class_name, diff) : Fanout.t =
  let class_dep = Dep.make (Dep.Type class_name) in
  match diff with
  | Unchanged -> Fanout.empty
  | Major_change change ->
    if
      TypecheckerOptions.optimized_parent_fanout
        (Provider_context.get_tcopt ctx)
    then
      get_major_change_fanout ctx class_dep change
    else
      get_maximum_fanout ctx class_dep
  | Minor_change minor_change ->
    if
      TypecheckerOptions.optimized_member_fanout
        (Provider_context.get_tcopt ctx)
    then
      get_minor_change_fanout ~ctx class_dep minor_change
    else
      get_minor_change_fanout_legacy ~ctx class_dep minor_change

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

module Log : sig
  val log_class_fanout :
    Provider_context.t -> string * ClassDiff.t -> Fanout.t -> unit

  val log_fanout :
    Provider_context.t ->
    'a list ->
    Fanout.t ->
    max_class_fanout_cardinal:int ->
    unit
end = struct
  let do_log ctx ~fanout_cardinal =
    TypecheckerOptions.log_fanout ~fanout_cardinal
    @@ Provider_context.get_tcopt ctx

  let log_class_fanout
      ctx ((class_name : string), (diff : ClassDiff.t)) (fanout : Fanout.t) :
      unit =
    let fanout_cardinal = Fanout.cardinal fanout in
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
      ctx (changes : _ list) (fanout : Fanout.t) ~max_class_fanout_cardinal :
      unit =
    let fanout_cardinal = Fanout.cardinal fanout in
    if do_log ~fanout_cardinal ctx then
      HackEventLogger.Fanouts.log
        ~changes_cardinal:(List.length changes)
        ~max_class_fanout_cardinal
        ~fanout_cardinal
end

let add_fanout
    ~ctx
    ((fanout_acc, max_class_fanout_cardinal) : Fanout.t * int)
    (diff : string * ClassDiff.t) : Fanout.t * int =
  let fanout = get_fanout ~ctx diff in
  Log.log_class_fanout ctx diff fanout;
  let fanout_acc = Fanout.union fanout_acc fanout in
  let max_class_fanout_cardinal =
    Int.max max_class_fanout_cardinal (Fanout.cardinal fanout)
  in
  (fanout_acc, max_class_fanout_cardinal)

let fanout_of_changes
    ~(ctx : Provider_context.t) (changes : (string * ClassDiff.t) list) :
    Fanout.t =
  let (fanout, max_class_fanout_cardinal) =
    List.fold changes ~init:(Fanout.empty, 0) ~f:(add_fanout ~ctx)
  in
  Log.log_fanout ctx changes fanout ~max_class_fanout_cardinal;
  fanout
