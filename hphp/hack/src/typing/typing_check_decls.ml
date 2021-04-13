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
module TGenConstraint = Typing_generic_constraint
module Subst = Decl_subst
module TUtils = Typing_utils
module Cls = Decl_provider.Class

type env = {
  typedef_tparams: Nast.tparam list;
  tenv: Typing_env_types.env;
}

let check_hint_wellkindedness env hint =
  let decl_ty = Decl_hint.hint env.decl_env hint in
  Typing_kinding.Simple.check_well_kinded_type env decl_ty

(* Check if the __Atom attributes is on the parameter. If that's the case,
 * we check that it is only involving an enum class or a generic
 *)
let check_atom_on_param env pos dty lty =
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
   * In all cases, we check that the requested atom is part of the
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
      Errors.atom_invalid_generic pos name
  in
  match get_node lty with
  (* Uncomment the next line to allow normal enums with __Atom *)
  (* | Tnewtype (enum_name, _, _) when Env.is_enum env enum_name -> () *)
  | Tnewtype (name, [ty_enum; _ty_interface], _)
    when String.equal name SN.Classes.cMemberOf ->
    (match get_node ty_enum with
    | Tclass ((_, enum_name), _, _) when Env.is_enum_class env enum_name -> ()
    | Tgeneric (name, _) ->
      let () = check_tgeneric name in
      let upper_bounds =
        Typing_utils.collect_enum_class_upper_bounds env name
      in
      if SSet.cardinal upper_bounds <> 1 then
        Errors.atom_invalid_parameter_in_enum_class pos
    | _ -> Errors.atom_invalid_parameter_in_enum_class pos)
  | _ -> Errors.atom_invalid_parameter pos

