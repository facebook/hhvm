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
module SN = Naming_special_names

module Parents = struct
  type t = {
    extends: Typing_defs.decl_ty list;
    implements: Typing_defs.decl_ty list;
    req_extends: Typing_defs.decl_ty list;
    req_implements: Typing_defs.decl_ty list;
    req_class: Typing_defs.decl_ty list;
    uses: Typing_defs.decl_ty list;
    xhp_attr_uses: Typing_defs.decl_ty list;
  }
  [@@deriving eq]

  let of_shallow_class c =
    let {
      sc_extends;
      sc_implements;
      sc_req_extends;
      sc_req_implements;
      sc_uses;
      sc_req_class;
      sc_xhp_attr_uses;
      sc_mode = _;
      sc_final = _;
      sc_abstract = _;
      sc_is_xhp = _;
      sc_internal = _;
      sc_has_xhp_keyword = _;
      sc_kind = _;
      sc_module = _;
      sc_name = _;
      sc_tparams = _;
      sc_where_constraints = _;
      sc_xhp_enum_values = _;
      sc_xhp_marked_empty = _;
      sc_support_dynamic_type = _;
      sc_consts = _;
      sc_typeconsts = _;
      sc_props = _;
      sc_sprops = _;
      sc_constructor = _;
      sc_static_methods = _;
      sc_methods = _;
      sc_user_attributes = _;
      sc_enum_type = _;
      sc_docs_url = _;
    } =
      c
    in
    {
      extends = sc_extends;
      implements = sc_implements;
      req_extends = sc_req_extends;
      req_implements = sc_req_implements;
      req_class = sc_req_class;
      uses = sc_uses;
      xhp_attr_uses = sc_xhp_attr_uses;
    }
end

