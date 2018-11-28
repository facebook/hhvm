(**
* Copyright (c) 2015, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the "hack" directory of this source tree.
*
*)

open Core_kernel
open Decl_defs
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
module TraversePos(ImplementPos: sig val pos: Pos.t -> Pos.t end) = struct
open Typing_reason

let pos = ImplementPos.pos

let rec reason = function
  | Rnone                  -> Rnone
  | Rwitness p             -> Rwitness (pos p)
  | Ridx (p, r)            -> Ridx (pos p, reason r)
  | Ridx_vector p          -> Ridx_vector (pos p)
  | Rappend p              -> Rappend (pos p)
  | Rfield p               -> Rfield (pos p)
  | Rforeach p             -> Rforeach (pos p)
  | Rasyncforeach p        -> Rasyncforeach (pos p)
  | Raccess p              -> Raccess (pos p)
  | Rarith p               -> Rarith (pos p)
  | Rarith_ret p           -> Rarith_ret (pos p)
  | Rstring2 p             -> Rstring2 (pos p)
  | Rcomp p                -> Rcomp (pos p)
  | Rconcat p              -> Rconcat (pos p)
  | Rconcat_ret p          -> Rconcat_ret (pos p)
  | Rlogic p               -> Rlogic (pos p)
  | Rlogic_ret p           -> Rlogic_ret (pos p)
  | Rbitwise p             -> Rbitwise (pos p)
  | Rbitwise_ret p         -> Rbitwise_ret (pos p)
  | Rstmt p                -> Rstmt (pos p)
  | Rno_return p           -> Rno_return (pos p)
  | Rno_return_async p     -> Rno_return_async (pos p)
  | Rret_fun_kind (p, k)   -> Rret_fun_kind (pos p, k)
  | Rhint p                -> Rhint (pos p)
  | Rnull_check p          -> Rnull_check (pos p)
  | Rnot_in_cstr p         -> Rnot_in_cstr (pos p)
  | Rthrow p               -> Rthrow (pos p)
  | Rplaceholder p         -> Rplaceholder (pos p)
  | Rattr p                -> Rattr (pos p)
  | Rxhp p                 -> Rxhp (pos p)
  | Rret_div p             -> Rret_div (pos p)
  | Ryield_gen p           -> Ryield_gen (pos p)
  | Ryield_asyncgen p      -> Ryield_asyncgen (pos p)
  | Ryield_asyncnull p     -> Ryield_asyncnull (pos p)
  | Ryield_send p          -> Ryield_send (pos p)
  | Rlost_info (s, r1, p2) -> Rlost_info (s, reason r1, pos p2)
  | Rcoerced (r1, p2, x)   -> Rcoerced (reason r1, pos p2, x)
  | Rformat (p1, s, r)     -> Rformat (pos p1, s, reason r)
  | Rclass_class (p, s)    -> Rclass_class (pos p, s)
  | Runknown_class p       -> Runknown_class (pos p)
  | Rdynamic_yield (p1, p2, s1, s2) -> Rdynamic_yield(pos p1, pos p2, s1, s2)
  | Rmap_append p          -> Rmap_append (pos p)
  | Rvar_param p           -> Rvar_param (pos p)
  | Runpack_param p        -> Runpack_param (pos p)
  | Rinout_param p         -> Rinout_param (pos p)
  | Rinstantiate (r1,x,r2) -> Rinstantiate (reason r1, x, reason r2)
  | Rarray_filter (p, r)   -> Rarray_filter (pos p, reason r)
  | Rtype_access (r1, x, r2) -> Rtype_access (reason r1, x, reason r2)
  | Rexpr_dep_type (r, p, n) -> Rexpr_dep_type (reason r, pos p, n)
  | Rnullsafe_op p           -> Rnullsafe_op (pos p)
  | Rtconst_no_cstr (p, s)   -> Rtconst_no_cstr (pos p, s)
  | Rused_as_map p           -> Rused_as_map (pos p)
  | Rused_as_shape p         -> Rused_as_shape (pos p)
  | Rpredicated (p, f)       -> Rpredicated (pos p, f)
  | Rinstanceof (p, f)       -> Rinstanceof (pos p, f)
  | Ris p                    -> Ris (pos p)
  | Ras p                    -> Ras (pos p)
  | Rfinal_property p        -> Rfinal_property (pos p)
  | Rvarray_or_darray_key p -> Rvarray_or_darray_key (pos p)
  | Rusing p                 -> Rusing (pos p)
  | Rdynamic_prop p          -> Rdynamic_prop (pos p)
  | Rdynamic_call p          -> Rdynamic_call (pos p)
  | Ridx_dict p              -> Ridx_dict (pos p)
  | Rmissing_optional_field (p, n) -> Rmissing_optional_field (pos p, n)
  | Rcontravariant_generic (r1, n) -> Rcontravariant_generic (reason r1, n)
  | Rinvariant_generic (r1, n) -> Rcontravariant_generic (reason r1, n)
  | Rregex p                 -> Rregex (pos p)
  | Rlambda_use p            -> Rlambda_use (pos p)
  | Rimplicit_upper_bound (p, s) -> Rimplicit_upper_bound (pos p, s)
  | Rarith_int p -> Rarith_int (pos p)
  | Rarith_ret_int p -> Rarith_ret_int (pos p)
  | Rarith_ret_float (p, r, s) -> Rarith_ret_float (pos p, reason r, s)
  | Rarith_ret_num (p, r, s) -> Rarith_ret_num (pos p, reason r, s)
  | Rsum_dynamic p -> Rsum_dynamic (pos p)
  | Rbitwise_dynamic p -> Rbitwise_dynamic (pos p)
  | Rincdec_dynamic p -> Rincdec_dynamic (pos p)
  | Rtype_variable p -> Rtype_variable (pos p)