(** Mostly check constraints on type parameters. *)
let check_happly ?(is_atom = false) unchecked_tparams env h =
  let pos = fst h in
  let decl_ty = Decl_hint.hint env.decl_env h in
  let unchecked_tparams =
    List.map
      unchecked_tparams
      (Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
  in
  let tyl =
    List.map unchecked_tparams (fun t ->
        mk (Reason.Rwitness_from_decl (fst t.tp_name), Typing_defs.make_tany ()))
  in
  let subst = Inst.make_subst unchecked_tparams tyl in
  let decl_ty = Inst.instantiate subst decl_ty in
  match get_node decl_ty with
  | Tapply _ ->
    let (env, locl_ty) =
      Phase.localize_with_self env ~ignore_errors:true decl_ty
    in
    let () = if is_atom then check_atom_on_param env pos decl_ty locl_ty in
    begin
      match get_node (TUtils.get_base_type env locl_ty) with
      | Tclass (cls, _, tyl) ->
        (match Env.get_class env (snd cls) with
        | Some cls ->
          let tc_tparams = Cls.tparams cls in
          (* We want to instantiate the class type parameters with the
           * type list of the class we are localizing. We do not want to
           * add any more constraints when we localize the constraints
           * stored in the class_type since it may lead to infinite
           * recursion
           *)
          let ety_env =
            {
              (Phase.env_with_self env ~on_error:Errors.ignore_error) with
              substs = Subst.make_locl tc_tparams tyl;
            }
          in
          iter2_shortest
            begin
              fun { tp_name = (p, x); tp_constraints = cstrl; _ } ty ->
              List.iter cstrl (fun (ck, cstr_ty) ->
                  let r = Reason.Rwitness_from_decl p in
                  let (env, cstr_ty) = Phase.localize ~ety_env env cstr_ty in
                  let (_ : Typing_env_types.env) =
                    TGenConstraint.check_constraint
                      env
                      ck
                      ty
                      ~cstr_ty
                      (fun ?code:_ reasons ->
                        Reason.explain_generic_constraint (fst h) r x reasons)
                  in
                  ())
            end
            tc_tparams
            tyl
        | _ -> ())
      | _ -> ()
    end
  | _ -> ()

let rec fun_ tenv f =
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
  maybe hint env (hint_of_type_hint f.f_ret);

  List.iter f.f_tparams (tparam env);
  List.iter f.f_params (fun_param env);
  variadic_param env f.f_variadic

and tparam env t = List.iter t.tp_constraints (fun (_, h) -> hint env h)

and where_constr env (h1, _, h2) =
  hint env h1;
  hint env h2

and contexts env (_, hl) = List.iter ~f:(hint env) hl

and hint ?(is_atom = false) env (p, h) =
  (* Do not use this one recursively to avoid quadratic runtime! *)
  check_hint_wellkindedness env.tenv (p, h);
  hint_ ~is_atom env p h

and hint_ ~is_atom env p h_ =
  let hint env (p, h) = hint_ ~is_atom:false env p h in
  let () =
    if is_atom then
      (* __Atom is only allowed on HH\MemberOf, so we check everything that
       * is not a class with this, and make a more refined check for Happly
       *)
      match h_ with
      | Happly _ -> () (* checked in check_happly *)
      | _ -> Errors.atom_invalid_parameter p
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
    List.iter hl (hint env)
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
    List.iter hl (hint env);
    hint env h;
    Option.iter variadic_hint (hint env);
    Option.iter ~f:(contexts env) hf_ctxs
  | Happly ((p, "\\Tuple"), _)
  | Happly ((p, "\\tuple"), _) ->
    Errors.tuple_syntax p
  | Happly ((_, x), hl) as h ->
    begin
      match Env.get_class_or_typedef env.tenv x with
      | None -> ()
      | Some (Env.TypedefResult _) ->
        check_happly ~is_atom env.typedef_tparams env.tenv (p, h);
        List.iter hl (hint env)
      | Some (Env.ClassResult _) ->
        check_happly ~is_atom env.typedef_tparams env.tenv (p, h);
        List.iter hl (hint env)
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

and fun_param env param =
  let is_atom =
    Naming_attributes.mem SN.UserAttributes.uaAtom param.param_user_attributes
  in
  maybe (hint ~is_atom) env (hint_of_type_hint param.param_type_hint)

and variadic_param env vparam =
  match vparam with
  | FVvariadicArg p -> fun_param env p
  | _ -> ()

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
  | TCPartiallyAbstract { c_patc_constraint; c_patc_type } ->
    hint env c_patc_constraint;
    hint env c_patc_type

let class_var env cv = maybe hint env (hint_of_type_hint cv.cv_type)

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
  let env = { env with tenv } in
  List.iter m.m_params (fun_param env);
  variadic_param env m.m_variadic;
  List.iter m.m_tparams (tparam env);
  List.iter m.m_where_constraints (where_constr env);
  maybe hint env (hint_of_type_hint m.m_ret)

let hint_no_kind_check env (p, h) = hint_ ~is_atom:false env p h

let class_ tenv c =
  let env = { typedef_tparams = []; tenv } in
  (* Add type parameters to typing environment and localize the bounds *)
  let tenv =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      tenv
      ~ignore_errors:true
      c.c_tparams
      c.c_where_constraints
  in
  let env = { env with tenv } in
  let (c_constructor, c_statics, c_methods) = split_methods c in
  let (c_static_vars, c_vars) = split_vars c in
  if not Ast_defs.(equal_class_kind c.c_kind Cinterface) then
    maybe method_ env c_constructor;
  List.iter c.c_tparams (tparam env);
  List.iter c.c_where_constraints (where_constr env);
  List.iter c.c_extends (hint env);
  List.iter c.c_implements (hint env);
  List.iter c.c_uses (hint env);
  List.iter c.c_typeconsts (typeconst (env, c.c_tparams));
  List.iter c_static_vars (class_var env);
  List.iter c_vars (class_var env);
  List.iter c.c_consts (const env);
  List.iter c_statics (method_ env);
  List.iter c_methods (method_ env);
  maybe enum env c.c_enum

let typedef tenv t =
  let {
    t_tparams;
    t_annotation = _;
    t_name = _;
    t_constraint;
    t_kind;
    t_mode = _;
    t_vis = _;
    t_namespace = _;
    t_user_attributes = _;
    t_span = _;
    t_emit_id = _;
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
  maybe check_hint_wellkindedness tenv_with_typedef_tparams t_constraint;
  check_hint_wellkindedness tenv_with_typedef_tparams t_kind;
  let env =
    {
      (* Since typedefs cannot have constraints we shouldn't check
       * if its type params satisfy the constraints of any tapply it
       * references.
       *)
      typedef_tparams = t.t_tparams;
      tenv;
    }
  in
  (* We checked the kinds already above.  *)
  maybe hint_no_kind_check env t.t_constraint;
  hint_no_kind_check env t_kind
