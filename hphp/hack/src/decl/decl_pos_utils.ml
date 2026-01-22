(*
* Copyright (c) 2015, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the "hack" directory of this source tree.
*
*)

open Hh_prelude
open Decl_defs
open Shallow_decl_defs
open Typing_defs

(*****************************************************************************)
(* Functor traversing a type, but applies a user defined function for
 * positions. Positions in errors (_decl_errors) are not mapped - entire
 * field is erased instead. This is safe because there exists a completely
 * different system for tracking and updating positions in errors
 * (see ServerTypeCheck.get_files_with_stale_errors)
 *)
(*****************************************************************************)
module TraversePos (ImplementPos : sig
  val pos : Pos.t -> Pos.t

  val pos_or_decl : Pos_or_decl.t -> Pos_or_decl.t
end) =
struct
  open Typing_reason

  let pos = ImplementPos.pos

  let pos_or_decl = ImplementPos.pos_or_decl

  let positioned_id : Typing_defs.pos_id -> Typing_defs.pos_id =
   (fun (p, x) -> (pos_or_decl p, x))

  let rec ty t =
    let (r, x) = deref t in
    mk (map_pos pos pos_or_decl r, ty_ x)

  and ty_ : decl_phase ty_ -> decl_phase ty_ = function
    | (Tany _ | Tthis | Tmixed | Twildcard | Tnonnull | Tdynamic) as x -> x
    | Tvec_or_dict (ty1, ty2) -> Tvec_or_dict (ty ty1, ty ty2)
    | Tprim _ as x -> x
    | Tgeneric _ as x -> x
    | Ttuple { t_required; t_optional; t_extra } ->
      Ttuple
        {
          t_required = List.map t_required ~f:ty;
          t_optional = List.map t_optional ~f:ty;
          t_extra = tuple_extra t_extra;
        }
    | Tunion tyl -> Tunion (List.map tyl ~f:ty)
    | Tintersection tyl -> Tintersection (List.map tyl ~f:ty)
    | Toption x -> Toption (ty x)
    | Tlike x -> Tlike (ty x)
    | Tfun ft -> Tfun (fun_type ft)
    | Tapply (sid, xl) -> Tapply (positioned_id sid, List.map xl ~f:ty)
    | Taccess (root_ty, id) -> Taccess (ty root_ty, positioned_id id)
    | Trefinement (root_ty, rs) ->
      let rs = Class_refinement.map ty rs in
      Trefinement (ty root_ty, rs)
    | Tshape s -> Tshape (shape_type s)
    | Tclass_ptr x -> Tclass_ptr (ty x)

  and tuple_extra e =
    match e with
    | Tsplat t_splat -> Tsplat (ty t_splat)
    | Tvariadic t_variadic -> Tvariadic (ty t_variadic)

  and ty_opt x = Option.map x ~f:ty

  and shape_field_name = function
    | Typing_defs.TSFregex_group (p, s) ->
      Typing_defs.TSFregex_group (pos_or_decl p, s)
    | Typing_defs.TSFlit_str (p, s) -> Typing_defs.TSFlit_str (pos_or_decl p, s)
    | Typing_defs.TSFclass_const (id, s) ->
      Typing_defs.TSFclass_const (positioned_id id, positioned_id s)

  and constraint_ x = List.map ~f:(fun (ck, x) -> (ck, ty x)) x

  and capability = function
    | CapTy cap -> CapTy (ty cap)
    | CapDefaults p -> CapDefaults (pos_or_decl p)

  and fun_implicit_params implicit =
    { capability = capability implicit.capability }

  and fun_type ft =
    {
      ft with
      ft_tparams = List.map ~f:type_param ft.ft_tparams;
      ft_where_constraints =
        List.map ft.ft_where_constraints ~f:where_constraint;
      ft_params = List.map ft.ft_params ~f:fun_param;
      ft_implicit_params = fun_implicit_params ft.ft_implicit_params;
      ft_ret = ty ft.ft_ret;
    }

  and shape_type { s_origin = _; s_unknown_value = shape_kind; s_fields = fdm }
      =
    (* TODO(shapes) Should this be changing s_origin? *)
    {
      s_origin = Missing_origin;
      s_unknown_value = shape_kind;
      (* TODO(shapes) s_unknown_value is missing a call to ty *)
      s_fields = ShapeFieldMap.map_and_rekey fdm shape_field_name ty;
    }

  and fun_elt fe =
    {
      fe with
      fe_type = ty fe.fe_type;
      fe_pos = pos_or_decl fe.fe_pos;
      fe_package = Option.map fe.fe_package ~f:package_membership;
      fe_package_requirement = package_requirement fe.fe_package_requirement;
    }

  and where_constraint (ty1, c, ty2) = (ty ty1, c, ty ty2)

  and fun_param param =
    { param with fp_pos = pos_or_decl param.fp_pos; fp_type = ty param.fp_type }

  and class_const cc =
    {
      cc_synthesized = cc.cc_synthesized;
      cc_abstract = cc.cc_abstract;
      cc_pos = pos_or_decl cc.cc_pos;
      cc_type = ty cc.cc_type;
      cc_origin = cc.cc_origin;
      cc_refs = cc.cc_refs;
    }

  and typeconst = function
    | TCAbstract { atc_as_constraint; atc_super_constraint; atc_default } ->
      TCAbstract
        {
          atc_as_constraint = ty_opt atc_as_constraint;
          atc_super_constraint = ty_opt atc_super_constraint;
          atc_default = ty_opt atc_default;
        }
    | TCConcrete { tc_type } -> TCConcrete { tc_type = ty tc_type }

  and typeconst_type tc =
    {
      ttc_synthesized = tc.ttc_synthesized;
      ttc_name = positioned_id tc.ttc_name;
      ttc_kind = typeconst tc.ttc_kind;
      ttc_origin = tc.ttc_origin;
      ttc_enforceable = Tuple.T2.map_fst ~f:pos_or_decl tc.ttc_enforceable;
      ttc_reifiable = Option.map tc.ttc_reifiable ~f:pos_or_decl;
      ttc_concretized = tc.ttc_concretized;
      ttc_is_ctx = tc.ttc_is_ctx;
    }

  and user_attribute { ua_name; ua_params; _ } =
    { ua_name = positioned_id ua_name; ua_params; ua_raw_val = None }

  and type_param t =
    {
      tp_name = positioned_id t.tp_name;
      tp_variance = t.tp_variance;
      tp_reified = t.tp_reified;
      tp_constraints = constraint_ t.tp_constraints;
      tp_user_attributes = List.map ~f:user_attribute t.tp_user_attributes;
    }

  and package_membership m =
    let open Aast_defs in
    match m with
    | PackageOverride (p, name) -> PackageOverride (pos p, name)
    | PackageConfigAssignment name -> PackageConfigAssignment name

  and package_requirement r =
    match r with
    | RPRequire (p, s) -> RPRequire (pos_or_decl p, s)
    | RPSoft (p, s) -> RPSoft (pos_or_decl p, s)
    | RPNormal -> RPNormal

  and class_type dc =
    {
      dc_final = dc.dc_final;
      dc_const = dc.dc_const;
      dc_internal = dc.dc_internal;
      dc_need_init = dc.dc_need_init;
      dc_deferred_init_members = dc.dc_deferred_init_members;
      dc_abstract = dc.dc_abstract;
      dc_kind = dc.dc_kind;
      dc_is_xhp = dc.dc_is_xhp;
      dc_has_xhp_keyword = dc.dc_has_xhp_keyword;
      dc_module = dc.dc_module;
      dc_is_module_level_trait = dc.dc_is_module_level_trait;
      dc_name = dc.dc_name;
      dc_pos = dc.dc_pos;
      dc_extends = dc.dc_extends;
      dc_sealed_whitelist = dc.dc_sealed_whitelist;
      dc_xhp_attr_deps = dc.dc_xhp_attr_deps;
      dc_xhp_enum_values = dc.dc_xhp_enum_values;
      dc_xhp_marked_empty = dc.dc_xhp_marked_empty;
      dc_req_ancestors = List.map dc.dc_req_ancestors ~f:requirement;
      dc_req_ancestors_extends = dc.dc_req_ancestors_extends;
      dc_req_constraints_ancestors =
        List.map dc.dc_req_constraints_ancestors ~f:constraint_requirement;
      dc_tparams = List.map dc.dc_tparams ~f:type_param;
      dc_substs =
        SMap.map
          begin
            fun ({ sc_subst; _ } as sc) ->
              { sc with sc_subst = SMap.map ty sc_subst }
          end
          dc.dc_substs;
      dc_consts = SMap.map class_const dc.dc_consts;
      dc_typeconsts = SMap.map typeconst_type dc.dc_typeconsts;
      dc_props = dc.dc_props;
      dc_sprops = dc.dc_sprops;
      dc_methods = dc.dc_methods;
      dc_smethods = dc.dc_smethods;
      dc_construct = dc.dc_construct;
      dc_ancestors = SMap.map ty dc.dc_ancestors;
      dc_support_dynamic_type = dc.dc_support_dynamic_type;
      dc_enum_type = Option.map dc.dc_enum_type ~f:enum_type;
      dc_decl_errors = [];
      dc_docs_url = dc.dc_docs_url;
      dc_allow_multiple_instantiations = dc.dc_allow_multiple_instantiations;
      dc_sort_text = dc.dc_sort_text;
      dc_package = Option.map dc.dc_package ~f:package_membership;
    }

  and requirement (p, t) = (pos_or_decl p, ty t)

  and constraint_requirement cr =
    match cr with
    | CR_Equal r -> CR_Equal (requirement r)
    | CR_Subtype r -> CR_Subtype (requirement r)

  and enum_type te =
    {
      te_base = ty te.te_base;
      te_constraint = ty_opt te.te_constraint;
      te_includes = List.map te.te_includes ~f:ty;
    }

  and typedef tdef =
    {
      td_module = tdef.td_module;
      td_pos = pos_or_decl tdef.td_pos;
      td_tparams = List.map tdef.td_tparams ~f:type_param;
      td_as_constraint = ty_opt tdef.td_as_constraint;
      td_super_constraint = ty_opt tdef.td_super_constraint;
      td_type_assignment =
        (match tdef.td_type_assignment with
        | SimpleTypeDef (vis, t) -> SimpleTypeDef (vis, ty t)
        | CaseType (variant, variants) ->
          let f (t, wcs) = (ty t, List.map wcs ~f:where_constraint) in
          CaseType (f variant, List.map variants ~f));
      td_is_ctx = tdef.td_is_ctx;
      td_attributes = List.map tdef.td_attributes ~f:user_attribute;
      td_internal = tdef.td_internal;
      td_docs_url = tdef.td_docs_url;
      td_package = Option.map tdef.td_package ~f:package_membership;
    }

  and shallow_class sc =
    {
      sc_mode = sc.sc_mode;
      sc_final = sc.sc_final;
      sc_abstract = sc.sc_abstract;
      sc_is_xhp = sc.sc_is_xhp;
      sc_internal = sc.sc_internal;
      sc_has_xhp_keyword = sc.sc_has_xhp_keyword;
      sc_kind = sc.sc_kind;
      sc_module = sc.sc_module;
      sc_name = positioned_id sc.sc_name;
      sc_tparams = List.map sc.sc_tparams ~f:type_param;
      sc_extends = List.map sc.sc_extends ~f:ty;
      sc_uses = List.map sc.sc_uses ~f:ty;
      sc_xhp_attr_uses = List.map sc.sc_xhp_attr_uses ~f:ty;
      sc_xhp_enum_values = sc.sc_xhp_enum_values;
      sc_xhp_marked_empty = sc.sc_xhp_marked_empty;
      sc_req_extends = List.map sc.sc_req_extends ~f:ty;
      sc_req_implements = List.map sc.sc_req_implements ~f:ty;
      sc_req_constraints =
        List.map sc.sc_req_constraints ~f:shallow_req_constraint;
      sc_implements = List.map sc.sc_implements ~f:ty;
      sc_support_dynamic_type = sc.sc_support_dynamic_type;
      sc_consts = List.map sc.sc_consts ~f:shallow_class_const;
      sc_typeconsts = List.map sc.sc_typeconsts ~f:shallow_typeconst;
      sc_props = List.map sc.sc_props ~f:shallow_prop;
      sc_sprops = List.map sc.sc_sprops ~f:shallow_prop;
      sc_constructor = Option.map sc.sc_constructor ~f:shallow_method;
      sc_static_methods = List.map sc.sc_static_methods ~f:shallow_method;
      sc_methods = List.map sc.sc_methods ~f:shallow_method;
      sc_user_attributes = List.map sc.sc_user_attributes ~f:user_attribute;
      sc_enum_type = Option.map sc.sc_enum_type ~f:enum_type;
      sc_docs_url = sc.sc_docs_url;
      sc_package = Option.map sc.sc_package ~f:package_membership;
    }

  and shallow_class_const scc =
    {
      scc_abstract = scc.scc_abstract;
      scc_name = positioned_id scc.scc_name;
      scc_type = ty scc.scc_type;
      scc_refs = scc.scc_refs;
      scc_value = scc.scc_value;
    }

  and shallow_typeconst stc =
    {
      stc_kind = typeconst stc.stc_kind;
      stc_name = positioned_id stc.stc_name;
      stc_enforceable =
        (pos_or_decl (fst stc.stc_enforceable), snd stc.stc_enforceable);
      stc_reifiable = Option.map stc.stc_reifiable ~f:pos_or_decl;
      stc_is_ctx = stc.stc_is_ctx;
    }

  and shallow_prop sp =
    {
      sp_name = positioned_id sp.sp_name;
      sp_xhp_attr = sp.sp_xhp_attr;
      sp_type = ty sp.sp_type;
      sp_visibility = sp.sp_visibility;
      sp_flags = sp.sp_flags;
    }

  and shallow_method sm =
    {
      sm_name = positioned_id sm.sm_name;
      sm_type = ty sm.sm_type;
      sm_visibility = sm.sm_visibility;
      sm_deprecated = sm.sm_deprecated;
      sm_flags = sm.sm_flags;
      sm_attributes = sm.sm_attributes;
      sm_sort_text = sm.sm_sort_text;
      sm_package_requirement = package_requirement sm.sm_package_requirement;
    }

  and shallow_req_constraint src =
    match src with
    | DCR_Equal cr -> DCR_Equal (ty cr)
    | DCR_Subtype cr -> DCR_Subtype (ty cr)
end

(*****************************************************************************)
(* Returns a signature with all the positions replaced with Pos.none *)
(*****************************************************************************)
module NormalizeSig = TraversePos (struct
  let pos _ = Pos.none

  let pos_or_decl _ = Pos_or_decl.none
end)
