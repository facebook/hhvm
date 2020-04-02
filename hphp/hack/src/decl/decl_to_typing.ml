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
  let {
    smr_abstract = sm_abstract;
    smr_final = sm_final;
    smr_static = _;
    smr_name = sm_name;
    smr_type = sm_type;
    smr_visibility = sm_visibility;
    smr_trait = _;
    smr_method = _;
    smr_fixme_codes = sm_fixme_codes;
  } =
    smr
  in
  {
    sm_abstract;
    sm_final;
    sm_memoizelsb = false;
    sm_name;
    sm_override = false;
    sm_dynamicallycallable = false;
    sm_reactivity = None;
    sm_type;
    sm_visibility;
    sm_fixme_codes;
    sm_deprecated = None;
  }

let redecl_list_to_method_seq redecls =
  redecls
  |> Sequence.of_list
  |> Sequence.map ~f:method_redeclaration_to_shallow_method

let base_visibility origin_class_name = function
  | Public -> Vpublic
  | Private -> Vprivate origin_class_name
  | Protected -> Vprotected origin_class_name

let shallow_method_to_class_elt child_class mro subst meth : class_elt =
  let {
    sm_abstract = abstract;
    sm_final = final;
    sm_memoizelsb = memoizelsb;
    sm_name = (pos, _);
    sm_dynamicallycallable = dynamicallycallable;
    sm_override = override;
    sm_reactivity = _;
    sm_type = ty;
    sm_visibility;
    sm_fixme_codes = _;
    sm_deprecated;
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
        ~synthesized:mro.mro_via_req_extends
        ~abstract
        ~final
        ~const:false
        ~lateinit:false
        ~lsb:false
        ~override
        ~memoizelsb
        ~dynamicallycallable;
  }

let shallow_method_to_telt child_class mro subst meth : tagged_elt =
  {
    id = snd meth.sm_name;
    inherit_when_private = mro.mro_copy_private_members;
    elt = shallow_method_to_class_elt child_class mro subst meth;
  }

let shallow_prop_to_telt child_class mro subst prop : tagged_elt =
  let {
    sp_const = const;
    sp_xhp_attr = xhp_attr;
    sp_lateinit = lateinit;
    sp_lsb = lsb;
    sp_name;
    sp_needs_init = _;
    sp_type;
    sp_abstract;
    sp_visibility;
    sp_fixme_codes = _;
  } =
    prop
  in
  let visibility = base_visibility mro.mro_name sp_visibility in
  let ty =
    lazy
      begin
        let ty =
          match sp_type with
          | None -> mk (Reason.Rwitness (fst sp_name), Typing_defs.make_tany ())
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
    inherit_when_private = mro.mro_copy_private_members;
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
            ~lsb
            ~const
            ~lateinit
            ~abstract:sp_abstract
            ~final:true
            ~override:false
            ~memoizelsb:false
            ~synthesized:false
            ~dynamicallycallable:false;
      };
  }

let shallow_const_to_class_const child_class mro subst const =
  let { scc_abstract = cc_abstract; scc_expr = cc_expr; scc_name; scc_type } =
    const
  in
  let ty =
    let ty = scc_type in
    if String.equal child_class mro.mro_name then
      ty
    else
      Decl_instantiate.instantiate subst ty
  in
  ( snd scc_name,
    {
      cc_synthesized = mro.mro_via_req_extends;
      cc_abstract;
      cc_pos = fst scc_name;
      cc_type = ty;
      cc_expr;
      cc_origin = mro.mro_name;
    } )

(** Each class [C] implicitly defines a class constant named [class], which has
    type [classname<C>]. *)
let classname_const class_id =
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
      cc_expr = None;
      cc_origin = name;
    } )

(** Each concrete type constant [const type <sometype> T] implicitly defines a
    class constant of the same name with type [TypeStructure<sometype>].
    Given a typeconst definition, this function returns the corresponding
    implicit class constant representing its reified type structure. *)
let typeconst_structure mro class_name stc =
  let pos = fst stc.stc_name in
  let r = Reason.Rwitness pos in
  let tsid = (pos, SN.FB.cTypeStructure) in
  let ts_ty =
    mk (r, Tapply (tsid, [mk (r, Taccess (mk (r, Tthis), [stc.stc_name]))]))
  in
  let abstract =
    match stc.stc_abstract with
    | TCAbstract (Some _) when not mro.mro_passthrough_abstract_typeconst ->
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
      cc_expr = None;
      cc_origin = class_name;
    } )

let shallow_typeconst_to_typeconst_type child_class mro subst stc =
  let {
    stc_abstract;
    stc_constraint;
    stc_name = ttc_name;
    stc_type;
    stc_enforceable = ttc_enforceable;
    stc_reifiable = ttc_reifiable;
  } =
    stc
  in
  let constraint_ =
    if String.equal child_class mro.mro_name then
      stc_constraint
    else
      Option.map stc_constraint (Decl_instantiate.instantiate subst)
  in
  let ty =
    if String.equal child_class mro.mro_name then
      stc_type
    else
      Option.map stc_type (Decl_instantiate.instantiate subst)
  in
  let abstract =
    match stc_abstract with
    | TCAbstract default_opt when String.( <> ) child_class mro.mro_name ->
      TCAbstract (Option.map default_opt (Decl_instantiate.instantiate subst))
    | _ -> stc_abstract
  in
  let typeconst =
    match abstract with
    | TCAbstract (Some default) when not mro.mro_passthrough_abstract_typeconst
      ->
      {
        ttc_abstract = TCConcrete;
        ttc_name;
        ttc_constraint = None;
        ttc_type = Some default;
        ttc_origin = mro.mro_name;
        ttc_enforceable;
        ttc_reifiable;
      }
    | _ ->
      {
        ttc_abstract = abstract;
        ttc_name;
        ttc_constraint = constraint_;
        ttc_type = ty;
        ttc_origin = mro.mro_name;
        ttc_enforceable;
        ttc_reifiable;
      }
  in
  (snd ttc_name, typeconst)

let shallow_pu_enum_to_pu_enum_type spu =
  let to_member { spum_atom; spum_types; spum_exprs } =
    {
      tpum_atom = spum_atom;
      tpum_types =
        List.fold
          ~init:SMap.empty
          ~f:
            begin
              fun acc (k, t) ->
              SMap.add (snd k) (k, t) acc
            end
          spum_types;
      tpum_exprs =
        List.fold
          ~init:SMap.empty
          ~f:
            begin
              fun acc k ->
              SMap.add (snd k) k acc
            end
          spum_exprs;
    }
  in
  let { spu_name; spu_is_final; spu_case_types; spu_case_values; spu_members } =
    spu
  in
  {
    tpu_name = spu_name;
    tpu_is_final = spu_is_final;
    tpu_case_types =
      List.fold
        ~init:SMap.empty
        ~f:
          begin
            fun acc (sid, k) ->
            SMap.add (snd sid) (sid, k) acc
          end
        spu_case_types;
    tpu_case_values =
      List.fold
        ~init:SMap.empty
        ~f:
          begin
            fun acc (k, t) ->
            SMap.add (snd k) (k, t) acc
          end
        spu_case_values;
    tpu_members =
      List.fold
        ~init:SMap.empty
        ~f:
          begin
            fun acc pum ->
            SMap.add (snd pum.spum_atom) (to_member pum) acc
          end
        spu_members;
  }
