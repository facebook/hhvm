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

type changed_class = {
  name: string;
  diff: ClassDiff.t;
  dep: Dep.t;
  descendant_deps: Typing_deps.DepSet.t;
}

let get_all_deps (ctx : Provider_context.t) (changed_class : changed_class) :
    DepSet.t =
  let mode = Provider_context.get_deps_mode ctx in
  Typing_deps.add_typing_deps mode changed_class.descendant_deps

let get_maximum_fanout
    (ctx : Provider_context.t) (changed_class : changed_class) : Fanout.t =
  {
    Fanout.changed = DepSet.singleton changed_class.dep;
    to_recheck = get_all_deps ctx changed_class;
    to_recheck_if_errors = DepSet.make ();
  }

module FanoutMonad = struct
  type t =
    | Max_fanout
    | Fanout of Fanout.t

  let bind (fanout : t) (f : Fanout.t -> t) : t =
    match fanout with
    | Max_fanout -> Max_fanout
    | Fanout fanout -> f fanout

  let map (fanout : t) (f : Fanout.t -> Fanout.t) : t =
    match fanout with
    | Max_fanout -> Max_fanout
    | Fanout fanout -> Fanout (f fanout)

  let to_fanout ctx changed_class t =
    match t with
    | Fanout fanout -> fanout
    | Max_fanout -> get_maximum_fanout ctx changed_class

  module Infix = struct
    let ( >>= ) = bind

    let ( >>| ) = map
  end
end

module FM = FanoutMonad

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
    (changed_class : changed_class)
    (member_diff : ClassDiff.member_diff)
    (fanout_acc : Fanout.t) : Fanout.t =
  let { consts; typeconsts; props; sprops; methods; smethods; constructor } =
    member_diff
  in
  let { Fanout.changed; to_recheck; to_recheck_if_errors } = fanout_acc in
  let acc_fanout = acc_member_fanout ctx changed_class.dep in
  let acc_fanouts make_member changes fanout_acc =
    acc_member_fanouts ctx changed_class.dep make_member changes fanout_acc
  in
  let to_recheck =
    DepSet.add to_recheck changed_class.dep
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
    Fanout.changed = DepSet.add changed changed_class.dep;
    to_recheck;
    to_recheck_if_errors;
  }

let get_minor_change_fanout_legacy
    ~(ctx : Provider_context.t)
    (changed_class : changed_class)
    (member_diff : ClassDiff.member_diff) : Fanout.t =
  let mode = Provider_context.get_deps_mode ctx in
  let acc = DepSet.singleton changed_class.dep in
  let { consts; typeconsts; props; sprops; methods; smethods; constructor } =
    member_diff
  in
  (* Recheck any file with a dependency on the provided member
     in the changed class and in each of its descendants,
     even if the member was overridden in a subclass. This deals with the case
     where adding, removing, or changing the abstract-ness of a member causes
     some subclass which inherits a member of that name from multiple parents
     to resolve the conflict in a different way than it did previously. *)
  let recheck_descendants_and_their_member_dependents (acc : DepSet.t) member :
      DepSet.t =
    DepSet.fold changed_class.descendant_deps ~init:acc ~f:(fun dep acc ->
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
        (Dep.make_member_dep_from_type_dep changed_class.dep member)
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
    Fanout.changed = DepSet.singleton changed_class.dep;
    to_recheck = acc;
    to_recheck_if_errors = DepSet.make ();
  }

(** This is all descendants + anything that depends on any descendants. *)