let string_id (p, x) = pos p, x

let rec ty (p, x) =
  reason p, ty_ x

  and ty_: decl ty_ -> decl ty_ = function
    | (Tany | Tthis | Terr | Tmixed | Tnonnull | Tdynamic) as x -> x
    | Tarray (ty1, ty2)    -> Tarray (ty_opt ty1, ty_opt ty2)
    | Tdarray (ty1, ty2)   -> Tdarray (ty ty1, ty ty2)
    | Tvarray root_ty      -> Tvarray (ty root_ty)
    | Tvarray_or_darray root_ty -> Tvarray_or_darray (ty root_ty)
    | Tprim _ as x         -> x
    | Tgeneric _ as x      -> x
    | Ttuple tyl           -> Ttuple (List.map tyl ty)
    | Toption x            -> Toption (ty x)
    | Tfun ft              -> Tfun (fun_type ft)
    | Tapply (sid, xl)     -> Tapply (string_id sid, List.map xl ty)
    | Taccess (root_ty, ids) ->
        Taccess (ty root_ty, List.map ids string_id)
    | Tshape (fields_known, fdm) ->
        Tshape (shape_fields_known fields_known,
          ShapeFieldMap.map_and_rekey fdm shape_field_name ty)

  and ty_opt x = Option.map x ty

  and shape_fields_known = function
    | FieldsFullyKnown -> FieldsFullyKnown
    | FieldsPartiallyKnown m ->
      FieldsPartiallyKnown (ShapeMap.map_and_rekey m shape_field_name pos)

  and shape_field_name = function
    | Ast.SFlit_int s -> Ast.SFlit_int (string_id s)
    | Ast.SFlit_str s -> Ast.SFlit_str (string_id s)
    | Ast.SFclass_const (id, s) -> Ast.SFclass_const (string_id id, string_id s)

  and constraint_ = List.map ~f:(fun (ck, x) -> (ck, ty x))

  and fun_type ft =
    { ft with
      ft_tparams = List.map ft.ft_tparams type_param   ;
      ft_where_constraints = List.map
        ft.ft_where_constraints where_constraint       ;
      ft_params  = List.map ft.ft_params fun_param     ;
      ft_ret     = ty ft.ft_ret                        ;
      ft_pos     = pos ft.ft_pos                       ;
      ft_arity   = fun_arity ft.ft_arity               ;
      ft_reactive = fun_reactive ft.ft_reactive        ;
      ft_decl_errors = None                            ;
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
    { param with
      fp_pos = pos param.fp_pos;
      fp_type = ty param.fp_type;
      fp_rx_annotation = param_rx_annotation param.fp_rx_annotation
    }

  and param_rx_annotation = function
    | Some (Param_rx_if_impl t) -> Some (Param_rx_if_impl (ty t))
    | c -> c

  and class_const cc =
    { cc_synthesized = cc.cc_synthesized;
      cc_abstract = cc.cc_abstract;
      cc_pos = pos cc.cc_pos;
      cc_type = ty cc.cc_type;
      cc_expr = Option.map cc.cc_expr (Nast_pos_mapper.expr pos);
      cc_origin = cc.cc_origin;
    }

  and typeconst tc =
    { ttc_name = string_id tc.ttc_name;
      ttc_constraint = ty_opt tc.ttc_constraint;
      ttc_type = ty_opt tc.ttc_type;
      ttc_origin = tc.ttc_origin;
    }

  and type_param (variance, sid, x, reified) =
    variance, string_id sid, constraint_ x, reified

  and class_type dc =
    { dc_final                 = dc.dc_final                          ;
      dc_const                 = dc.dc_const                          ;
      dc_ppl                   = dc.dc_ppl                            ;
      dc_need_init             = dc.dc_need_init                      ;
      dc_deferred_init_members = dc.dc_deferred_init_members          ;
      dc_abstract              = dc.dc_abstract                       ;
      dc_members_fully_known   = dc.dc_members_fully_known            ;
      dc_kind                  = dc.dc_kind                           ;
      dc_is_xhp                = dc.dc_is_xhp                         ;
      dc_is_disposable         = dc.dc_is_disposable                  ;
      dc_name                  = dc.dc_name                           ;
      dc_pos                   = dc.dc_pos                            ;
      dc_extends               = dc.dc_extends                        ;
      dc_sealed_whitelist      = dc.dc_sealed_whitelist               ;
      dc_xhp_attr_deps         = dc.dc_xhp_attr_deps                  ;
      dc_req_ancestors         = List.map dc.dc_req_ancestors
        requirement                                                   ;
      dc_req_ancestors_extends = dc.dc_req_ancestors_extends          ;
      dc_tparams               = List.map dc.dc_tparams type_param    ;
      dc_substs                = SMap.map begin fun ({ sc_subst; _ } as sc) ->
        {sc with sc_subst = SMap.map ty sc_subst}
      end dc.dc_substs;
      dc_consts                = SMap.map class_const dc.dc_consts    ;
      dc_typeconsts            = SMap.map typeconst dc.dc_typeconsts  ;
      dc_props                 = dc.dc_props                          ;
      dc_sprops                = dc.dc_sprops                         ;
      dc_methods               = dc.dc_methods                        ;
      dc_smethods              = dc.dc_smethods                       ;
      dc_construct             = dc.dc_construct                      ;
      dc_ancestors             = SMap.map ty dc.dc_ancestors          ;
      dc_enum_type             = Option.map dc.dc_enum_type enum_type ;
      dc_decl_errors           = None                                 ;
      dc_condition_types       = dc.dc_condition_types                ;
      dc_linearization         = dc.dc_linearization                  ;
    }

  and requirement (p, t) = (pos p, ty t)

  and enum_type te =
    { te_base       = ty te.te_base           ;
      te_constraint = ty_opt te.te_constraint ;
    }

  and typedef tdef =
    { td_pos        = pos tdef.td_pos                     ;
      td_vis        = tdef.td_vis                         ;
      td_tparams    = List.map tdef.td_tparams type_param ;
      td_constraint = ty_opt tdef.td_constraint           ;
      td_type       = ty tdef.td_type                     ;
      td_decl_errors = None;
    }
end

(*****************************************************************************)
(* Returns a signature with all the positions replaced with Pos.none *)
(*****************************************************************************)
module NormalizeSig = TraversePos(struct let pos _ = Pos.none end)
