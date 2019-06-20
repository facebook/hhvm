(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Helpers for the Decl_inheritance module.

    Converts Decl representations of class elements (Shallow_decl_defs) to the
    representations used by the typechecker (Typing_defs). *)

open Core_kernel
open Decl_defs
open Nast
open Shallow_decl_defs
open Typing_defs

module Reason = Typing_reason

(** [tagged_elt] is a representation internal to Decl_inheritance which is used
    for both methods and properties (members represented using
    {!Typing_defs.class_elt}). Tagging these members with [inherit_when_private]
    allows us to assign private trait members to the class which used the trait
    and to filter out other private members. *)
type tagged_elt = {
  id: string;
  inherit_when_private: bool;
  elt: class_elt;
}

let method_redeclaration_to_shallow_method smr =
  {
    sm_abstract = smr.smr_abstract;
    sm_final = smr.smr_final;
    sm_memoizelsb = false;
    sm_name = smr.smr_name;
    sm_override = false;
    sm_reactivity = None;
    sm_type = smr.smr_type;
    sm_visibility = smr.smr_visibility;
  }

let redecl_list_to_method_seq redecls =
  redecls
  |> Sequence.of_list
  |> Sequence.map ~f:method_redeclaration_to_shallow_method

let base_visibility origin_class_name = function
  | Public -> Vpublic
  | Private -> Vprivate origin_class_name
  | Protected -> Vprotected origin_class_name

let ft_to_ty ft = Reason.Rwitness ft.ft_pos, Tfun ft

let shallow_method_to_class_elt child_class mro subst meth : class_elt =
  let visibility = base_visibility mro.mro_name meth.sm_visibility in
  let ty = lazy begin
    let ty = ft_to_ty meth.sm_type in
    if child_class = mro.mro_name then ty else
    Decl_instantiate.instantiate subst ty
  end in
  {
    ce_abstract = meth.sm_abstract;
    ce_final = meth.sm_final;
    ce_xhp_attr = None;
    ce_const = false;
    ce_lateinit = false;
    ce_override = meth.sm_override;
    ce_lsb = false;
    ce_memoizelsb = meth.sm_memoizelsb;
    ce_synthesized = mro.mro_synthesized;
    ce_visibility = visibility;
    ce_origin = mro.mro_name;
    ce_type = ty;
  }

let shallow_method_to_telt child_class mro subst meth : tagged_elt =
  {
    id = snd meth.sm_name;
    inherit_when_private = mro.mro_copy_private_members;
    elt = shallow_method_to_class_elt child_class mro subst meth;
  }

let shallow_prop_to_telt child_class mro subst prop : tagged_elt =
  let visibility = base_visibility mro.mro_name prop.sp_visibility in
  let ty = lazy begin
    let ty = match prop.sp_type with
      | None -> Reason.Rwitness (fst prop.sp_name), Tany
      | Some ty -> ty
    in
    if child_class = mro.mro_name then ty else
    Decl_instantiate.instantiate subst ty
  end in
  {
    id = snd prop.sp_name;
    inherit_when_private = mro.mro_copy_private_members;
    elt = {
      ce_abstract = false;
      ce_final = true;
      ce_xhp_attr = prop.sp_xhp_attr;
      ce_const = prop.sp_const;
      ce_lateinit = prop.sp_lateinit;
      ce_override = false;
      ce_lsb = prop.sp_lsb;
      ce_memoizelsb = false;
      ce_synthesized = false;
      ce_visibility = visibility;
      ce_origin = mro.mro_name;
      ce_type = ty;
    }
  }

let shallow_const_to_class_const child_class mro subst const =
  let ty =
    let ty = const.scc_type in
    if child_class = mro.mro_name then ty else
    Decl_instantiate.instantiate subst ty
  in
  snd const.scc_name, {
    cc_synthesized = mro.mro_synthesized;
    cc_abstract = const.scc_abstract;
    cc_pos = fst const.scc_name;
    cc_type = ty;
    cc_expr = const.scc_expr;
    cc_origin = mro.mro_name;
  }

(** Each class [C] implicitly defines a class constant named [class], which has
    type [classname<C>]. *)
let classname_const class_id =
  let pos, name = class_id in
  let reason = Reason.Rclass_class (pos, name) in
  let classname_ty =
    reason, Tapply ((pos, SN.Classes.cClassname), [reason, Tthis]) in
  SN.Members.mClass, {
    cc_abstract = false;
    cc_pos = pos;
    cc_synthesized = true;
    cc_type = classname_ty;
    cc_expr = None;
    cc_origin = name;
  }

(** Each concrete type constant [const type <sometype> T] implicitly defines a
    class constant of the same name with type [TypeStructure<sometype>].
    Given a typeconst definition, this function returns the corresponding
    implicit class constant representing its reified type structure. *)
let typeconst_structure mro class_name stc =
  let pos = fst stc.stc_name in
  let r = Reason.Rwitness pos in
  let tsid = pos, SN.FB.cTypeStructure in
  let ts_ty = r, Tapply (tsid, [r, Taccess ((r, Tthis), [stc.stc_name])]) in
  let abstract =
    match stc.stc_abstract with
    | TCAbstract (Some _) when not mro.mro_passthrough_abstract_typeconst ->
      false
    | TCAbstract _ ->
      true
    | TCPartiallyAbstract
    | TCConcrete ->
      false in
  snd stc.stc_name, {
    cc_abstract    = abstract;
    cc_pos         = pos;
    cc_synthesized = true;
    cc_type        = ts_ty;
    cc_expr        = None;
    cc_origin      = class_name;
  }

let shallow_typeconst_to_typeconst_type child_class mro subst stc =
  let constraint_ =
    if child_class = mro.mro_name then stc.stc_constraint else
    Option.map stc.stc_constraint (Decl_instantiate.instantiate subst)
  in
  let ty =
    if child_class = mro.mro_name then stc.stc_type else
    Option.map stc.stc_type (Decl_instantiate.instantiate subst) in
  let abstract = match stc.stc_abstract with
  | TCAbstract default_opt when child_class <> mro.mro_name ->
    TCAbstract (Option.map default_opt (Decl_instantiate.instantiate subst))
  | _ -> stc.stc_abstract in
  let typeconst = match abstract with
  | TCAbstract (Some default) when not mro.mro_passthrough_abstract_typeconst ->
    {
      ttc_abstract = TCConcrete;
      ttc_name = stc.stc_name;
      ttc_constraint = None;
      ttc_type = Some default;
      ttc_origin = mro.mro_name;
      ttc_enforceable = stc.stc_enforceable;
    }
  | _ ->
    {
      ttc_abstract = abstract;
      ttc_name = stc.stc_name;
      ttc_constraint = constraint_;
      ttc_type = ty;
      ttc_origin = mro.mro_name;
      ttc_enforceable = stc.stc_enforceable;
    } in
  snd stc.stc_name, typeconst
