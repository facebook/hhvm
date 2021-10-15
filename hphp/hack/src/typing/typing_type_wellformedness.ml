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
module SN = Naming_special_names
module Subst = Decl_subst
module TUtils = Typing_utils
module Cls = Decl_provider.Class

(** This module checks well-formedness of type hints. See .mli file for more. *)

type env = {
  typedef_tparams: Nast.tparam list;
  tenv: Typing_env_types.env;
}

(* Check if the __ViaLabel attributes is on the parameter. If that's the case,
 * we check that it is only involving an enum class or a generic
 *)
let check_via_label_on_param env pos dty lty =
  (* If lty is HH\MemberOf<Foo, Bar>, we need to check that Foo is
   * - an enum class
   * - a generic
   *
   * If it is a generic, we need to check that it is
   * - a reified generic
   * - a type constant
   *
   * in both cases, it must be bounded by an enum class
   *
   * In all cases, we check that the requested label is part of the
   * detected enum class.
   *)
  let check_tgeneric name =
    let is_taccess_this =
      match get_node dty with
      | Tapply ((_, _name), [ty_enum; _ty_interface]) ->
        (match get_node ty_enum with
        | Taccess (dty, _) ->
          (match get_node dty with
          | Tthis -> true
          | _ -> false)
        | _ -> false)
      | _ -> assert false
      (* we just checked that *)
    in
    let is_reified =
      match Env.get_reified env name with
      | Erased -> false
      | Reified
      | SoftReified ->
        true
    in
    if is_taccess_this || is_reified then
      ()
    else
      Errors.via_label_invalid_generic pos name
  in
  match get_node lty with
  | Tnewtype (name, [ty_enum; _ty_interface], _)
    when String.equal name SN.Classes.cMemberOf ->
    (match get_node ty_enum with
    | Tclass ((_, enum_name), _, _) when Env.is_enum_class env enum_name -> env
    | Tgeneric (name, _) ->
      let () = check_tgeneric name in
      let (env, upper_bounds) =
        Typing_utils.collect_enum_class_upper_bounds env name
      in
      let () =
        match upper_bounds with
        | Some upper_bound ->
          if Typing_utils.is_tintersection env upper_bound then
            Errors.via_label_invalid_parameter_in_enum_class pos
        | None -> Errors.via_label_invalid_parameter_in_enum_class pos
      in
      env
    | _ ->
      let () = Errors.via_label_invalid_parameter_in_enum_class pos in
      env)
  | _ ->
    let () = Errors.via_label_invalid_parameter pos in
    env

let check_tparams_constraints env use_pos tparams targs =
  let ety_env : expand_env =
    { empty_expand_env with substs = Subst.make_locl tparams targs }
  in
  Typing_phase.check_tparams_constraints ~use_pos ~ety_env env tparams

