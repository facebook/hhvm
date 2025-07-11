(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
open Typing_env_types
open Utils
module Env = Typing_env
module FunUtils = Decl_fun_utils
module Inst = Decl_instantiate
module Phase = Typing_phase
module Subst = Decl_subst

(** This module checks well-formedness of type hints. See .mli file for more. *)

type env = {
  typedef_tparams: Nast.tparam list;
  tenv: Typing_env_types.env;
}

let check_tparams_constraints env use_pos tparams targs =
  let ety_env : expand_env =
    { empty_expand_env with substs = Subst.make_locl tparams targs }
  in
  Phase.check_tparams_constraints ~use_pos ~ety_env env tparams

let loclty_of_hint unchecked_tparams env h =
  let hint_pos = fst h in
  let decl_ty = Decl_hint.hint env.decl_env h in
  let unchecked_tparams =
    List.map
      unchecked_tparams
      ~f:(Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
  in
  let tyl =
    List.map unchecked_tparams ~f:(fun t ->
        mk (Reason.witness_from_decl (fst t.tp_name), Typing_defs.make_tany ()))
  in
  let subst = Inst.make_subst unchecked_tparams tyl in
  let decl_ty = Inst.instantiate subst decl_ty in
  let ety_env =
    empty_expand_env_with_on_error
      (Typing_error.Reasons_callback.invalid_type_hint hint_pos)
  in
  let ety_env = { ety_env with visibility_behavior = Never_expand_newtype } in
  let ((env, ty_err_opt), locl_ty) = Phase.localize env ~ety_env decl_ty in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, hint_pos, locl_ty)

let check_hrefinement unchecked_tparams env h rl =
  let (env, _, locl_ty) = loclty_of_hint unchecked_tparams env h in
  match get_node locl_ty with
  | Tclass (cls_id, _, _) ->
    let get_class_const =
      match Env.get_class env (snd cls_id) with
      | Decl_entry.Found cls -> begin
        fun const_sid ->
          match Env.get_typeconst env cls (snd const_sid) with
          | Some ttc -> `Found ttc
          | None -> `Missing
      end
      | _ -> (fun _ -> `Skip)
    in
    let check_ref r =
      let (const_sid, is_ctx) =
        match r with
        | Rctx (sid, _) -> (sid, true)
        | Rtype (sid, _) -> (sid, false)
      in
      let on_error =
        Typing_error.Reasons_callback.invalid_class_refinement (fst const_sid)
      in
      match get_class_const const_sid with
      | `Skip -> None
      | `Found ttc when Bool.(ttc.ttc_is_ctx = is_ctx) -> None
      | `Missing ->
        Some
          Typing_error.(
            apply_reasons ~on_error
            @@ Secondary.Missing_class_constant
                 {
                   pos = fst cls_id;
                   class_name = snd cls_id;
                   const_name = snd const_sid;
                 })
      | `Found ttc ->
        let kind_str is_ctx =
          if is_ctx then
            "ctx"
          else
            "type"
        in
        let correct_kind = kind_str ttc.ttc_is_ctx in
        let wrong_kind = kind_str is_ctx in
        Some
          Typing_error.(
            apply_reasons ~on_error
            @@ Secondary.Invalid_refined_const_kind
                 {
                   pos = fst cls_id;
                   class_name = snd cls_id;
                   const_name = snd const_sid;
                   correct_kind;
                   wrong_kind;
                 })
    in
    List.iter rl ~f:(fun r ->
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) (check_ref r))
  | _ -> ()

(** Mostly check constraints on type parameters. *)
let check_happly unchecked_tparams env h =
  let (env, hint_pos, locl_ty) = loclty_of_hint unchecked_tparams env h in
  let (env, ty_err_opt) =
    match get_node locl_ty with
    | Tnewtype (name, targs, _)
    | Tclass ((_, name), _, targs) ->
      let tparams = Env.get_class_or_typedef_tparams env name in
      check_tparams_constraints env hint_pos tparams targs
    | _ -> (env, None)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  env

let check_splat_hint env p h =
  (* It's important that we pass tenv down to subtyping
   * because tpenv might have been updated during localization *)
  let (tenv, hint_pos, locl_ty) =
    loclty_of_hint env.typedef_tparams env.tenv h
  in
  (* The top tuple (mixed...) *)
  let cstr_ty =
    mk
      ( Reason.witness p,
        Ttuple
          {
            t_required = [];
            t_extra =
              Textra
                {
                  t_optional = [];
                  t_variadic = Typing_make_type.mixed (Reason.witness p);
                };
          } )
  in
  let (_env, err) =
    Typing_generic_constraint.check_tparams_constraint
      tenv
      ~use_pos:hint_pos
      Ast_defs.Constraint_as
      ~cstr_ty
      locl_ty
  in
  Option.to_list err

let rec context_hint ?(in_signature = true) env (p, h) =
  Typing_type_integrity.check_context_hint_integrity
    ~in_signature
    env.tenv
    (p, h);
  hint_ ~in_signature env p h

and hint_ ~in_signature env p h_ =
  let hint env (p, h) = hint_ ~in_signature env p h in
  let hint_opt env = Option.value_map ~f:(hint env) ~default:[] in
  let hints env xs = List.concat_map xs ~f:(hint env) in
  match h_ with
  | Hmixed
  | Hwildcard
  | Hnonnull
  | Hprim _
  | Hthis
  | Haccess _
  | Habstr _
  | Hdynamic
  | Hnothing ->
    []
  | Hoption (_, Hprim Tnull) ->
    Lint.option_null p;
    []
  | Hoption (_, Hprim Tvoid) ->
    [
      Typing_error.(
        primary @@ Primary.Option_return_only_typehint { pos = p; kind = `void });
    ]
  | Hoption (_, Hprim Tnoreturn) ->
    [
      Typing_error.(
        primary
        @@ Primary.Option_return_only_typehint { pos = p; kind = `noreturn });
    ]
  | Hoption (_, Hmixed) ->
    Lint.option_mixed p;
    []
  | Hvec_or_dict (ty1, ty2) -> hint_opt env ty1 @ hint env ty2
  | Hunion hl
  | Hintersection hl ->
    hints env hl
  | Htuple { tup_required; tup_extra } ->
    hints env tup_required
    @ begin
        match tup_extra with
        | Hextra { tup_optional; tup_variadic } ->
          hints env tup_optional @ hint_opt env tup_variadic
        | Hsplat h -> hint env h @ check_splat_hint env p h
      end
  | Hclass_ptr (_, h)
  | Hoption h
  | Hsoft h
  | Hlike h ->
    hint env h
  | Hfun
      {
        hf_is_readonly = _;
        hf_tparams;
        hf_param_tys = hl;
        hf_param_info;
        hf_variadic_ty = variadic_hint;
        hf_ctxs;
        hf_return_ty = h;
        hf_is_readonly_return = _;
      } ->
    let splat_err =
      match (List.rev hl, List.rev hf_param_info) with
      | (h :: _, Some { hfparam_splat = Some Ast_defs.Splat; _ } :: _) ->
        check_splat_hint env p h
      | _ -> []
    in
    (* Bind any type parameters bound in the function hint and check the hint *)
    let env =
      let tparams = List.map hf_tparams ~f:Aast_defs.tparam_of_hint_tparam in
      { env with typedef_tparams = tparams @ env.typedef_tparams }
    in
    hints env hl
    @ splat_err
    @ hint env h
    @ hint_opt env variadic_hint
    @ contexts_opt env hf_ctxs
    @ List.concat_map hf_tparams ~f:(fun Aast_defs.{ htp_constraints; _ } ->
          List.concat_map htp_constraints ~f:(fun (_, h) -> hint env h))
  | Happly ((p, "\\Tuple"), _)
  | Happly ((p, "\\tuple"), _) ->
    [Typing_error.(wellformedness @@ Primary.Wellformedness.Tuple_syntax p)]
  | Happly (_, hl) as h ->
    let _tenv = check_happly env.typedef_tparams env.tenv (p, h) in
    hints env hl
  | Hrefinement (hr, rl) as h ->
    check_hrefinement env.typedef_tparams env.tenv (p, h) rl;
    let refinement_hints =
      List.fold rl ~init:[] ~f:(fun rl r ->
          match r with
          | Rctx (_, CRexact h) -> h :: rl
          | Rctx (_, CRloose { cr_lower = hl; cr_upper = hr }) ->
            Option.to_list hl @ Option.to_list hr @ rl
          | Rtype (_, TRexact h) -> h :: rl
          | Rtype (_, TRloose { tr_lower = hl; tr_upper = hr }) -> hl @ hr @ rl)
    in
    hints env (hr :: refinement_hints)
  | Hshape { nsi_allows_unknown_fields = _; nsi_field_map } ->
    let (tenv, errors) =
      let get_name sfi = sfi.sfi_name in
      Typing_shapes.check_shape_keys_validity
        env.tenv
        (List.map ~f:get_name nsi_field_map)
    in
    let env = { env with tenv } in
    let rec_errors =
      let compute_hint_for_shape_field_info { sfi_hint; _ } =
        hint env sfi_hint
      in
      List.concat_map ~f:compute_hint_for_shape_field_info nsi_field_map
    in
    errors @ rec_errors
  | Hfun_context _ ->
    (* TODO(coeffects): check if arg is a function type in the locals? *)
    []
  | Hvar _ ->
    (* TODO(coeffects) *)
    []

and contexts env (_, hl) = List.concat_map ~f:(context_hint env) hl

and contexts_opt env = Option.value_map ~default:[] ~f:(contexts env)

let hint
    ?(in_signature = true)
    ?(should_check_package_boundary = `Yes "symbol")
    env
    (p, h) =
  (* Do not use this one recursively to avoid quadratic runtime! *)
  Typing_type_integrity.check_hint_integrity
    ~in_signature
    ~should_check_package_boundary
    env.tenv
    (p, h);
  hint_ ~in_signature env p h

let hint_opt ?in_signature ?should_check_package_boundary env =
  Option.value_map
    ~default:[]
    ~f:(hint ?in_signature ?should_check_package_boundary env)

let hints ?in_signature env = List.concat_map ~f:(hint ?in_signature env)

let type_hint env th =
  Option.value_map
    ~default:[]
    ~f:(hint ~should_check_package_boundary:`No env)
    (hint_of_type_hint th)

let fun_param env param = type_hint env param.param_type_hint

let fun_params env = List.concat_map ~f:(fun_param env)

let tparam env t =
  List.concat_map t.Aast.tp_constraints ~f:(fun (_, h) ->
      (* ignore package checks on tparam constraints *)
      hint ~should_check_package_boundary:`No env h)

let tparams env = List.concat_map ~f:(tparam env)

let where_constr env (h1, _, h2) =
  hint env h1 @ hint ~should_check_package_boundary:`No env h2

let where_constrs env = List.concat_map ~f:(where_constr env)

let requirements env = List.concat_map ~f:(fun (h, _kind) -> hint env h)

let fun_ tenv f =
  let support_dynamic_type =
    Naming_attributes.mem
      Naming_special_names.UserAttributes.uaSupportDynamicType
      f.f_user_attributes
    || Env.get_support_dynamic_type tenv
  in
  let no_auto_likes =
    Naming_attributes.mem
      Naming_special_names.UserAttributes.uaNoAutoLikes
      f.f_user_attributes
    || Env.get_no_auto_likes tenv
  in
  let add_implicit_upper_bound =
    support_dynamic_type
    && (not no_auto_likes)
    && TypecheckerOptions.everything_sdt tenv.genv.tcopt
  in

  let (tenv, ty_err_opt) =
    let tparams =
      List.map f.f_tparams ~f:(fun ht_param ->
          (* We have to add the implicit upper bound here *)
          let tparam = Aast_defs.tparam_of_hint_tparam ht_param in
          if
            List.exists
              tparam.tp_user_attributes
              ~f:(fun { ua_name = (_, name); _ } ->
                String.equal
                  name
                  Naming_special_names.UserAttributes.uaNoAutoBound)
            || not add_implicit_upper_bound
          then
            tparam
          else
            let Aast_defs.{ tp_name = (pos, _); tp_constraints; _ } = tparam in
            let hint =
              ( pos,
                Aast_defs.Happly
                  ( (pos, Naming_special_names.Classes.cSupportDyn),
                    [(pos, Hmixed)] ) )
            in
            let tp_constraints =
              (Ast_defs.Constraint_as, hint) :: tp_constraints
            in
            { tparam with tp_constraints })
    in
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      tenv
      ~ignore_errors:true
      tparams
      []
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env:tenv) ty_err_opt;
  let env = { typedef_tparams = []; tenv } in
  let errs =
    FunUtils.check_params ~from_abstract_method:false tenv.decl_env f.f_params
  in
  let splat_err =
    match List.rev f.f_params with
    | {
        param_splat = Some Ast_defs.Splat;
        param_pos;
        param_type_hint = (_, Some h);
        _;
      }
      :: _ ->
      check_splat_hint env param_pos h
    | _ -> []
  in
  List.iter ~f:(Typing_error_utils.add_typing_error ~env:tenv) (splat_err @ errs);
  type_hint env f.f_ret @ fun_params env f.f_params

let fun_def tenv fd =
  (* Add type parameters to typing environment and localize the bounds
     and where constraints *)
  let (tenv, ty_err_opt) =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      tenv
      ~ignore_errors:true
      fd.fd_tparams
      fd.fd_where_constraints
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env:tenv) ty_err_opt;
  let env = { typedef_tparams = []; tenv } in
  tparams env fd.fd_tparams
  @ fun_ tenv fd.fd_fun
  @ where_constrs env fd.fd_where_constraints

let enum_opt env =
  let f { e_base; e_constraint; _ } =
    hint ~should_check_package_boundary:`No env e_base
    @ hint_opt ~should_check_package_boundary:`No env e_constraint
  in
  Option.value_map ~default:[] ~f

let const env { Aast.cc_type; _ } =
  hint_opt ~should_check_package_boundary:`No env cc_type

let consts env cs = List.concat_map ~f:(const env) cs

let typeconsts env tcs cls_name =
  let get_class_const =
    match Env.get_class env.tenv (snd cls_name) with
    | Decl_entry.Found cls -> begin
      fun const_sid ->
        match Folded_class.get_typeconst cls (snd const_sid) with
        | Some ttc -> `Found ttc
        | None -> `Missing
    end
    | _ -> (fun _ -> `Skip)
  in
  let f tconst =
    let should_check_package_boundary =
      if not @@ Env.package_v2_allow_all_tconst_violations env.tenv then
        `Yes "type constant"
      else
        match get_class_const tconst.c_tconst_name with
        | `Found ttc when Option.is_none ttc.ttc_reifiable -> `No
        | `Found _ ->
          if Env.package_v2_allow_reifiable_tconst_violations env.tenv then
            `No
          else
            `Yes "reifiable type constant"
        | _ -> `No
    in
    match tconst.c_tconst_kind with
    | TCAbstract { c_atc_as_constraint; c_atc_super_constraint; c_atc_default }
      ->
      hint_opt ~should_check_package_boundary env c_atc_as_constraint
      @ hint_opt ~should_check_package_boundary env c_atc_super_constraint
      @ hint_opt ~should_check_package_boundary env c_atc_default
    | TCConcrete { c_tc_type } ->
      hint ~should_check_package_boundary env c_tc_type
  in
  List.concat_map ~f tcs

(* Treat private or internal class members as internal *)
let at_least_internal = function
  | Internal
  | Private ->
    true
  | Protected
  | ProtectedInternal
  | Public ->
    false

let class_vars env cvs =
  let f cv =
    let tenv = Env.set_internal env.tenv (at_least_internal cv.cv_visibility) in
    let env = { env with tenv } in
    type_hint env cv.cv_type
  in
  List.concat_map ~f cvs

let method_ env m =
  (* Add method type parameters to environment and localize the bounds
     and where constraints *)
  let (tenv, ty_err_opt) =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      env.tenv
      ~ignore_errors:true
      m.m_tparams
      m.m_where_constraints
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env:tenv) ty_err_opt;
  (* Only throw hack error if class and method are both public *)
  let tenv =
    Env.set_internal
      tenv
      (at_least_internal m.m_visibility || Env.get_internal tenv)
  in

  let env = { env with tenv } in
  fun_params env m.m_params
  @ tparams env m.m_tparams
  @ where_constrs env m.m_where_constraints
  @ type_hint env m.m_ret

let methods env = List.concat_map ~f:(method_ env)

let method_opt env = Option.value_map ~default:[] ~f:(method_ env)

let hint_no_kind_check env (p, h) = hint_ ~in_signature:true env p h

let class_ tenv c =
  let env = { typedef_tparams = []; tenv } in
  let {
    c_span = _;
    c_annotation = _;
    c_mode = _;
    c_final = _;
    c_is_xhp = _;
    c_has_xhp_keyword = _;
    c_kind;
    c_name;
    c_tparams;
    c_extends;
    c_uses;
    c_xhp_category = _;
    c_reqs;
    c_implements;
    c_consts;
    c_typeconsts;
    c_vars;
    c_methods;
    c_xhp_children = _;
    c_xhp_attrs = _;
    (* Collapsed into c_vars during Naming *)
    c_xhp_attr_uses = _;
    (* These represent `attribute :a;` declarations, and it is a parse error
     * to write `attribute :a<string>` *)
    c_namespace = _;
    c_user_attributes = _;
    c_file_attributes = _;
    c_enum;
    c_doc_comment = _;
    c_emit_id = _;
    c_internal = _;
    c_module = _;
    c_docs_url = _;
    c_package = _;
  } =
    c
  in

  (* Add type parameters to typing environment and localize the bounds *)
  let (tenv, ty_err_opt) =
    let req_class_constraints =
      List.filter_map c_reqs ~f:(fun req ->
          match req with
          | (t, RequireClass) ->
            let pos = fst t in
            Some ((pos, Hthis), Ast_defs.Constraint_eq, t)
          | _ -> None)
    in
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      tenv
      ~ignore_errors:true
      c_tparams
      req_class_constraints
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env:tenv) ty_err_opt;
  let env = { env with tenv } in
  let (c_constructor, c_statics, c_methods) = split_methods c_methods in
  let (c_static_vars, c_vars) = split_vars c_vars in
  List.concat
    [
      (if not Ast_defs.(is_c_interface c_kind) then
        method_opt env c_constructor
      else
        []);
      tparams env c_tparams;
      (* Extends clause must be checked, so that we reject public classes extending internal ones *)
      hints ~in_signature:true env c_extends;
      (* But for interfaces and trait use, we allow internal *)
      hints ~in_signature:false env c_implements;
      hints ~in_signature:false env c_uses;
      requirements env c_reqs;
      typeconsts env c_typeconsts c_name;
      class_vars env c_static_vars;
      class_vars env c_vars;
      consts env c_consts;
      methods env c_statics;
      methods env c_methods;
      enum_opt env c_enum;
    ]

let typedef tenv (t : (_, _) typedef) =
  let {
    t_tparams;
    t_annotation = _;
    t_name;
    t_as_constraint;
    t_super_constraint;
    t_assignment;
    t_runtime_type = _;
    t_mode = _;
    t_namespace = _;
    t_user_attributes = _;
    t_span = _;
    t_emit_id = _;
    t_is_ctx = _;
    t_file_attributes = _;
    t_internal = _;
    t_module = _;
    t_docs_url = _;
    t_doc_comment = _;
    t_package = _;
  } =
    t
  in
  let ( should_check_internal_signature,
        should_check_package_boundary,
        typedef_tparams,
        hint_constraints_pairs ) =
    match t_assignment with
    (* We only need to check that the type alias as a public API if it's transparent, since
       an opaque type alias is inherently internal *)
    (* Since type aliases cannot have constraints we shouldn't check
       if its type params satisfy the constraints of any tapply it
       references. *)
    | SimpleTypeDef { tvh_vis = Transparent; tvh_hint } ->
      let should_check_package_boundary =
        if Env.package_v2_allow_typedef_violations tenv then
          `No
        else
          `Yes "transparent type alias"
      in
      (true, should_check_package_boundary, t_tparams, [(tvh_hint, [])])
    | SimpleTypeDef { tvh_vis = _; tvh_hint } ->
      (false, `No, [], [(tvh_hint, [])])
    | CaseType (variant, variants) ->
      let variants = variant :: variants in
      let hint_constraints_pairs =
        List.map variants ~f:(fun v -> (v.tctv_hint, v.tctv_where_constraints))
      in
      (false, `Yes "case type", [], hint_constraints_pairs)
  in
  (* We don't allow constraints on typdef parameters, but we still
     need to record their kinds in the generic var environment *)
  let tenv_with_typedef_tparams =
    let (env, ty_err_opt) =
      let where_constraints = [] in
      Phase.localize_and_add_ast_generic_parameters_and_where_constraints
        tenv
        ~ignore_errors:true
        t_tparams
        where_constraints
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    env
  in

  (* For typdefs, we do want to do the simple kind checks on the body
     (e.g., arities match up), but no constraint checks. We need to check the
     kinds of typedefs separately, because check_happly replaces all the generic
     parameters of typedefs by Tany, which makes the kind check moot *)
  maybe
    (* We always check the constraints for internal types, so treat in_signature:true *)
    (Typing_type_integrity.check_hint_integrity
       ~in_signature:true
       ~should_check_package_boundary)
    tenv_with_typedef_tparams
    t_as_constraint;
  maybe
    (Typing_type_integrity.check_hint_integrity
       ~in_signature:true
       ~should_check_package_boundary)
    tenv_with_typedef_tparams
    t_super_constraint;
  (* TODO check kinded-ness for constraints too *)
  List.iter hint_constraints_pairs ~f:(fun (hint, _constraints) ->
      Typing_type_integrity.check_hint_integrity
        ~in_signature:should_check_internal_signature
        ~should_check_package_boundary
        tenv_with_typedef_tparams
        hint);
  let env = { typedef_tparams; tenv } in
  (* We checked the kinds already above.  *)
  Option.value_map ~default:[] ~f:(hint_no_kind_check env) t.t_as_constraint
  @ Option.value_map
      ~default:[]
      ~f:(hint_no_kind_check env)
      t.t_super_constraint
  @ List.concat_map hint_constraints_pairs ~f:(fun (hint, constraints) ->
        let (tenv, _err) =
          Phase.localize_and_add_where_constraints
            tenv
            ~ignore_errors:true
            (Typing_case_types.filter_where_clauses_with_recursive_mentions
               tenv
               t_name
               constraints)
        in
        let env = { typedef_tparams; tenv } in
        hint_no_kind_check env hint @ where_constrs env constraints)

let global_constant tenv gconst =
  let env = { typedef_tparams = []; tenv } in
  let {
    cst_annotation = _;
    cst_mode = _;
    cst_name = _;
    cst_type;
    cst_value = _;
    cst_namespace = _;
    cst_span = _;
    cst_emit_id = _;
    cst_module = _;
  } =
    gconst
  in
  hint_opt ~should_check_package_boundary:`No env cst_type

let hint ?(should_check_package_boundary = `Yes "symbol") tenv h =
  let env = { typedef_tparams = []; tenv } in
  hint ~in_signature:false ~should_check_package_boundary env h

