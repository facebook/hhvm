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

  let rec reason : type ph. ph Typing_reason.t_ -> ph Typing_reason.t_ =
    function
    | Rnone -> Rnone
    | Rwitness p -> Rwitness (pos p)
    | Rwitness_from_decl p -> Rwitness_from_decl (pos_or_decl p)
    | Ridx (p, r) -> Ridx (pos p, reason r)
    | Ridx_vector p -> Ridx_vector (pos p)
    | Ridx_vector_from_decl p -> Ridx_vector_from_decl (pos_or_decl p)
    | Rforeach p -> Rforeach (pos p)
    | Rasyncforeach p -> Rasyncforeach (pos p)
    | Rarith p -> Rarith (pos p)
    | Rarith_ret p -> Rarith_ret (pos p)
    | Rcomp p -> Rcomp (pos p)
    | Rconcat_ret p -> Rconcat_ret (pos p)
    | Rlogic_ret p -> Rlogic_ret (pos p)
    | Rbitwise p -> Rbitwise (pos p)
    | Rbitwise_ret p -> Rbitwise_ret (pos p)
    | Rno_return p -> Rno_return (pos p)
    | Rno_return_async p -> Rno_return_async (pos p)
    | Rret_fun_kind (p, k) -> Rret_fun_kind (pos p, k)
    | Rret_fun_kind_from_decl (p, k) ->
      Rret_fun_kind_from_decl (pos_or_decl p, k)
    | Rhint p -> Rhint (pos_or_decl p)
    | Rthrow p -> Rthrow (pos p)
    | Rplaceholder p -> Rplaceholder (pos p)
    | Rret_div p -> Rret_div (pos p)
    | Ryield_gen p -> Ryield_gen (pos p)
    | Ryield_asyncgen p -> Ryield_asyncgen (pos p)
    | Ryield_asyncnull p -> Ryield_asyncnull (pos p)
    | Ryield_send p -> Ryield_send (pos p)
    | Rlost_info (s, r1, Blame (p2, l)) ->
      Rlost_info (s, reason r1, Blame (pos p2, l))
    | Rformat (p1, s, r) -> Rformat (pos p1, s, reason r)
    | Rclass_class (p, s) -> Rclass_class (pos_or_decl p, s)
    | Runknown_class p -> Runknown_class (pos p)
    | Rvar_param p -> Rvar_param (pos p)
    | Rvar_param_from_decl p -> Rvar_param_from_decl (pos_or_decl p)
    | Runpack_param (p1, p2, i) -> Runpack_param (pos p1, pos_or_decl p2, i)
    | Rinout_param p -> Rinout_param (pos_or_decl p)
    | Rinstantiate (r1, x, r2) -> Rinstantiate (reason r1, x, reason r2)
    | Rtypeconst (r1, (p, s1), s2, r2) ->
      Rtypeconst (reason r1, (pos_or_decl p, s1), s2, reason r2)
    | Rtype_access (r1, ls) ->
      Rtype_access (reason r1, List.map ls ~f:(fun (r, s) -> (reason r, s)))
    | Rexpr_dep_type (r, p, n) -> Rexpr_dep_type (reason r, pos_or_decl p, n)
    | Rnullsafe_op p -> Rnullsafe_op (pos p)
    | Rtconst_no_cstr id -> Rtconst_no_cstr (positioned_id id)
    | Rpredicated (p, f) -> Rpredicated (pos p, f)
    | Ris p -> Ris (pos p)
    | Ras p -> Ras (pos p)
    | Requal p -> Requal (pos p)
    | Rvarray_or_darray_key p -> Rvarray_or_darray_key (pos_or_decl p)
    | Rvec_or_dict_key p -> Rvec_or_dict_key (pos_or_decl p)
    | Rusing p -> Rusing (pos p)
    | Rdynamic_prop p -> Rdynamic_prop (pos p)
    | Rdynamic_call p -> Rdynamic_call (pos p)
    | Rdynamic_construct p -> Rdynamic_construct (pos p)
    | Ridx_dict p -> Ridx_dict (pos p)
    | Rset_element p -> Rset_element (pos p)
    | Rmissing_optional_field (p, n) ->
      Rmissing_optional_field (pos_or_decl p, n)
    | Runset_field (p, n) -> Runset_field (pos p, n)
    | Rcontravariant_generic (r1, n) -> Rcontravariant_generic (reason r1, n)
    | Rinvariant_generic (r1, n) -> Rcontravariant_generic (reason r1, n)
    | Rregex p -> Rregex (pos p)
    | Rimplicit_upper_bound (p, s) -> Rimplicit_upper_bound (pos_or_decl p, s)
    | Rarith_ret_int p -> Rarith_ret_int (pos p)
    | Rarith_ret_float (p, r, s) -> Rarith_ret_float (pos p, reason r, s)
    | Rarith_ret_num (p, r, s) -> Rarith_ret_num (pos p, reason r, s)
    | Rarith_dynamic p -> Rarith_dynamic (pos p)
    | Rbitwise_dynamic p -> Rbitwise_dynamic (pos p)
    | Rincdec_dynamic p -> Rincdec_dynamic (pos p)
    | Rtype_variable p -> Rtype_variable (pos p)
    | Rtype_variable_error p -> Rtype_variable_error (pos p)
    | Rtype_variable_generics (p, t, s) -> Rtype_variable_generics (pos p, t, s)
    | Rglobal_type_variable_generics (p, t, s) ->
      Rglobal_type_variable_generics (pos_or_decl p, t, s)
    | Rsolve_fail p -> Rsolve_fail (pos_or_decl p)
    | Rcstr_on_generics (p, sid) ->
      Rcstr_on_generics (pos_or_decl p, positioned_id sid)
    | Rlambda_param (p, r) -> Rlambda_param (pos p, reason r)
    | Rshape (p, fun_name) -> Rshape (pos p, fun_name)
    | Rshape_literal p -> Rshape_literal (pos p)
    | Renforceable p -> Renforceable (pos_or_decl p)
    | Rdestructure p -> Rdestructure (pos p)
    | Rkey_value_collection_key p -> Rkey_value_collection_key (pos p)
    | Rglobal_class_prop p -> Rglobal_class_prop (pos_or_decl p)
    | Rglobal_fun_param p -> Rglobal_fun_param (pos_or_decl p)
    | Rglobal_fun_ret p -> Rglobal_fun_ret (pos_or_decl p)
    | Rsplice p -> Rsplice (pos p)
    | Ret_boolean p -> Ret_boolean (pos p)
    | Rdefault_capability p -> Rdefault_capability (pos_or_decl p)
    | Rconcat_operand p -> Rconcat_operand (pos p)
    | Rinterp_operand p -> Rinterp_operand (pos p)
    | Rdynamic_coercion r -> Rdynamic_coercion (reason r)
    | Rsupport_dynamic_type p -> Rsupport_dynamic_type (pos_or_decl p)
    | Rdynamic_partial_enforcement (p, cn, r) ->
      Rdynamic_partial_enforcement (pos_or_decl p, cn, reason r)
    | Rrigid_tvar_escape (p, v, w, r) ->
      Rrigid_tvar_escape (pos p, v, w, reason r)
    | Ropaque_type_from_module (p, m, r) ->
      Ropaque_type_from_module (pos_or_decl p, m, reason r)
    | Rmissing_class p -> Rmissing_class (pos p)
    | Rinvalid -> Rinvalid
    | Rcaptured_like p -> Rcaptured_like (pos p)
    | Rpessimised_inout p -> Rpessimised_inout (pos_or_decl p)
    | Rpessimised_return p -> Rpessimised_return (pos_or_decl p)
    | Rpessimised_prop p -> Rpessimised_prop (pos_or_decl p)
    | Runsafe_cast p -> Runsafe_cast (pos p)
    | Rpattern p -> Rpattern (pos p)

  let rec ty t =
    let (p, x) = deref t in
    mk (reason p, ty_ x)

  and ty_ : decl_phase ty_ -> decl_phase ty_ = function
    | (Tany _ | Tthis | Tmixed | Twildcard | Tnonnull | Tdynamic) as x -> x
    | Tvec_or_dict (ty1, ty2) -> Tvec_or_dict (ty ty1, ty ty2)
    | Tprim _ as x -> x
    | Tgeneric (name, args) -> Tgeneric (name, List.map args ~f:ty)
    | Ttuple tyl -> Ttuple (List.map tyl ~f:ty)
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
    | Tnewtype (name, tyl, bound) ->
      let tyl = List.map tyl ~f:ty in
      let bound = ty bound in
      Tnewtype (name, tyl, bound)

  and ty_opt x = Option.map x ~f:ty

  and shape_field_name = function
    | Typing_defs.TSFlit_int (p, s) -> Typing_defs.TSFlit_int (pos_or_decl p, s)
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
    { fe with fe_type = ty fe.fe_type; fe_pos = pos_or_decl fe.fe_pos }

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

  and user_attribute { ua_name; ua_params } =
    { ua_name = positioned_id ua_name; ua_params }

  and type_param t =
    {
      tp_name = positioned_id t.tp_name;
      tp_variance = t.tp_variance;
      tp_reified = t.tp_reified;
      tp_tparams = List.map ~f:type_param t.tp_tparams;
      tp_constraints = constraint_ t.tp_constraints;
      tp_user_attributes = List.map ~f:user_attribute t.tp_user_attributes;
    }

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
      dc_req_class_ancestors = List.map dc.dc_req_class_ancestors ~f:requirement;
      dc_tparams = List.map dc.dc_tparams ~f:type_param;
      dc_where_constraints =
        List.map dc.dc_where_constraints ~f:where_constraint;
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
    }

  and requirement (p, t) = (pos_or_decl p, ty t)

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
      td_vis = tdef.td_vis;
      td_tparams = List.map tdef.td_tparams ~f:type_param;
      td_as_constraint = ty_opt tdef.td_as_constraint;
      td_super_constraint = ty_opt tdef.td_super_constraint;
      td_type = ty tdef.td_type;
      td_is_ctx = tdef.td_is_ctx;
      td_attributes = List.map tdef.td_attributes ~f:user_attribute;
      td_internal = tdef.td_internal;
      td_docs_url = tdef.td_docs_url;
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
      sc_where_constraints =
        List.map sc.sc_where_constraints ~f:where_constraint;
      sc_extends = List.map sc.sc_extends ~f:ty;
      sc_uses = List.map sc.sc_uses ~f:ty;
      sc_xhp_attr_uses = List.map sc.sc_xhp_attr_uses ~f:ty;
      sc_xhp_enum_values = sc.sc_xhp_enum_values;
      sc_xhp_marked_empty = sc.sc_xhp_marked_empty;
      sc_req_extends = List.map sc.sc_req_extends ~f:ty;
      sc_req_implements = List.map sc.sc_req_implements ~f:ty;
      sc_req_class = List.map sc.sc_req_class ~f:ty;
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
    }
end

(*****************************************************************************)
(* Returns a signature with all the positions replaced with Pos.none *)
(*****************************************************************************)
module NormalizeSig = TraversePos (struct
  let pos _ = Pos.none

  let pos_or_decl _ = Pos_or_decl.none
end)