(** Mostly check constraints on type parameters. *)
let check_happly ?(via_label = false) unchecked_tparams env h =
  let hint_pos = fst h in
  let decl_ty = Decl_hint.hint env.decl_env h in
  let unchecked_tparams =
    List.map
      unchecked_tparams
      ~f:(Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
  in
  let tyl =
    List.map unchecked_tparams ~f:(fun t ->
        mk (Reason.Rwitness_from_decl (fst t.tp_name), Typing_defs.make_tany ()))
  in
  let subst = Inst.make_subst unchecked_tparams tyl in
  let decl_ty = Inst.instantiate subst decl_ty in
  let ety_env = { empty_expand_env with expand_visible_newtype = false } in
  let (env, locl_ty) = Phase.localize env ~ety_env decl_ty in
  let env =
    if via_label then
      check_via_label_on_param env hint_pos decl_ty locl_ty
    else
      env
  in
  match get_node locl_ty with
  | Tnewtype (type_name, targs, _cstr_ty) ->
    (match Env.get_typedef env type_name with
    | None -> env
    | Some typedef ->
      check_tparams_constraints env hint_pos typedef.td_tparams targs)
  | _ ->
    (match get_node (TUtils.get_base_type env locl_ty) with
    | Tclass (cls, _, targs) ->
      (match Env.get_class env (snd cls) with
      | Some cls ->
        check_tparams_constraints env hint_pos (Cls.tparams cls) targs
      | None -> env)
    | _ -> env)

let rec hint ?(via_label = false) ?(in_signature = true) env (p, h) =
  (* Do not use this one recursively to avoid quadratic runtime! *)
  Typing_kinding.Simple.check_well_kinded_hint ~in_signature env.tenv (p, h);
  hint_ ~via_label ~in_signature env p h

and context_hint ?(via_label = false) ?(in_signature = true) env (p, h) =
  Typing_kinding.Simple.check_well_kinded_context_hint
    ~in_signature
    env.tenv
    (p, h);
  hint_ ~via_label ~in_signature env p h

and hint_ ~via_label ~in_signature env p h_ =
  let hint env (p, h) = hint_ ~via_label:false ~in_signature env p h in
  let () =
    if via_label then
      (* __ViaLabel is only allowed on HH\MemberOf, so we check everything that
       * is not a class with this, and make a more refined check for Happly
       *)
      match h_ with
      | Happly _ -> () (* checked in check_happly *)
      | _ -> Errors.via_label_invalid_parameter p
  in
  match h_ with
  | Hany
  | Herr
  | Hmixed
  | Hnonnull
  | Hprim _
  | Hthis
  | Haccess _
  | Habstr _
  | Hdynamic
  | Hsupportdynamic
  | Hnothing ->
    ()
  | Hoption (_, Hprim Tnull) -> Errors.option_null p
  | Hoption (_, Hprim Tvoid) -> Errors.option_return_only_typehint p `void
  | Hoption (_, Hprim Tnoreturn) ->
    Errors.option_return_only_typehint p `noreturn
  | Hoption (_, Hmixed) -> Errors.option_mixed p
  | Hdarray (ty1, ty2) ->
    hint env ty1;
    hint env ty2
  | Hvec_or_dict (ty1, ty2)
  | Hvarray_or_darray (ty1, ty2) ->
    maybe hint env ty1;
    hint env ty2
  | Hvarray ty -> hint env ty
  | Htuple hl
  | Hunion hl
  | Hintersection hl ->
    List.iter hl ~f:(hint env)
  | Hoption h
  | Hsoft h
  | Hlike h ->
    hint env h
  | Hfun
      {
        hf_is_readonly = _;
        hf_param_tys = hl;
        hf_param_info = _;
        hf_variadic_ty = variadic_hint;
        hf_ctxs;
        hf_return_ty = h;
        hf_is_readonly_return = _;
      } ->
    List.iter hl ~f:(hint env);
    hint env h;
    Option.iter variadic_hint ~f:(hint env);
    Option.iter ~f:(contexts env) hf_ctxs
  | Happly ((p, "\\Tuple"), _)
  | Happly ((p, "\\tuple"), _) ->
    Errors.tuple_syntax p
  | Happly ((_, x), hl) as h ->
    begin
      match Env.get_class_or_typedef env.tenv x with
      | None -> ()
      | Some (Env.TypedefResult _) ->
        let (_ : Typing_env_types.env) =
          check_happly ~via_label env.typedef_tparams env.tenv (p, h)
        in
        List.iter hl ~f:(hint env)
      | Some (Env.ClassResult _) ->
        let (_ : Typing_env_types.env) =
          check_happly ~via_label env.typedef_tparams env.tenv (p, h)
        in
        List.iter hl ~f:(hint env)
    end
  | Hshape { nsi_allows_unknown_fields = _; nsi_field_map } ->
    let compute_hint_for_shape_field_info { sfi_hint; _ } = hint env sfi_hint in
    List.iter ~f:compute_hint_for_shape_field_info nsi_field_map
  | Hfun_context _ ->
    (* TODO(coeffects): check if arg is a function type in the locals? *)
    ()
  | Hvar _ ->
    (* TODO(coeffects) *)
    ()

and contexts env (_, hl) = List.iter ~f:(context_hint env) hl

let type_hint ?via_label env th =
  maybe (hint ?via_label) env (hint_of_type_hint th)

let fun_param env param =
  let via_label =
    Naming_attributes.mem
      SN.UserAttributes.uaViaLabel
      param.param_user_attributes
  in
  type_hint ~via_label env param.param_type_hint

let variadic_param env vparam =
  match vparam with
  | FVvariadicArg p -> fun_param env p
  | _ -> ()

let tparam env t = List.iter t.Aast.tp_constraints ~f:(fun (_, h) -> hint env h)

let where_constr env (h1, _, h2) =
  hint env h1;
  hint env h2

let fun_ tenv f =
  FunUtils.check_params f.f_params;
  let env = { typedef_tparams = []; tenv } in
  (* Add type parameters to typing environment and localize the bounds
     and where constraints *)
  let tenv =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      env.tenv
      ~ignore_errors:true
      f.f_tparams
      f.f_where_constraints
  in
  let env = { env with tenv } in
  type_hint env f.f_ret;

  List.iter f.f_tparams ~f:(tparam env);
  List.iter f.f_params ~f:(fun_param env);
  variadic_param env f.f_variadic

let enum env e =
  hint env e.e_base;
  maybe hint env e.e_constraint

let const env class_const = maybe hint env class_const.Aast.cc_type

let typeconst (env, _) tconst =
  match tconst.c_tconst_kind with
  | TCAbstract { c_atc_as_constraint; c_atc_super_constraint; c_atc_default } ->
    maybe hint env c_atc_as_constraint;
    maybe hint env c_atc_super_constraint;
    maybe hint env c_atc_default
  | TCConcrete { c_tc_type } -> hint env c_tc_type

let class_var env cv =
  let tenv =
    Env.set_internal
      env.tenv
      (Naming_attributes.mem SN.UserAttributes.uaInternal cv.cv_user_attributes)
  in
  let env = { env with tenv } in
  type_hint env cv.cv_type

let method_ env m =
  (* Add method type parameters to environment and localize the bounds
     and where constraints *)
  let tenv =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      env.tenv
      ~ignore_errors:true
      m.m_tparams
      m.m_where_constraints
  in
  let tenv =
    Env.set_internal
      tenv
      (Naming_attributes.mem SN.UserAttributes.uaInternal m.m_user_attributes)
  in
  let env = { env with tenv } in
  List.iter m.m_params ~f:(fun_param env);
  variadic_param env m.m_variadic;
  List.iter m.m_tparams ~f:(tparam env);
  List.iter m.m_where_constraints ~f:(where_constr env);
  type_hint env m.m_ret

let hint_no_kind_check env (p, h) =
  hint_ ~via_label:false ~in_signature:true env p h

let class_attr env = function
  | CA_name _ -> ()
  | CA_field { ca_type; ca_id = _; ca_value = _; ca_required = _ } ->
    (match ca_type with
    | CA_hint h -> hint env h
    | CA_enum _ -> ())

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
    c_name = _;
    c_tparams;
    c_extends;
    c_uses;
    c_use_as_alias = _;
    c_insteadof_alias = _;
    c_xhp_category = _;
    (* TODO: c_reqs should be checked too. Problem: that causes errors in un-fixmeable places. *)
    c_reqs = _;
    c_implements;
    c_where_constraints;
    c_consts;
    c_typeconsts;
    c_vars;
    c_methods;
    c_attributes;
    c_xhp_children = _;
    (* c_xhp_attrs and c_xhp_attr_uses should probably be checked too, but
     * they have weird generics rules, e.g. not providing type arguments seems allowed. *)
    c_xhp_attrs = _;
    c_xhp_attr_uses = _;
    c_namespace = _;
    c_user_attributes = _;
    c_file_attributes = _;
    c_enum;
    c_doc_comment = _;
    c_emit_id = _;
  } =
    c
  in
  (* Add type parameters to typing environment and localize the bounds *)
  let tenv =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      tenv
      ~ignore_errors:true
      c_tparams
      c_where_constraints
  in
  let env = { env with tenv } in
  let (c_constructor, c_statics, c_methods) = split_methods c_methods in
  let (c_static_vars, c_vars) = split_vars c_vars in
  if not Ast_defs.(is_c_interface c_kind) then maybe method_ env c_constructor;
  List.iter c_tparams ~f:(tparam env);
  List.iter c_where_constraints ~f:(where_constr env);
  List.iter c_extends ~f:(hint env);
  List.iter c_implements ~f:(hint env);
  (* Use is not a signature from the point of view of modules because the trait
     being use'd does not need to be seen by users of the class *)
  List.iter c_uses ~f:(hint env ~in_signature:false);
  List.iter c_typeconsts ~f:(typeconst (env, c_tparams));
  List.iter c_static_vars ~f:(class_var env);
  List.iter c_vars ~f:(class_var env);
  List.iter c_consts ~f:(const env);
  List.iter c_statics ~f:(method_ env);
  List.iter c_methods ~f:(method_ env);
  List.iter c_attributes ~f:(class_attr env);
  maybe enum env c_enum;
  ()

let typedef tenv t =
  let {
    t_tparams;
    t_annotation = _;
    t_name = _;
    t_constraint;
    t_kind;
    t_mode = _;
    t_vis;
    t_namespace = _;
    t_user_attributes = _;
    t_span = _;
    t_emit_id = _;
    t_is_ctx = _;
  } =
    t
  in
  (* We don't allow constraints on typdef parameters, but we still
     need to record their kinds in the generic var environment *)
  let where_constraints = [] in
  let tenv_with_typedef_tparams =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      tenv
      ~ignore_errors:true
      t_tparams
      where_constraints
  in
  (* For typdefs, we do want to do the simple kind checks on the body
     (e.g., arities match up), but no constraint checks. We need to check the
     kinds of typedefs separately, because check_happly replaces all the generic
     parameters of typedefs by Tany, which makes the kind check moot *)
  maybe
    (Typing_kinding.Simple.check_well_kinded_hint ~in_signature:true)
    tenv_with_typedef_tparams
    t_constraint;
  Typing_kinding.Simple.check_well_kinded_hint
    ~in_signature:true
    tenv_with_typedef_tparams
    t_kind;
  let env =
    {
      typedef_tparams =
        (match t_vis with
        | Transparent
        | Tinternal ->
          (* Since type aliases cannot have constraints we shouldn't check
           * if its type params satisfy the constraints of any tapply it
           * references.
           *)
          t.t_tparams
        | Opaque -> []);
      tenv;
    }
  in
  (* We checked the kinds already above.  *)
  maybe hint_no_kind_check env t.t_constraint;
  hint_no_kind_check env t_kind

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
  } =
    gconst
  in
  maybe hint env cst_type;
  ()

