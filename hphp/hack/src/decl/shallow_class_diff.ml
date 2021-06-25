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

let diff_members
    (members : ('member option * 'member option) SMap.t)
    (diff_member : 'member -> 'member -> member_change option)
    (is_private : 'member -> bool)
    (class_kind : Ast_defs.class_kind) : member_change SMap.t =
  SMap.fold members ~init:SMap.empty ~f:(fun name old_and_new diff ->
      match old_and_new with
      | (None, None) -> failwith "merge_member_lists added (None, None)"
      | (Some member, None)
      | (None, Some member)
        when (not Ast_defs.(equal_class_kind class_kind Ctrait))
             && is_private member ->
        SMap.add diff ~key:name ~data:Private_change
      | (Some _, None) -> SMap.add diff ~key:name ~data:Removed
      | (None, Some _) -> SMap.add diff ~key:name ~data:Added
      | (Some old_member, Some new_member) ->
        diff_member old_member new_member
        |> Option.value_map ~default:diff ~f:(fun ch ->
               SMap.add diff ~key:name ~data:ch))

let diff_const (c1 : shallow_class_const) c2 : member_change option =
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

let diff_typeconst tc1 tc2 : member_change option =
  let tc1 = Decl_pos_utils.NormalizeSig.shallow_typeconst tc1 in
  let tc2 = Decl_pos_utils.NormalizeSig.shallow_typeconst tc2 in
  if equal_shallow_typeconst tc1 tc2 then
    None
  else
    let open Typing_defs in
    match (tc1.stc_kind, tc2.stc_kind) with
    | (TCAbstract _, TCAbstract _)
    | (TCPartiallyAbstract _, TCPartiallyAbstract _)
    | (TCConcrete _, TCConcrete _) ->
      Some Modified
    | (_, (TCAbstract _ | TCPartiallyAbstract _ | TCConcrete _)) ->
      Some Changed_inheritance

let diff_prop p1 p2 : member_change option =
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

let diff_method m1 m2 : member_change option =
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

let is_prop_private p : bool =
  Aast_defs.equal_visibility p.sp_visibility Aast_defs.Private

let is_method_private m : bool =
  Aast_defs.equal_visibility m.sm_visibility Aast_defs.Private

let diff_constructor old_cls new_cls old_cstr new_cstr : member_change option =
  match (old_cstr, new_cstr) with
  | (None, None) -> None
  | (Some _, None) -> Some Removed
  | (None, Some _) -> Some Added
  | (Some old_method, Some new_method) ->
    let consistent1 = Decl_utils.consistent_construct_kind old_cls in
    let consistent2 = Decl_utils.consistent_construct_kind new_cls in
    if not (Typing_defs.equal_consistent_kind consistent1 consistent2) then
      Some Changed_inheritance
    else
      diff_method old_method new_method

let diff_class_members (c1 : shallow_class) (c2 : shallow_class) :
    ClassDiff.member_diff =
  let diff = ClassDiff.empty_member_diff in
  let kind = c2.sc_kind in
  let diff =
    let get_name x = snd x.scc_name in
    let consts = merge_member_lists get_name c1.sc_consts c2.sc_consts in
    let is_private _ = false in
    let consts = diff_members consts diff_const is_private kind in
    { diff with consts }
  in
  let diff =
    let get_name x = snd x.stc_name in
    let typeconsts =
      merge_member_lists get_name c1.sc_typeconsts c2.sc_typeconsts
    in
    let is_private _ = false in
    let typeconsts = diff_members typeconsts diff_typeconst is_private kind in
    { diff with typeconsts }
  in
  let diff =
    let get_name x = snd x.sp_name in
    let props = merge_member_lists get_name c1.sc_props c2.sc_props in
    let props = diff_members props diff_prop is_prop_private kind in
    { diff with props }
  in
  let diff =
    let get_name x = snd x.sp_name in
    let sprops = merge_member_lists get_name c1.sc_sprops c2.sc_sprops in
    let sprops = diff_members sprops diff_prop is_prop_private kind in
    { diff with sprops }
  in
  let diff =
    let get_name x = snd x.sm_name in
    let methods = merge_member_lists get_name c1.sc_methods c2.sc_methods in
    let methods = diff_members methods diff_method is_method_private kind in
    { diff with methods }
  in
  let diff =
    let get_name x = snd x.sm_name in
    let smethods =
      merge_member_lists get_name c1.sc_static_methods c2.sc_static_methods
    in
    let smethods = diff_members smethods diff_method is_method_private kind in
    { diff with smethods }
  in
  let diff =
    let constructor =
      diff_constructor c1 c2 c1.sc_constructor c2.sc_constructor
    in
    { diff with constructor }
  in
  diff

(** Return true if the two classes would (shallowly) produce the same member
    resolution order (the output of the Decl_linearize module).

    NB: It is critical for the correctness of incremental typechecking that
    every bit of information used by Decl_linearize is checked here! *)
let mro_inputs_equal (c1 : shallow_class) (c2 : shallow_class) : bool =
  let is_to_string m = String.equal (snd m.sm_name) SN.Members.__toString in
  Typing_defs.equal_pos_id c1.sc_name c2.sc_name
  && Ast_defs.equal_class_kind c1.sc_kind c2.sc_kind
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
   - Remove all members (except for the toString method, which results in the
     implicit inclusion of the Stringish interface),
   - Replace all positions with Pos.none

   We consider any difference between normalized classes to be a "Major change".
   This means only that we have not implemented some means of computing a fanout
   for that change which is smaller than what would be added by
   AffectedDeps.add_maximum_fanout (e.g., since we DO have fine-grained
   dependency tracking for uses of members, and can handle them in a more
   intelligent way, we remove them during major-change-detection).

   There are certainly opportunities for us to be cleverer about some kinds of
   changes, but any kind of cleverness in reducing fanout must be implemented
   with great care. *)
let normalize sc =
  sc
  |> remove_consistent_construct_attribute
  |> remove_members_except_to_string
  |> Decl_pos_utils.NormalizeSig.shallow_class

let diff_class (c1 : shallow_class) (c2 : shallow_class) : ClassDiff.t =
  if not (equal_shallow_class (normalize c1) (normalize c2)) then
    Major_change
  else
    let mro_inputs_equal = mro_inputs_equal c1 c2 in
    (* If the old and new classes were identical with positions normalized, but
       mro_inputs_equal returns false, then we need to invalidate the MRO of
       this class and its descendants because its positions have changed. *)
    let mro_positions_changed = not mro_inputs_equal in
    let member_diff = diff_class_members c1 c2 in
    if mro_inputs_equal && ClassDiff.is_empty_member_diff member_diff then
      Unchanged
    else
      Minor_change { mro_positions_changed; member_diff }