let merge_member_lists
    (get_name : 'member -> string) (l1 : 'member list) (l2 : 'member list) :
    ('member option * 'member option) SMap.t =
  (* When a member of a given name is declared multiple times, keep the first
     (as Decl_inheritance does). *)
  let map =
    List.fold l1 ~init:SMap.empty ~f:(fun map x ->
        let name = get_name x in
        if SMap.mem map name then
          map
        else
          SMap.add map ~key:name ~data:(Some x, None))
  in
  List.fold l2 ~init:map ~f:(fun map y ->
      let name = get_name y in
      match SMap.find_opt map name with
      | Some (x, None) -> SMap.add map ~key:name ~data:(x, Some y)
      | None -> SMap.add map ~key:name ~data:(None, Some y)
      | Some (_, Some _) -> map)

module type Member_S = sig
  type t

  val diff : t -> t -> member_change option

  val is_private : t -> bool

  val is_internal : t -> bool

  (** Whether adding this member implies that the constructor and the
    constructor of all descendants should be considered modified.
    This is the case for example for required XHP attributes. *)
  val constructor_is_changed_inheritance_when_added : t -> bool

  (** Whether modifying this member implies that the constructor and the
    constructor of all descendants should be considered modified.
    This is the case for example for required XHP attributes. *)
  val constructor_is_changed_inheritance_when_modified : old:t -> new_:t -> bool
end

(** Returns the diff of two member lists, plus a diff on the constructor if the member changes
  impact the constructor. *)
let diff_members
    (type member)
    (members_left_right : (member option * member option) SMap.t)
    (module Member : Member_S with type t = member)
    (classish_kind : Ast_defs.classish_kind)
    (module_changed : bool) : member_change SMap.t * constructor_change =
  (* If both members are internal and the module changed, we have to treat it as a Modified change*)
  let check_module_change_internal m1 m2 diff =
    match diff with
    | None when module_changed && Member.is_internal m1 && Member.is_internal m2
      ->
      Some Modified
    | None
    | Some _ ->
      diff
  in
  SMap.fold
    members_left_right
    ~init:(SMap.empty, None)
    ~f:(fun name old_and_new (diff, constructor_change) ->
      match old_and_new with
      | (None, None) -> failwith "merge_member_lists added (None, None)"
      | (Some member, None)
      | (None, Some member)
        when (not Ast_defs.(is_c_trait classish_kind))
             && Member.is_private member ->
        (SMap.add diff ~key:name ~data:Private_change, constructor_change)
      | (Some _, None) ->
        (SMap.add diff ~key:name ~data:Removed, constructor_change)
      | (None, Some m) ->
        let constructor_change =
          max_constructor_change
            constructor_change
            (if Member.constructor_is_changed_inheritance_when_added m then
              Some Changed_inheritance
            else
              None)
        in
        (SMap.add diff ~key:name ~data:Added, constructor_change)
      | (Some old_member, Some new_member) ->
        let member_changes =
          Member.diff old_member new_member
          |> check_module_change_internal old_member new_member
          |> Option.value_map ~default:diff ~f:(fun ch ->
                 SMap.add diff ~key:name ~data:ch)
        in
        let constructor_change =
          max_constructor_change
            constructor_change
            (if
             Member.constructor_is_changed_inheritance_when_modified
               ~old:old_member
               ~new_:new_member
            then
              Some Changed_inheritance
            else
              None)
        in
        (member_changes, constructor_change))

module ClassConst : Member_S with type t = shallow_class_const = struct
  type t = shallow_class_const

  let diff (c1 : shallow_class_const) c2 : member_change option =
    let c1 = Decl_pos_utils.NormalizeSig.shallow_class_const c1 in
    let c2 = Decl_pos_utils.NormalizeSig.shallow_class_const c2 in
    if equal_shallow_class_const c1 c2 then
      None
    else if
      not (Typing_defs.equal_class_const_kind c1.scc_abstract c2.scc_abstract)
    then
      Some Changed_inheritance
    else
      Some Modified

  let is_private _ = false

  let is_internal _ = false

  let constructor_is_changed_inheritance_when_added _ = false

  let constructor_is_changed_inheritance_when_modified ~old:_ ~new_:_ = false
end

module TypeConst : Member_S with type t = shallow_typeconst = struct
  type t = shallow_typeconst

  let diff tc1 tc2 : member_change option =
    let tc1 = Decl_pos_utils.NormalizeSig.shallow_typeconst tc1 in
    let tc2 = Decl_pos_utils.NormalizeSig.shallow_typeconst tc2 in
    if equal_shallow_typeconst tc1 tc2 then
      None
    else
      let open Typing_defs in
      match (tc1.stc_kind, tc2.stc_kind) with
      | (TCAbstract _, TCAbstract _)
      | (TCConcrete _, TCConcrete _) ->
        Some Modified
      | (_, (TCAbstract _ | TCConcrete _)) -> Some Changed_inheritance

  let is_private _ = false

  let is_internal _ = false

  let constructor_is_changed_inheritance_when_added _ = false

  let constructor_is_changed_inheritance_when_modified ~old:_ ~new_:_ = false
end

module Prop : Member_S with type t = shallow_prop = struct
  type t = shallow_prop

  let diff p1 p2 : member_change option =
    let p1 = Decl_pos_utils.NormalizeSig.shallow_prop p1 in
    let p2 = Decl_pos_utils.NormalizeSig.shallow_prop p2 in
    if equal_shallow_prop p1 p2 then
      None
    else if
      (not (Aast.equal_visibility p1.sp_visibility p2.sp_visibility))
      || Bool.( <> ) (sp_abstract p1) (sp_abstract p2)
    then
      Some Changed_inheritance
    else
      Some Modified

  let is_private p : bool =
    Aast_defs.equal_visibility p.sp_visibility Aast_defs.Private

  let is_internal p : bool =
    Aast_defs.equal_visibility p.sp_visibility Aast_defs.Internal

  let constructor_is_changed_inheritance_when_added p =
    Xhp_attribute.opt_is_required p.sp_xhp_attr

  let constructor_is_changed_inheritance_when_modified ~old ~new_ =
    Xhp_attribute.opt_is_required new_.sp_xhp_attr
    && (not @@ Xhp_attribute.opt_is_required old.sp_xhp_attr)
end

module Method : Member_S with type t = shallow_method = struct
  type t = shallow_method

  let diff m1 m2 : member_change option =
    let m1 = Decl_pos_utils.NormalizeSig.shallow_method m1 in
    let m2 = Decl_pos_utils.NormalizeSig.shallow_method m2 in
    if equal_shallow_method m1 m2 then
      None
    else if
      (not (Aast.equal_visibility m1.sm_visibility m2.sm_visibility))
      || Bool.( <> ) (sm_abstract m1) (sm_abstract m2)
    then
      Some Changed_inheritance
    else
      Some Modified

  let is_private m : bool =
    Aast_defs.equal_visibility m.sm_visibility Aast_defs.Private

  let is_internal m : bool =
    Aast_defs.equal_visibility m.sm_visibility Aast_defs.Internal

  let constructor_is_changed_inheritance_when_added _ = false

  let constructor_is_changed_inheritance_when_modified ~old:_ ~new_:_ = false
end

let diff_constructor old_cls new_cls old_cstr new_cstr : member_change option =
  let consistent1 = Decl_utils.consistent_construct_kind old_cls in
  let consistent2 = Decl_utils.consistent_construct_kind new_cls in
  if not (Typing_defs.equal_consistent_kind consistent1 consistent2) then
    Some Changed_inheritance
  else
    match (old_cstr, new_cstr) with
    | (None, None) -> None
    | (Some _, None) -> Some Removed
    | (None, Some _) -> Some Added
    | (Some old_method, Some new_method) -> Method.diff old_method new_method

let diff_class_members (c1 : shallow_class) (c2 : shallow_class) :
    ClassDiff.member_diff =
  let diff = ClassDiff.empty_member_diff in
  let kind = c2.sc_kind in
  let module_changed =
    match (c1.sc_module, c2.sc_module) with
    | (Some (_, m1), Some (_, m2)) when String.equal m1 m2 -> false
    | (None, None) -> false
    | _ -> true
  in
  let diff =
    let get_name x = snd x.scc_name in
    let consts = merge_member_lists get_name c1.sc_consts c2.sc_consts in
    let (consts, constructor_change) =
      diff_members
        consts
        (module ClassConst : Member_S with type t = shallow_class_const)
        kind
        module_changed
    in
    {
      diff with
      consts;
      constructor = max_constructor_change diff.constructor constructor_change;
    }
  in
  let diff =
    let get_name x = snd x.stc_name in
    let typeconsts =
      merge_member_lists get_name c1.sc_typeconsts c2.sc_typeconsts
    in
    let (typeconsts, constructor_change) =
      diff_members
        typeconsts
        (module TypeConst : Member_S with type t = shallow_typeconst)
        kind
        module_changed
    in
    {
      diff with
      typeconsts;
      constructor = max_constructor_change diff.constructor constructor_change;
    }
  in
  let diff =
    let get_name x = snd x.sp_name in
    let props = merge_member_lists get_name c1.sc_props c2.sc_props in
    let (props, constructor_change) =
      diff_members
        props
        (module Prop : Member_S with type t = shallow_prop)
        kind
        module_changed
    in
    {
      diff with
      props;
      constructor = max_constructor_change diff.constructor constructor_change;
    }
  in
  let diff =
    let get_name x = snd x.sp_name in
    let sprops = merge_member_lists get_name c1.sc_sprops c2.sc_sprops in
    let (sprops, constructor_change) =
      diff_members
        sprops
        (module Prop : Member_S with type t = shallow_prop)
        kind
        module_changed
    in
    {
      diff with
      sprops;
      constructor = max_constructor_change diff.constructor constructor_change;
    }
  in
  let diff =
    let get_name x = snd x.sm_name in
    let methods = merge_member_lists get_name c1.sc_methods c2.sc_methods in
    let (methods, constructor_change) =
      diff_members
        methods
        (module Method : Member_S with type t = shallow_method)
        kind
        module_changed
    in
    {
      diff with
      methods;
      constructor = max_constructor_change diff.constructor constructor_change;
    }
  in
  let diff =
    let get_name x = snd x.sm_name in
    let smethods =
      merge_member_lists get_name c1.sc_static_methods c2.sc_static_methods
    in
    let (smethods, constructor_change) =
      diff_members
        smethods
        (module Method : Member_S with type t = shallow_method)
        kind
        module_changed
    in
    {
      diff with
      smethods;
      constructor = max_constructor_change diff.constructor constructor_change;
    }
  in
  let diff =
    let constructor =
      diff_constructor c1 c2 c1.sc_constructor c2.sc_constructor
    in
    {
      diff with
      constructor = max_constructor_change diff.constructor constructor;
    }
  in
  diff

(** Return true if the two classes would (shallowly) produce the same member
    resolution order (the output of the Decl_linearize module).

    NB: It is critical for the correctness of incremental typechecking that
    every bit of information used by Decl_linearize is checked here! *)
let mro_inputs_equal (c1 : shallow_class) (c2 : shallow_class) : bool =
  let is_to_string m = String.equal (snd m.sm_name) SN.Members.__toString in
  Typing_defs.equal_pos_id c1.sc_name c2.sc_name
  && Ast_defs.equal_classish_kind c1.sc_kind c2.sc_kind
  && Option.equal
       equal_shallow_method
       (List.find c1.sc_methods ~f:is_to_string)
       (List.find c2.sc_methods ~f:is_to_string)
  && List.equal Poly.( = ) c1.sc_tparams c2.sc_tparams
  && List.equal Poly.( = ) c1.sc_extends c2.sc_extends
  && List.equal Poly.( = ) c1.sc_implements c2.sc_implements
  && List.equal Poly.( = ) c1.sc_uses c2.sc_uses
  && List.equal Poly.( = ) c1.sc_req_extends c2.sc_req_extends
  && List.equal Poly.( = ) c1.sc_req_implements c2.sc_req_implements
  && List.equal Poly.( = ) c1.sc_xhp_attr_uses c2.sc_xhp_attr_uses

(* The ConsistentConstruct attribute is propagated down the inheritance
   hierarchy, so we can model it with Changed_inheritance on the constructor. To
   do so, though, we need to remove the attribute while normalizing classes (so
   that we don't consider it a Major_change). *)
let remove_consistent_construct_attribute sc =
  {
    sc with
    sc_user_attributes =
      List.filter sc.sc_user_attributes ~f:(fun ua ->
          not
            (String.equal
               (snd ua.Typing_defs.ua_name)
               SN.UserAttributes.uaConsistentConstruct));
  }

(* Normalize module if the class is public *)
let remove_modules_if_public sc =
  if sc.sc_internal then
    sc
  else
    { sc with sc_module = None }

let remove_members_except_to_string sc =
  {
    sc with
    sc_constructor = None;
    sc_consts = [];
    sc_typeconsts = [];
    sc_props = [];
    sc_sprops = [];
    sc_static_methods = [];
    sc_methods =
      List.filter sc.sc_methods ~f:(fun m ->
          String.equal (snd m.sm_name) SN.Members.__toString);
  }

(* To normalize classes for comparison, we:

   - Remove the ConsistentConstruct attribute
   - Remove module if class itself is public
   - Remove all members (except for the toString method, which results in the
     implicit inclusion of the Stringish interface),
   - Replace all positions with Pos.none

   We consider any difference between normalized classes to be a "Major change".
   This means only that we have not implemented some means of computing a fanout
   for that change which is smaller than what would be added by
   Shallow_class_fanout.add_maximum_fanout (e.g., since we DO have fine-grained
   dependency tracking for uses of members, and can handle them in a more
   intelligent way, we remove them during major-change-detection).

   There are certainly opportunities for us to be cleverer about some kinds of
   changes, but any kind of cleverness in reducing fanout must be implemented
   with great care. *)
let normalize sc ~same_package =
  let id sc = sc in
  sc
  |> remove_consistent_construct_attribute
  |> remove_members_except_to_string
  |> Decl_pos_utils.NormalizeSig.shallow_class
  |>
  if same_package then
    remove_modules_if_public
  else
    id

let type_name ty =
  let (_, (_, name), tparams) = Decl_utils.unwrap_class_type ty in
  (name, tparams)

let diff_value_lists values1 values2 ~equal ~get_name_value ~diff =
  if List.equal equal values1 values2 then
    None
  else
    Some
      (let values1 = List.map ~f:get_name_value values1 in
       let values2 = List.map ~f:get_name_value values2 in
       {
         NamedItemsListChange.order_change =
           not
           @@ List.equal
                String.equal
                (List.map ~f:fst values1)
                (List.map ~f:fst values2);
         per_name_changes =
           SMap.merge
             (SMap.of_list values1)
             (SMap.of_list values2)
             ~f:(fun _ value1 value2 ->
               match (value1, value2) with
               | (None, None) -> None
               | (None, Some _) -> Some ValueChange.Added
               | (Some _, None) -> Some ValueChange.Removed
               | (Some value1, Some value2) ->
                 let open Option.Monad_infix in
                 diff value1 value2 >>| fun change ->
                 ValueChange.Modified change);
       })

let diff_of_equal equal x y =
  if equal x y then
    None
  else
    Some ()

let diff_type_lists =
  diff_value_lists
    ~equal:Typing_defs.ty_equal
    ~get_name_value:type_name
    ~diff:
      (diff_value_lists
         ~equal:Typing_defs.ty_equal
         ~get_name_value:type_name
         ~diff:(diff_of_equal Typing_defs.tyl_equal))

let diff_parents (c1 : Parents.t) (c2 : Parents.t) : parent_changes option =
  if Parents.equal c1 c2 then
    None
  else
    Some
      {
        extends_changes = diff_type_lists c1.Parents.extends c2.Parents.extends;
        implements_changes =
          diff_type_lists c1.Parents.implements c2.Parents.implements;
        req_extends_changes =
          diff_type_lists c1.Parents.req_extends c2.Parents.req_extends;
        req_implements_changes =
          diff_type_lists c1.Parents.req_implements c2.Parents.req_implements;
        req_class_changes =
          diff_type_lists c1.Parents.req_class c2.Parents.req_class;
        uses_changes = diff_type_lists c1.Parents.uses c2.Parents.uses;
        xhp_attr_changes =
          diff_type_lists c1.Parents.xhp_attr_uses c2.Parents.xhp_attr_uses;
      }

let diff_kinds kind1 kind2 =
  if Ast_defs.equal_classish_kind kind1 kind2 then
    None
  else
    Some { KindChange.new_kind = kind2 }

let diff_bools b1 b2 =
  match (b1, b2) with
  | (true, true)
  | (false, false) ->
    None
  | (false, true) -> Some BoolChange.Became
  | (true, false) -> Some BoolChange.No_more

let diff_options option1 option2 ~diff =
  match (option1, option2) with
  | (None, None) -> None
  | (None, Some _) -> Some ValueChange.Added
  | (Some _, None) -> Some ValueChange.Removed
  | (Some value1, Some value2) ->
    (match diff value1 value2 with
    | None -> None
    | Some diff -> Some (ValueChange.Modified diff))

let diff_modules = diff_options ~diff:(diff_of_equal Ast_defs.equal_id)

let diff
    (type value)
    ~(equal : value -> value -> bool)
    (old_value : value)
    (new_value : value) : value ValueDiff.t option =
  if equal old_value new_value then
    None
  else
    Some { ValueDiff.old_value; new_value }

let diff_types = diff ~equal:Typing_defs.ty_equal

let diff_enum_types
    (enum_type1 : Typing_defs.enum_type) (enum_type2 : Typing_defs.enum_type) :
    enum_type_change option =
  if Typing_defs.equal_enum_type enum_type1 enum_type2 then
    None
  else
    Option.some
    @@
    let {
      Typing_defs.te_base = base1;
      te_constraint = constraint1;
      te_includes = includes1;
    } =
      enum_type1
    in
    let {
      Typing_defs.te_base = base2;
      te_constraint = constraint2;
      te_includes = includes2;
    } =
      enum_type2
    in

    {
      base_change = diff_types base1 base2;
      constraint_change = diff_options ~diff:diff_types constraint1 constraint2;
      includes_change = diff_type_lists includes1 includes2;
    }

let diff_enum_type_options = diff_options ~diff:diff_enum_types

let user_attribute_name_value { Typing_defs.ua_name = (_, name); ua_params } =
  (name, ua_params)

let equal_user_attr_params = [%derive.eq: Typing_defs.user_attribute_param list]

let diff_class_shells (c1 : shallow_class) (c2 : shallow_class) :
    class_shell_change =
  {
    classish_kind = c1.sc_kind;
    parent_changes =
      diff_parents (Parents.of_shallow_class c1) (Parents.of_shallow_class c2);
    type_parameters_change =
      diff_value_lists
        c2.sc_tparams
        c1.sc_tparams
        ~equal:Typing_defs.equal_decl_tparam
        ~get_name_value:(fun tparam -> (snd tparam.Typing_defs.tp_name, tparam))
        ~diff:(diff_of_equal Typing_defs.equal_decl_tparam);
    kind_change = diff_kinds c1.sc_kind c2.sc_kind;
    final_change = diff_bools c1.sc_final c2.sc_final;
    abstract_change = diff_bools c1.sc_abstract c2.sc_abstract;
    is_xhp_change = diff_bools c1.sc_is_xhp c2.sc_is_xhp;
    internal_change = diff_bools c1.sc_internal c2.sc_internal;
    has_xhp_keyword_change =
      diff_bools c1.sc_has_xhp_keyword c2.sc_has_xhp_keyword;
    support_dynamic_type_change =
      diff_bools c1.sc_support_dynamic_type c2.sc_support_dynamic_type;
    module_change = diff_modules c1.sc_module c2.sc_module;
    xhp_enum_values_change =
      not @@ equal_xhp_enum_values c1.sc_xhp_enum_values c2.sc_xhp_enum_values;
    user_attributes_changes =
      diff_value_lists
        c1.sc_user_attributes
        c2.sc_user_attributes
        ~equal:Typing_defs.equal_user_attribute
        ~get_name_value:user_attribute_name_value
        ~diff:(diff_of_equal equal_user_attr_params);
    enum_type_change = diff_enum_type_options c1.sc_enum_type c2.sc_enum_type;
  }

let same_package
    (info : PackageInfo.t) (c1 : shallow_class) (c2 : shallow_class) : bool =
  let get_package_for_module info sc_module =
    match sc_module with
    | Some (_, name) -> PackageInfo.get_package_for_module info name
    | None -> None
  in
  let p1 = get_package_for_module info c1.sc_module in
  let p2 = get_package_for_module info c2.sc_module in
  Option.equal Package.equal p1 p2

let diff_class (info : PackageInfo.t) (c1 : shallow_class) (c2 : shallow_class)
    : ClassDiff.t =
  let same_package = same_package info c1 c2 in
  let class_shell1 = normalize c1 ~same_package
  and class_shell2 = normalize c2 ~same_package in
  let member_diff = diff_class_members c1 c2 in
  if not (equal_shallow_class class_shell1 class_shell2) then
    Major_change
      (MajorChange.Modified
         (diff_class_shells class_shell1 class_shell2, member_diff))
  else
    let mro_inputs_equal = mro_inputs_equal c1 c2 in
    if mro_inputs_equal && ClassDiff.is_empty_member_diff member_diff then
      Unchanged
    else
      Minor_change member_diff
