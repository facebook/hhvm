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
module ShapeMap = Nast.ShapeMap

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
end) =
struct
  open Typing_reason

  let pos = ImplementPos.pos

  let string_id (p, x) = (pos p, x)

  let rec reason = function
    | Rnone -> Rnone
    | Rwitness p -> Rwitness (pos p)
    | Ridx (p, r) -> Ridx (pos p, reason r)
    | Ridx_vector p -> Ridx_vector (pos p)
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
    | Rhint p -> Rhint (pos p)
    | Rthrow p -> Rthrow (pos p)
    | Rplaceholder p -> Rplaceholder (pos p)
    | Rret_div p -> Rret_div (pos p)
    | Ryield_gen p -> Ryield_gen (pos p)
    | Ryield_asyncgen p -> Ryield_asyncgen (pos p)
    | Ryield_asyncnull p -> Ryield_asyncnull (pos p)
    | Ryield_send p -> Ryield_send (pos p)
    | Rlost_info (s, r1, p2, l) -> Rlost_info (s, reason r1, pos p2, l)
    | Rformat (p1, s, r) -> Rformat (pos p1, s, reason r)
    | Rclass_class (p, s) -> Rclass_class (pos p, s)
    | Runknown_class p -> Runknown_class (pos p)
    | Rdynamic_yield (p1, p2, s1, s2) -> Rdynamic_yield (pos p1, pos p2, s1, s2)
    | Rmap_append p -> Rmap_append (pos p)
    | Rvar_param p -> Rvar_param (pos p)
    | Runpack_param (p1, p2, i) -> Runpack_param (pos p1, pos p2, i)
    | Rinout_param p -> Rinout_param (pos p)
    | Rinstantiate (r1, x, r2) -> Rinstantiate (reason r1, x, reason r2)
    | Rarray_filter (p, r) -> Rarray_filter (pos p, reason r)
    | Rtypeconst (r1, (p, s1), s2, r2) ->
      Rtypeconst (reason r1, (pos p, s1), s2, reason r2)
    | Rtype_access (r1, ls) ->
      Rtype_access (reason r1, List.map ls ~f:(fun (r, s) -> (reason r, s)))
    | Rexpr_dep_type (r, p, n) -> Rexpr_dep_type (reason r, pos p, n)
    | Rnullsafe_op p -> Rnullsafe_op (pos p)
    | Rtconst_no_cstr (p, s) -> Rtconst_no_cstr (pos p, s)
    | Rpredicated (p, f) -> Rpredicated (pos p, f)
    | Ris p -> Ris (pos p)
    | Ras p -> Ras (pos p)
    | Rvarray_or_darray_key p -> Rvarray_or_darray_key (pos p)
    | Rusing p -> Rusing (pos p)
    | Rdynamic_prop p -> Rdynamic_prop (pos p)
    | Rdynamic_call p -> Rdynamic_call (pos p)
    | Ridx_dict p -> Ridx_dict (pos p)
    | Rmissing_required_field (p, n) -> Rmissing_required_field (pos p, n)
    | Rmissing_optional_field (p, n) -> Rmissing_optional_field (pos p, n)
    | Runset_field (p, n) -> Runset_field (pos p, n)
    | Rcontravariant_generic (r1, n) -> Rcontravariant_generic (reason r1, n)
    | Rinvariant_generic (r1, n) -> Rcontravariant_generic (reason r1, n)
    | Rregex p -> Rregex (pos p)
    | Rlambda_use p -> Rlambda_use (pos p)
    | Rimplicit_upper_bound (p, s) -> Rimplicit_upper_bound (pos p, s)
    | Rarith_int p -> Rarith_int (pos p)
    | Rarith_ret_int p -> Rarith_ret_int (pos p)
    | Rarith_ret_float (p, r, s) -> Rarith_ret_float (pos p, reason r, s)
    | Rarith_ret_num (p, r, s) -> Rarith_ret_num (pos p, reason r, s)
    | Rarith_dynamic p -> Rarith_dynamic (pos p)
    | Rbitwise_dynamic p -> Rbitwise_dynamic (pos p)
    | Rincdec_dynamic p -> Rincdec_dynamic (pos p)
    | Rtype_variable p -> Rtype_variable (pos p)
    | Rtype_variable_generics (p, t, s) -> Rtype_variable_generics (pos p, t, s)
    | Rsolve_fail p -> Rsolve_fail (pos p)
    | Rcstr_on_generics (p, sid) -> Rcstr_on_generics (pos p, string_id sid)
    | Rlambda_param (p, r) -> Rlambda_param (pos p, reason r)
    | Rshape (p, fun_name) -> Rshape (pos p, fun_name)
    | Renforceable p -> Renforceable (pos p)
    | Rdestructure p -> Rdestructure (pos p)
    | Rkey_value_collection_key p -> Rkey_value_collection_key (pos p)
    | Rglobal_class_prop p -> Rglobal_class_prop (pos p)
    | Rglobal_fun_param p -> Rglobal_fun_param (pos p)
    | Rglobal_fun_ret p -> Rglobal_fun_ret (pos p)

  let pos_mapper =
    object
      inherit [_] Aast.map

      method! on_pos _ p = pos p

      method on_'fb _ fb = fb

      method on_'en _ en = en

      method on_'ex _ ex = pos ex

      method on_'hi _ hi = hi
    end

  let rec ty t =
    let (p, x) = deref t in
    mk (reason p, ty_ x)

  and ty_ : decl_phase ty_ -> decl_phase ty_ = function
    | (Tany _ | Tthis | Terr | Tmixed | Tnonnull | Tdynamic | Tvar _) as x -> x
    | Tarray (ty1, ty2) -> Tarray (ty_opt ty1, ty_opt ty2)
    | Tdarray (ty1, ty2) -> Tdarray (ty ty1, ty ty2)
    | Tvarray root_ty -> Tvarray (ty root_ty)
    | Tvarray_or_darray (ty1_opt, ty2) ->
      Tvarray_or_darray (ty_opt ty1_opt, ty ty2)
    | Tprim _ as x -> x
    | Tgeneric _ as x -> x
    | Ttuple tyl -> Ttuple (List.map tyl ty)
    | Tunion tyl -> Tunion (List.map tyl ty)
    | Tintersection tyl -> Tintersection (List.map tyl ty)
    | Toption x -> Toption (ty x)
    | Tlike x -> Tlike (ty x)
    | Tfun ft -> Tfun (fun_type ft)
    | Tapply (sid, xl) -> Tapply (string_id sid, List.map xl ty)
    | Taccess (root_ty, ids) -> Taccess (ty root_ty, List.map ids string_id)
    | Tshape (shape_kind, fdm) ->
      Tshape (shape_kind, ShapeFieldMap.map_and_rekey fdm shape_field_name ty)
    | Tpu_access (base, sid, pu_loc) ->
      Tpu_access (ty base, string_id sid, pu_loc)

  and ty_opt x = Option.map x ty

  and shape_field_name = function
    | Ast_defs.SFlit_int s -> Ast_defs.SFlit_int (string_id s)
    | Ast_defs.SFlit_str s -> Ast_defs.SFlit_str (string_id s)
    | Ast_defs.SFclass_const (id, s) ->
      Ast_defs.SFclass_const (string_id id, string_id s)

  and constraint_ x = List.map ~f:(fun (ck, x) -> (ck, ty x)) x

  and possibly_enforced_ty et = { et with et_type = ty et.et_type }

  and fun_type ft =
    {
      ft with
      ft_tparams = List.map ~f:type_param ft.ft_tparams;
      ft_where_constraints = List.map ft.ft_where_constraints where_constraint;
      ft_params = List.map ft.ft_params fun_param;
      ft_ret = possibly_enforced_ty ft.ft_ret;
      ft_arity = fun_arity ft.ft_arity;
      ft_reactive = fun_reactive ft.ft_reactive;
    }

  and fun_elt fe =
    {
      fe with
      fe_type = ty fe.fe_type;
      fe_pos = pos fe.fe_pos;
      fe_decl_errors = None;
    }

  and fun_reactive = function
    | Local (Some ty1) -> Local (Some (ty ty1))
    | Shallow (Some ty1) -> Shallow (Some (ty ty1))
    | Reactive (Some ty1) -> Reactive (Some (ty ty1))
    | r -> r

  and where_constraint (ty1, c, ty2) = (ty ty1, c, ty ty2)

  and fun_arity = function
    | Fstandard _ as x -> x
    | Fellipsis (n, p) -> Fellipsis (n, pos p)
    | Fvariadic (n, param) -> Fvariadic (n, fun_param param)

  and fun_param param =
    {
      param with
      fp_pos = pos param.fp_pos;
      fp_type = possibly_enforced_ty param.fp_type;
      fp_rx_annotation = param_rx_annotation param.fp_rx_annotation;
    }

  and param_rx_annotation = function
    | Some (Param_rx_if_impl t) -> Some (Param_rx_if_impl (ty t))
    | c -> c

  and class_const cc =
    {
      cc_synthesized = cc.cc_synthesized;
      cc_abstract = cc.cc_abstract;
      cc_pos = pos cc.cc_pos;
      cc_type = ty cc.cc_type;
      cc_expr = Option.map ~f:(pos_mapper#on_expr ()) cc.cc_expr;
      cc_origin = cc.cc_origin;
    }

  and typeconst_abstract_kind = function
    | TCAbstract default -> TCAbstract (ty_opt default)
    | TCPartiallyAbstract -> TCPartiallyAbstract
    | TCConcrete -> TCConcrete

  and typeconst tc =
    {
      ttc_abstract = typeconst_abstract_kind tc.ttc_abstract;
      ttc_name = string_id tc.ttc_name;
      ttc_constraint = ty_opt tc.ttc_constraint;
      ttc_type = ty_opt tc.ttc_type;
      ttc_origin = tc.ttc_origin;
      ttc_enforceable = Tuple.T2.map_fst ~f:pos tc.ttc_enforceable;
      ttc_reifiable = Option.map tc.ttc_reifiable pos;
    }

  and pu_enum_member pum =
    {
      tpum_atom = string_id pum.tpum_atom;
      tpum_types =
        SMap.map
          begin
            fun (id, t) ->
            (string_id id, ty t)
          end
          pum.tpum_types;
      tpum_exprs =
        SMap.map
          begin
            fun id ->
            string_id id
          end
          pum.tpum_exprs;
    }

  and pu_enum pu =
    {
      tpu_name = string_id pu.tpu_name;
      tpu_is_final = pu.tpu_is_final;
      tpu_case_types =
        SMap.map (fun (sid, k) -> (string_id sid, k)) pu.tpu_case_types;
      tpu_case_values =
        SMap.map
          begin
            fun (id, t) ->
            (string_id id, ty t)
          end
          pu.tpu_case_values;
      tpu_members = SMap.map pu_enum_member pu.tpu_members;
    }

  and user_attribute ua =
    {
      Aast.ua_name = string_id ua.Aast.ua_name;
      ua_params = List.map ~f:(pos_mapper#on_expr ()) ua.Aast.ua_params;
    }

  and type_param t =
    {
      t with
      tp_name = string_id t.tp_name;
      tp_constraints = constraint_ t.tp_constraints;
      tp_user_attributes = List.map ~f:user_attribute t.tp_user_attributes;
    }

  and class_type dc =
    {
      dc_final = dc.dc_final;
      dc_const = dc.dc_const;
      dc_ppl = dc.dc_ppl;
      dc_need_init = dc.dc_need_init;
      dc_deferred_init_members = dc.dc_deferred_init_members;
      dc_abstract = dc.dc_abstract;
      dc_members_fully_known = dc.dc_members_fully_known;
      dc_kind = dc.dc_kind;
      dc_is_xhp = dc.dc_is_xhp;
      dc_has_xhp_keyword = dc.dc_has_xhp_keyword;
      dc_is_disposable = dc.dc_is_disposable;
      dc_name = dc.dc_name;
      dc_pos = dc.dc_pos;
      dc_extends = dc.dc_extends;
      dc_sealed_whitelist = dc.dc_sealed_whitelist;
      dc_xhp_attr_deps = dc.dc_xhp_attr_deps;
      dc_req_ancestors = List.map dc.dc_req_ancestors requirement;
      dc_req_ancestors_extends = dc.dc_req_ancestors_extends;
      dc_tparams = List.map dc.dc_tparams type_param;
      dc_where_constraints = List.map dc.dc_where_constraints where_constraint;
      dc_substs =
        SMap.map
          begin
            fun ({ sc_subst; _ } as sc) ->
            { sc with sc_subst = SMap.map ty sc_subst }
          end
          dc.dc_substs;
      dc_consts = SMap.map class_const dc.dc_consts;
      dc_typeconsts = SMap.map typeconst dc.dc_typeconsts;
      dc_pu_enums = SMap.map pu_enum dc.dc_pu_enums;
      dc_props = dc.dc_props;
      dc_sprops = dc.dc_sprops;
      dc_methods = dc.dc_methods;
      dc_smethods = dc.dc_smethods;
      dc_construct = dc.dc_construct;
      dc_ancestors = SMap.map ty dc.dc_ancestors;
      dc_enum_type = Option.map dc.dc_enum_type enum_type;
      dc_decl_errors = None;
      dc_condition_types = dc.dc_condition_types;
    }

  and requirement (p, t) = (pos p, ty t)

  and enum_type te =
    { te_base = ty te.te_base; te_constraint = ty_opt te.te_constraint }

  and typedef tdef =
    {
      td_pos = pos tdef.td_pos;
      td_vis = tdef.td_vis;
      td_tparams = List.map tdef.td_tparams type_param;
      td_constraint = ty_opt tdef.td_constraint;
      td_type = ty tdef.td_type;
      td_decl_errors = None;
    }

  and shallow_class sc =
    {
      sc_mode = sc.sc_mode;
      sc_final = sc.sc_final;
      sc_is_xhp = sc.sc_is_xhp;
      sc_has_xhp_keyword = sc.sc_has_xhp_keyword;
      sc_kind = sc.sc_kind;
      sc_name = string_id sc.sc_name;
      sc_tparams = List.map sc.sc_tparams type_param;
      sc_where_constraints = List.map sc.sc_where_constraints where_constraint;
      sc_extends = List.map sc.sc_extends ty;
      sc_uses = List.map sc.sc_uses ty;
      sc_method_redeclarations = sc.sc_method_redeclarations;
      sc_xhp_attr_uses = List.map sc.sc_xhp_attr_uses ty;
      sc_req_extends = List.map sc.sc_req_extends ty;
      sc_req_implements = List.map sc.sc_req_implements ty;
      sc_implements = List.map sc.sc_implements ty;
      sc_consts = List.map sc.sc_consts shallow_class_const;
      sc_typeconsts = List.map sc.sc_typeconsts shallow_typeconst;
      sc_pu_enums = List.map sc.sc_pu_enums shallow_pu_enum;
      sc_props = List.map sc.sc_props shallow_prop;
      sc_sprops = List.map sc.sc_sprops shallow_prop;
      sc_constructor = Option.map sc.sc_constructor shallow_method;
      sc_static_methods = List.map sc.sc_static_methods shallow_method;
      sc_methods = List.map sc.sc_methods shallow_method;
      sc_user_attributes = List.map sc.sc_user_attributes user_attribute;
      sc_enum_type = Option.map sc.sc_enum_type enum_type;
      sc_decl_errors = Errors.empty;
    }

  and shallow_class_const scc =
    {
      scc_abstract = scc.scc_abstract;
      scc_expr = Option.map scc.scc_expr (pos_mapper#on_expr ());
      scc_name = string_id scc.scc_name;
      scc_type = ty scc.scc_type;
    }

  and shallow_typeconst stc =
    {
      stc_abstract = typeconst_abstract_kind stc.stc_abstract;
      stc_constraint = Option.map stc.stc_constraint ty;
      stc_name = string_id stc.stc_name;
      stc_type = Option.map stc.stc_type ty;
      stc_enforceable = (pos (fst stc.stc_enforceable), snd stc.stc_enforceable);
      stc_reifiable = Option.map stc.stc_reifiable pos;
    }

  and shallow_pu_member spum =
    {
      spum_atom = string_id spum.spum_atom;
      spum_types = List.map spum.spum_types (fun (s, t) -> (string_id s, ty t));
      spum_exprs = List.map spum.spum_exprs string_id;
    }

  and shallow_pu_enum spu =
    {
      spu_name = string_id spu.spu_name;
      spu_is_final = spu.spu_is_final;
      spu_case_types =
        List.map ~f:(fun (sid, k) -> (string_id sid, k)) spu.spu_case_types;
      spu_case_values =
        List.map spu.spu_case_values (fun (s, t) -> (string_id s, ty t));
      spu_members = List.map spu.spu_members shallow_pu_member;
    }

  and shallow_prop sp =
    {
      sp_const = sp.sp_const;
      sp_xhp_attr = sp.sp_xhp_attr;
      sp_lateinit = sp.sp_lateinit;
      sp_lsb = sp.sp_lsb;
      sp_name = string_id sp.sp_name;
      sp_needs_init = sp.sp_needs_init;
      sp_type = Option.map sp.sp_type ty;
      sp_abstract = sp.sp_abstract;
      sp_visibility = sp.sp_visibility;
      sp_fixme_codes = ISet.empty;
    }

  and shallow_method sm =
    {
      sm_abstract = sm.sm_abstract;
      sm_final = sm.sm_final;
      sm_memoizelsb = sm.sm_memoizelsb;
      sm_name = string_id sm.sm_name;
      sm_override = sm.sm_override;
      sm_dynamicallycallable = sm.sm_dynamicallycallable;
      sm_reactivity = sm.sm_reactivity;
      sm_type = ty sm.sm_type;
      sm_visibility = sm.sm_visibility;
      sm_fixme_codes = ISet.empty;
      sm_deprecated = sm.sm_deprecated;
    }
end

(*****************************************************************************)
(* Returns a signature with all the positions replaced with Pos.none *)
(*****************************************************************************)
module NormalizeSig = TraversePos (struct
  let pos _ = Pos.none
end)
