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
        ~support_dynamic_type:(sm_support_dynamic_type meth)
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
            ~support_dynamic_type:false;
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
  (* TODO(T88552052) this belongs under the umbrella of distinctions between
   * abstract type constants with and without defaults *)
  let abstract =
    match stc.stc_kind with
    | TCAbstract { atc_default = Some _; _ }
      when not (is_set mro_passthrough_abstract_typeconst mro.mro_flags) ->
      false
    | TCAbstract _ -> true
    | TCPartiallyAbstract _
    | TCConcrete _ ->
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
    stc_kind = ttc_kind;
    stc_name = ttc_name;
    stc_enforceable = ttc_enforceable;
    stc_reifiable = ttc_reifiable;
    stc_is_ctx = ttc_is_ctx;
  } =
    stc
  in
  let child_and_mro_same = String.equal child_class mro.mro_name in
  let ttc_kind =
    if child_and_mro_same then
      ttc_kind
    else
      Decl_instantiate.instantiate_typeconst subst ttc_kind
  in
  let ttc_origin = mro.mro_name in
  let typeconst =
    match ttc_kind with
    | TCAbstract { atc_default = Some default; _ }
      when not (is_set mro_passthrough_abstract_typeconst mro.mro_flags) ->
      (* concretization of abstract type constant with default *)
      {
        ttc_synthesized = is_set mro_via_req_extends mro.mro_flags;
        ttc_name;
        ttc_kind = TCConcrete { tc_type = default };
        ttc_origin;
        ttc_enforceable;
        ttc_reifiable;
        ttc_is_ctx;
        ttc_concretized = true;
      }
    | _ ->
      {
        ttc_synthesized = is_set mro_via_req_extends mro.mro_flags;
        ttc_name;
        ttc_kind;
        ttc_origin;
        ttc_enforceable;
        ttc_reifiable;
        ttc_is_ctx;
        ttc_concretized = false;
      }
  in
  (snd ttc_name, typeconst)
