(*
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

open Hh_prelude
open Decl_defs
open Aast
open Shallow_decl_defs
open Typing_defs
module Reason = Typing_reason
module SN = Naming_special_names

(** [tagged_elt] is a representation internal to Decl_inheritance which is used
    for both methods and properties (members represented using
    {!Typing_defs.class_elt}). Tagging these members with [inherit_when_private]
    allows us to assign private trait members to the class which used the trait
    and to filter out other private members. *)
type tagged_elt = {
  id: string;
  inherit_when_private: bool;
  elt: Typing_defs.class_elt;
}

let base_visibility origin_class_name = function
  | Public -> Vpublic
  | Private -> Vprivate origin_class_name
  | Protected -> Vprotected origin_class_name

let shallow_method_to_class_elt child_class mro subst meth : class_elt =
  let {
    sm_name = (pos, _);
    sm_type = ty;
    sm_visibility;
    sm_deprecated;
    sm_flags = _;
  } =
    meth
  in
  let visibility = base_visibility mro.mro_name sm_visibility in
  let ty =
    lazy
      begin
        if String.equal child_class mro.mro_name then
          ty
        else
          Decl_instantiate.instantiate subst ty
      end
  in
  {
    ce_visibility = visibility;
    ce_origin = mro.mro_name;
    ce_type = ty;
    ce_deprecated = sm_deprecated;
    ce_pos = lazy pos;
    ce_flags =
      make_ce_flags
        ~xhp_attr:None
        ~synthesized:(is_set mro_via_req_extends mro.mro_flags)
        ~abstract:(sm_abstract meth)
        ~final:(sm_final meth)
        ~const:false
        ~lateinit:false
        ~lsb:false
        ~override:(sm_override meth)
        ~dynamicallycallable:(sm_dynamicallycallable meth)
        ~readonly_prop:false
        ~sound_dynamic_callable:(sm_sound_dynamic_callable meth)
      (* The readonliness of the method is on the type *);
  }

let shallow_method_to_telt child_class mro subst meth : tagged_elt =
  {
    id = snd meth.sm_name;
    inherit_when_private = is_set mro_copy_private_members mro.mro_flags;
    elt = shallow_method_to_class_elt child_class mro subst meth;
  }

let shallow_prop_to_telt child_class mro subst prop : tagged_elt =
  let { sp_xhp_attr = xhp_attr; sp_name; sp_type; sp_visibility; sp_flags = _ }
      =
    prop
  in
  let visibility = base_visibility mro.mro_name sp_visibility in
  let ty =
    lazy
      begin
        let ty =
          match sp_type with
          | None ->
            mk
              (Reason.Rwitness_from_decl (fst sp_name), Typing_defs.make_tany ())
          | Some ty -> ty
        in
        if String.equal child_class mro.mro_name then
          ty
        else
          Decl_instantiate.instantiate subst ty
      end
  in
  {
    id = snd sp_name;
    inherit_when_private = is_set mro_copy_private_members mro.mro_flags;
    elt =
      {
        ce_visibility = visibility;
        ce_origin = mro.mro_name;
        ce_type = ty;
        ce_deprecated = None;
        ce_pos = lazy (fst sp_name);
        ce_flags =
          make_ce_flags
            ~xhp_attr
            ~lsb:(sp_lsb prop)
            ~const:(sp_const prop)
            ~lateinit:(sp_lateinit prop)
            ~abstract:(sp_abstract prop)
            ~final:true
            ~override:false
            ~synthesized:false
            ~dynamicallycallable:false
            ~readonly_prop:(sp_readonly prop)
            ~sound_dynamic_callable:false;
      };
  }

let shallow_const_to_class_const child_class mro subst const =
  let { scc_abstract = cc_abstract; scc_name; scc_type; scc_refs } = const in
  let ty =
    let ty = scc_type in
    if String.equal child_class mro.mro_name then
      ty
    else
      Decl_instantiate.instantiate subst ty
  in
  ( snd scc_name,
    {
      cc_synthesized = is_set mro_via_req_extends mro.mro_flags;
      cc_abstract;
      cc_pos = fst scc_name;
      cc_type = ty;
      cc_origin = mro.mro_name;
      cc_refs = scc_refs;
    } )

(** Each class [C] implicitly defines a class constant named [class], which has
    type [classname<C>]. *)
let classname_const : Typing_defs.pos_id -> string * Typing_defs.class_const =
 fun class_id ->
  let (pos, name) = class_id in
  let reason = Reason.Rclass_class (pos, name) in
  let classname_ty =
    mk (reason, Tapply ((pos, SN.Classes.cClassname), [mk (reason, Tthis)]))
  in
  ( SN.Members.mClass,
    {
      cc_abstract = false;
      cc_pos = pos;
      cc_synthesized = true;
      cc_type = classname_ty;
      cc_origin = name;
      cc_refs = [];
    } )

(** Each concrete type constant [const type <sometype> T] implicitly defines a
    class constant of the same name with type [TypeStructure<sometype>].
    Given a typeconst definition, this function returns the corresponding
    implicit class constant representing its reified type structure. *)
let typeconst_structure mro class_name stc =
  let pos = fst stc.stc_name in
  let r = Reason.Rwitness_from_decl pos in
  let tsid = (pos, SN.FB.cTypeStructure) in
  let ts_ty =
    mk (r, Tapply (tsid, [mk (r, Taccess (mk (r, Tthis), stc.stc_name))]))
  in
  let abstract =
    match stc.stc_abstract with
    | TCAbstract (Some _)
      when not (is_set mro_passthrough_abstract_typeconst mro.mro_flags) ->
      false
    | TCAbstract _ -> true
    | TCPartiallyAbstract
    | TCConcrete ->
      false
  in
  ( snd stc.stc_name,
    {
      cc_abstract = abstract;
      cc_pos = pos;
      cc_synthesized = true;
      cc_type = ts_ty;
      cc_origin = class_name;
      cc_refs = [];
    } )

let shallow_typeconst_to_typeconst_type child_class mro subst stc =
  let {
    stc_abstract;
    stc_as_constraint;
    stc_super_constraint;
    stc_name = ttc_name;
    stc_type;
    stc_enforceable = ttc_enforceable;
    stc_reifiable = ttc_reifiable;
  } =
    stc
  in
  let child_and_mro_same = String.equal child_class mro.mro_name in
  let (as_constraint, super_constraint) =
    if child_and_mro_same then
      (stc_as_constraint, stc_super_constraint)
    else
      ( Option.map stc_as_constraint (Decl_instantiate.instantiate subst),
        Option.map stc_super_constraint (Decl_instantiate.instantiate subst) )
  in
  let ty =
    if child_and_mro_same then
      stc_type
    else
      Option.map stc_type (Decl_instantiate.instantiate subst)
  in
  let abstract =
    match stc_abstract with
    | TCAbstract default_opt when not child_and_mro_same ->
      TCAbstract (Option.map default_opt (Decl_instantiate.instantiate subst))
    | _ -> stc_abstract
  in
  let ttc_origin = mro.mro_name in
  let typeconst =
    match abstract with
    | TCAbstract (Some default)
      when not (is_set mro_passthrough_abstract_typeconst mro.mro_flags) ->
      {
        ttc_abstract = TCConcrete;
        ttc_synthesized = is_set mro_via_req_extends mro.mro_flags;
        ttc_name;
        ttc_as_constraint = None;
        ttc_super_constraint = None;
        ttc_type = Some default;
        ttc_origin;
        ttc_enforceable;
        ttc_reifiable;
        ttc_concretized = true;
      }
    | _ ->
      {
        ttc_abstract = abstract;
        ttc_synthesized = is_set mro_via_req_extends mro.mro_flags;
        ttc_name;
        ttc_as_constraint = as_constraint;
        ttc_super_constraint = super_constraint;
        ttc_type = ty;
        ttc_origin;
        ttc_enforceable;
        ttc_reifiable;
        ttc_concretized = false;
      }
  in
  (snd ttc_name, typeconst)