(** Get the fanout of an added parent. *)
let get_added_parent_fanout
    ctx
    (changed_class : changed_class)
    classish_kind
    added_parent_name
    fanout_acc : Fanout.t =
  match Decl_provider.get_class ctx added_parent_name with
  | Decl_entry.NotYetAvailable
  | Decl_entry.DoesNotExist ->
    fanout_acc
  | Decl_entry.Found (cls : Decl_provider.class_decl) ->
    let { Fanout.changed; to_recheck; to_recheck_if_errors } = fanout_acc in
    let { name = _; diff = _; dep; descendant_deps } = changed_class in
    (* The type check performs two categories of checks:
     * - function/method body checks
     * - wellformedness/hierarchy checks
     *
     * We need to reason about both to figure out fanouts.
     *
     * # Hierarchy checks
     *
     * For hierarchy checks, precise fanout computation is
     * tricky because there are a wide variety of checks.
     * Most checks only consider information declared in direct parents,
     * but some checks use information declared in non-direct ancestors,
     * for example:
     * - member override checks
     * - requirement checks (e.g. `require extends`)
     * - constructor consistency checks (using attribute __ConsistentConstruct
     *   from remote ancestors)
     * - dynamic support checks
     * - multiple instantiation inheritence checks
     *
     * Since it's hard to keep track of those checks and more may be added
     * in the future, we simply include all descendants in the fanout.
     *)
    let to_recheck = DepSet.union descendant_deps to_recheck in
    (* # Function body checks
     *
     * Function body checks use two kinds of basic decl facts
     * that can be affected by parent changes:
     * - subtyping relationships: t1 <: t2
     * - member types: X::m : t
     * Adding a parent Y to a type X should:
     * - Add new subtyping relationships, but not remove any.
     *   And therefore there should be no new subtyping errors.
     * - For each member m in Y, possibly change the type of X::m.
     *   The fanout should therefore include the fanout of X::m.
     *
     * So conclusion: fanout = union over m in Y of fanout(X::m)
     * Which is what this function implements
     *)
    let acc_fanout member = acc_member_fanout ctx dep member Added in
    let acc_fanouts make_member members =
      acc_member_fanouts
        ctx
        dep
        make_member
        (List.map members ~f:(fun m -> (fst m, Added)) |> SMap.of_list)
    in
    let consts =
      Decl_provider.Class.consts cls
      |> List.filter ~f:(fun (name, _) ->
             not @@ String.equal name Naming_special_names.Members.mClass)
    in
    let to_recheck =
      to_recheck
      |> acc_fanouts Dep.Member.method_ (Decl_provider.Class.methods cls)
      |> acc_fanouts Dep.Member.smethod (Decl_provider.Class.smethods cls)
      |> acc_fanouts Dep.Member.prop (Decl_provider.Class.props cls)
      |> acc_fanouts Dep.Member.sprop (Decl_provider.Class.sprops cls)
      |> acc_fanouts Dep.Member.const consts
      |> acc_fanouts Dep.Member.const (Decl_provider.Class.typeconsts cls)
    in
    let to_recheck =
      let (construct, _consistent_kind) = Decl_provider.Class.construct cls in
      Option.fold construct ~init:to_recheck ~f:(fun to_recheck _construct ->
          acc_fanout Dep.Member.constructor to_recheck)
    in
    let to_recheck =
      if Ast_defs.is_c_enum classish_kind && (not @@ List.is_empty consts) then
        acc_fanout Dep.Member.all to_recheck
      else
        to_recheck
    in
    (* Caveat:
     * Some typing derivations actually use facts like
     * 'A is not a subtype of B', for example when inferring that two types
     * are disjoint and a refinement results in type 'nothing'.
     * These basics facts may be invalidated by adding parents.
     * To handle this case,
     * - whenever we fact 'A is not a subtype of X' for any X,
     *   we add a 'NotSubtype' edge from A.
     * - when detecting an added parent to A, we gather descendants of
     *   A via Extends / RequireExtends edges, then follow any NotSubtype
     *   edge from these descendants and collect these edges' ends to add to
     *   the fanout. (This is done by the call to get_not_subtype_fanout below)
     *)
    let to_recheck =
      Typing_deps.get_not_subtype_fanout
        (Provider_context.get_deps_mode ctx)
        ~descendant_deps
        to_recheck
    in
    {
      Fanout.changed = DepSet.add changed dep;
      to_recheck;
      to_recheck_if_errors =
        DepSet.union to_recheck_if_errors (get_all_deps ctx changed_class);
    }

let get_added_parents_fanout
    ctx (changed_class : changed_class) classish_kind added_parents fanout_acc :
    Fanout.t =
  SSet.fold
    added_parents
    ~init:fanout_acc
    ~f:(get_added_parent_fanout ctx changed_class classish_kind)

let get_parent_changes_fanout
    ctx
    (changed_class : changed_class)
    classish_kind
    (parent_changes : ClassDiff.parent_changes option)
    (fanout_acc : Fanout.t) : FM.t =
  match parent_changes with
  | None -> FM.Fanout fanout_acc
  | Some parent_changes ->
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
    (* If any parent was modified or removed, then some subtyping relationships are invalidated
     * and we must return max fanout.
     * Otherwise if we're only dealing with added parents, we can calculate a smaller fanout.
     * Let's detect removed/modified parents sooner rather than later, by going over the list
     * once first, and collect all the added parents at the same occasion. *)
    let (added_parents, removed_parents) =
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
        ~init:(SSet.empty, SSet.empty)
        ~f:(fun (added_parents, removed_parents) changes ->
          Option.fold
            changes
            ~init:(added_parents, removed_parents)
            ~f:(fun
                 (added_parents, removed_parents)
                 (change : _ ClassDiff.NamedItemsListChange.t)
               ->
              let {
                ClassDiff.NamedItemsListChange.per_name_changes;
                order_change = _;
              } =
                change
              in
              SMap.fold
                per_name_changes
                ~init:(added_parents, removed_parents)
                ~f:(fun name change (added_parents, removed_parents) ->
                  match change with
                  | ClassDiff.ValueChange.(Modified _ | Removed) ->
                    let removed_parents = SSet.add removed_parents name in
                    (added_parents, removed_parents)
                  | ClassDiff.ValueChange.Added ->
                    let added_parents = SSet.add added_parents name in
                    (added_parents, removed_parents))))
    in
    if not (SSet.is_empty removed_parents) then
      FM.Max_fanout
    else
      FM.Fanout
        (get_added_parents_fanout
           ctx
           changed_class
           classish_kind
           added_parents
           fanout_acc)