(** Check well-formedness of type hints. See .mli file for more. *)
let expr tenv ((), _p, e) =
  (* We don't recurse on expressions here because this is called by Typing.expr *)
  match e with
  | Is (_, h)
  | As { expr = _; hint = h; is_nullable = _; enforce_deep = _ } ->
    hint tenv h ~should_check_package_boundary:`No
  | Upcast (_, h)
  | Cast (h, _) ->
    hint tenv h
  | New (_, hl, _, _, _)
  | Call { targs = hl; _ } ->
    List.concat_map hl ~f:(fun (_, h) ->
        (* ignore package checks on targs *)
        hint ~should_check_package_boundary:`No tenv h)
  | Lfun (f, _)
  | Efun { ef_fun = f; _ } ->
    fun_ tenv f
  | ExpressionTree _
  | Invalid _
  | Shape _
  | ValCollection _
  | KeyValCollection _
  | Null
  | This
  | True
  | False
  | Omitted
  | Id _
  | Lvar _
  | Dollardollar _
  | Clone _
  | Array_get _
  | Obj_get _
  | Class_get _
  | Class_const _
  | FunctionPointer _
  | Int _
  | Float _
  | String _
  | String2 _
  | PrefixedString _
  | Yield _
  | Await _
  | ReadonlyExpr _
  | Tuple _
  | List _
  | Unop _
  | Binop _
  | Assign _
  | Pipe _
  | Eif _
  | Xml _
  | Import _
  | Collection _
  | Lplaceholder _
  | Method_caller _
  | Pair _
  | EnumClassLabel _
  | ET_Splice _
  | Hole _
  | Package _
  | Nameof _ ->
    []

(** Check well-formedness of type hints. See .mli file for more. *)
let _toplevel_def tenv = function
  (* This function is not used but ensures we don't forget to
   * extend this module for future top-level definitions we may add. *)
  | Fun f ->
    let {
      fd_namespace = _;
      fd_mode = _;
      fd_file_attributes = _;
      fd_fun;
      fd_name = _;
      fd_internal = _;
      fd_module = _;
      fd_tparams = _;
      fd_where_constraints = _;
      fd_package = _;
    } =
      f
    in

    fun_ tenv fd_fun
  | Constant gc -> global_constant tenv gc
  | Typedef td -> typedef tenv td
  | Class c -> class_ tenv c
  | Stmt _
  | Namespace _
  | NamespaceUse _
  | SetNamespaceEnv _
  | FileAttributes _
  | SetModule _
  | Module _ ->
    []