let record_def tenv record =
  let env = { typedef_tparams = []; tenv } in
  let {
    rd_annotation = _;
    rd_name = _;
    rd_extends;
    rd_abstract = _;
    rd_fields;
    rd_user_attributes = _;
    rd_namespace = _;
    rd_span = _;
    rd_doc_comment = _;
    rd_emit_id = _;
  } =
    record
  in
  maybe hint env rd_extends;
  List.iter rd_fields ~f:(fun (_id, h, _expr) -> hint env h);
  ()

let hint tenv h =
  let env = { typedef_tparams = []; tenv } in
  hint ~via_label:false ~in_signature:false env h

(** Check well-formedness of type hints. See .mli file for more. *)
let expr : Typing_env_types.env -> Nast.expr -> unit =
 fun tenv ((), _p, e) ->
  (* We don't recurse on expressions here because this is called by Typing.expr *)
  match e with
  | ExpressionTree
      {
        et_hint = h;
        et_splices = _;
        et_function_pointers = _;
        et_virtualized_expr = _;
        et_runtime_expr = _;
        et_dollardollar_pos = _;
      }
  | Is (_, h)
  | As (_, h, _)
  | Upcast (_, h)
  | Cast (h, _) ->
    hint tenv h
  | Lfun (f, _)
  | Efun (f, _) ->
    fun_ tenv f
  | Darray _
  | Varray _
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
  | Call _
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
  | Pipe _
  | Eif _
  | New _
  | Record _
  | Xml _
  | Import _
  | Collection _
  | Lplaceholder _
  | Fun_id _
  | Method_id _
  | Method_caller _
  | Smethod_id _
  | Pair _
  | EnumClassLabel _
  | ET_Splice _
  | Hole _ ->
    ()

(** Check well-formedness of type hints. See .mli file for more. *)
let _toplevel_def tenv = function
  (* This function is not used but ensures we don't forget to
   * extend this module for future top-level definitions we may add. *)
  | Fun f ->
    let { fd_namespace = _; fd_mode = _; fd_file_attributes = _; fd_fun } = f in
    fun_ tenv fd_fun
  | Constant gc -> global_constant tenv gc
  | Typedef td -> typedef tenv td
  | Class c -> class_ tenv c
  | RecordDef rd -> record_def tenv rd
  | Stmt _
  | Namespace _
  | NamespaceUse _
  | SetNamespaceEnv _
  | FileAttributes _ ->
    ()