let all_or_nothing_bool (has_changed : bool) fanout_acc =
  if has_changed then
    FM.Max_fanout
  else
    FM.Fanout fanout_acc

let all_or_nothing (type change) (change_opt : change option) fanout_acc =
  match change_opt with
  | None -> FM.Fanout fanout_acc
  | Some _ -> FM.Max_fanout

let get_shell_change_fanout
    ctx
    (changed_class : changed_class)
    (shell_change : ClassDiff.class_shell_change)
    (member_diff : ClassDiff.member_diff)
    (fanout_acc : Fanout.t) : FM.t =
  let {
    ClassDiff.old_classish_kind;
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
  let open FM.Infix in
  fanout_acc
  |> all_or_nothing type_parameters_change
  >>= all_or_nothing kind_change
  >>= all_or_nothing final_change
  >>= all_or_nothing abstract_change
  >>= all_or_nothing is_xhp_change
  >>= all_or_nothing internal_change
  >>= all_or_nothing has_xhp_keyword_change
  >>= all_or_nothing support_dynamic_type_change
  >>= all_or_nothing module_change
  >>= all_or_nothing enum_type_change
  >>= all_or_nothing user_attributes_changes
  >>= all_or_nothing_bool xhp_enum_values_change
  >>= get_parent_changes_fanout
        ctx
        changed_class
        old_classish_kind
        parent_changes
  >>| get_minor_change_fanout ~ctx changed_class member_diff

let get_major_change_fanout
    ctx
    (changed_class : changed_class)
    (change : ClassDiff.MajorChange.t)
    fanout_acc : FM.t =
  match change with
  | ClassDiff.MajorChange.(Added | Removed | Unknown _) -> FM.Max_fanout
  | ClassDiff.MajorChange.Modified (shell_change, member_diff) ->
    get_shell_change_fanout
      ctx
      changed_class
      shell_change
      member_diff
      fanout_acc

let get_fanout ~(ctx : Provider_context.t) (changed_class : changed_class) :
    FM.t =
  let fanout_acc = Fanout.empty in
  match changed_class.diff with
  | Major_change change ->
    if
      TypecheckerOptions.optimized_parent_fanout
        (Provider_context.get_tcopt ctx)
    then
      get_major_change_fanout ctx changed_class change fanout_acc
    else
      FM.Max_fanout
  | Minor_change minor_change ->
    let fanout =
      if
        TypecheckerOptions.optimized_member_fanout
          (Provider_context.get_tcopt ctx)
      then
        get_minor_change_fanout ~ctx changed_class minor_change fanout_acc
      else
        get_minor_change_fanout_legacy ~ctx changed_class minor_change
    in
    FM.Fanout fanout

let direct_references_cardinal mode dep : int =
  Typing_deps.get_ideps_from_hash mode dep |> DepSet.cardinal

let descendants_cardinal descendant_deps : int =
  DepSet.cardinal descendant_deps - 1

let children_cardinal mode class_name : int =
  Typing_deps.get_ideps mode (Dep.Extends class_name) |> DepSet.cardinal

module Log : sig
  val log_class_fanout : Provider_context.t -> changed_class -> Fanout.t -> unit

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

  let log_class_fanout ctx (changed_class : changed_class) (fanout : Fanout.t) :
      unit =
    let { name; diff; dep; descendant_deps } = changed_class in
    let fanout_cardinal = Fanout.cardinal fanout in
    if do_log ~fanout_cardinal ctx then
      let mode = Provider_context.get_deps_mode ctx in
      HackEventLogger.Fanouts.log_class
        ~class_name:name
        ~class_diff:(ClassDiff.show diff)
        ~fanout_cardinal
        ~class_diff_category:(ClassDiff.to_category_json diff)
        ~direct_references_cardinal:(direct_references_cardinal mode dep)
        ~descendants_cardinal:(descendants_cardinal descendant_deps)
        ~children_cardinal:(children_cardinal mode name)

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
    (changed_class : changed_class) : Fanout.t * int =
  let fanout =
    get_fanout ~ctx changed_class |> FM.to_fanout ctx changed_class
  in
  Log.log_class_fanout ctx changed_class fanout;
  let fanout_acc = Fanout.union fanout_acc fanout in
  let max_class_fanout_cardinal =
    Int.max max_class_fanout_cardinal (Fanout.cardinal fanout)
  in
  (fanout_acc, max_class_fanout_cardinal)

let fanout_of_changes ~(ctx : Provider_context.t) (changes : changed_class list)
    : Fanout.t =
  let (fanout, max_class_fanout_cardinal) =
    List.fold changes ~init:(Fanout.empty, 0) ~f:(add_fanout ~ctx)
  in
  Log.log_fanout ctx changes fanout ~max_class_fanout_cardinal;
  fanout
