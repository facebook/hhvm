(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This module performs some "validity" checks on declared types in
 * function and member signatures, extends, implements, uses, etc.
 * - trivial syntactic errors (e.g. writing ?nonnull instead of mixed)
 * - unsatisfied constraints (e.g. C<bool> where C requires T as arraykey)
 *)

open Hh_prelude
open Aast
open Typing_defs
open Typing_env_types
open Utils
module Env = Typing_env
module Inst = Decl_instantiate
module Phase = Typing_phase
module SN = Naming_special_names
module TGenConstraint = Typing_generic_constraint
module Subst = Decl_subst
module TUtils = Typing_utils
module Cls = Decl_provider.Class
module Partial = Partial_provider

type env = {
  typedef_tparams: Nast.tparam list;
  tenv: Typing_env_types.env;
}

let get_ctx env = Env.get_ctx env.tenv

let check_hint_wellkindedness env hint =
  let decl_ty = Decl_hint.hint env.decl_env hint in
  Typing_kinding.Simple.check_well_kinded_type env decl_ty

let rec fun_ tenv f =
  let env = { typedef_tparams = []; tenv } in
  let (p, _) = f.f_name in
  (* Add type parameters to typing environment and localize the bounds
     and where constraints *)
  let tenv =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      p
      env.tenv
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

and hint env (p, h) =
  (* Do not use this one recursively to avoid quadratic runtime! *)
  check_hint_wellkindedness env.tenv (p, h);
  hint_ env p h

and hint_no_kind_check env (p, h) = hint_ env p h

and hint_ env p h_ =
  let hint env (p, h) = hint_ env p h in
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
        hf_reactive_kind = _;
        hf_param_tys = hl;
        hf_param_kinds = _;
        hf_param_mutability = _;
        hf_variadic_ty = variadic_hint;
        hf_cap = cap_opt;
        hf_return_ty = h;
        hf_is_mutable_return = _;
      } ->
    List.iter hl (hint env);
    hint env h;
    Option.iter variadic_hint (hint env);
    maybe hint env cap_opt
  | Happly ((p, "\\Tuple"), _)
  | Happly ((p, "\\tuple"), _) ->
    Errors.tuple_syntax p
  | Happly ((_, x), hl) as h when Env.is_typedef env.tenv x ->
    begin
      match Decl_provider.get_typedef (get_ctx env) x with
      | Some _ ->
        check_happly env.typedef_tparams env.tenv (p, h);
        List.iter hl (hint env)
      | None -> ()
    end
  | Happly ((_, x), hl) as h ->
    (match Env.get_class env.tenv x with
    | None -> ()
    | Some _ ->
      check_happly env.typedef_tparams env.tenv (p, h);
      List.iter hl (hint env));
    ()
  | Hshape { nsi_allows_unknown_fields = _; nsi_field_map } ->
    let compute_hint_for_shape_field_info { sfi_hint; _ } = hint env sfi_hint in
    List.iter ~f:compute_hint_for_shape_field_info nsi_field_map
  | Hpu_access (h, _) ->
    (* We explicitly forbid any syntax other than Foo:@Bar in here, since it
       brings more complexity to the type checker and do not allow anything
       interesting at the moment. If one need a variable ranging over PU
       member, one should use "C:@E" instead, and for a type parameter TP
       bound by a PU, projecting its type T must be TP:@T *)
    (match snd h with
    | Hpu_access _ -> Errors.pu_invalid_access p ""
    | _ -> hint env h)

and check_happly unchecked_tparams env h =
  let decl_ty = Decl_hint.hint env.decl_env h in
  let unchecked_tparams =
    List.map
      unchecked_tparams
      (Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
  in
  let tyl =
    List.map unchecked_tparams (fun t ->
        mk (Reason.Rwitness (fst t.tp_name), Typing_defs.make_tany ()))
  in
  let subst = Inst.make_subst unchecked_tparams tyl in
  let decl_ty = Inst.instantiate subst decl_ty in
  match get_node decl_ty with
  | Tapply _ ->
    let (env, locl_ty) = Phase.localize_with_self env decl_ty in
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
              (Phase.env_with_self env) with
              substs = Subst.make_locl tc_tparams tyl;
            }
          in
          iter2_shortest
            begin
              fun { tp_name = (p, x); tp_constraints = cstrl; _ } ty ->
              List.iter cstrl (fun (ck, cstr_ty) ->
                  let r = Reason.Rwitness p in
                  let (env, cstr_ty) = Phase.localize ~ety_env env cstr_ty in
                  let (_ : Typing_env_types.env) =
                    TGenConstraint.check_constraint
                      env
                      ck
                      ty
                      ~cstr_ty
                      (fun ?code:_ l ->
                        Reason.explain_generic_constraint (fst h) r x l)
                  in
                  ())
            end
            tc_tparams
            tyl
        | _ -> ())
      | _ -> ()
    end
  | _ -> ()

and class_ tenv c =
  let env = { typedef_tparams = []; tenv } in
  (* Add type parameters to typing environment and localize the bounds *)
  let tenv =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      (fst c.c_name)
      tenv
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
  List.iter c.c_pu_enums (pu_enum env (snd c.c_name));
  maybe enum env c.c_enum

and typeconst (env, _) tconst =
  maybe hint env tconst.c_tconst_type;
  maybe hint env tconst.c_tconst_constraint

and const env class_const = maybe hint env class_const.cc_type

and class_var env cv = maybe hint env (hint_of_type_hint cv.cv_type)

and method_ env m =
  (* Add method type parameters to environment and localize the bounds
     and where constraints *)
  let tenv =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      (fst m.m_name)
      env.tenv
      m.m_tparams
      m.m_where_constraints
  in
  let env = { env with tenv } in
  List.iter m.m_params (fun_param env);
  variadic_param env m.m_variadic;
  List.iter m.m_tparams (tparam env);
  List.iter m.m_where_constraints (where_constr env);
  maybe hint env (hint_of_type_hint m.m_ret)

and fun_param env param =
  maybe hint env (hint_of_type_hint param.param_type_hint)

and variadic_param env vparam =
  match vparam with
  | FVvariadicArg p -> fun_param env p
  | _ -> ()

and pu_enum env c_name pu =
  (* PU case type definitions can be constrained, e.g.
   * class C {
   *   enum E {
   *     case type T as arraykey;
   *     :@I(type T = int); // ok, int <: arraykey
   *     :@S(type T = string); // ok, string <: arraykey
   *     :@F(type F = float); // ko, float is not a subtype of arraykey
   *   }
   *}
   *
   * In here, we localize the type hint (type T = ...) found in each
   * instances of a PU (the :@I/:@S/:@F above) and check if the resulting
   * locl_ty satisfies the sub-typing constraints of the definition (as
   * arraykey).
   *)
  let tenv = env.tenv in
  let cls = Env.get_class tenv c_name in
  let pu_enum =
    Option.bind cls ~f:(fun cls -> Cls.get_pu_enum cls (snd pu.pu_name))
  in
  let pu_enum_case_types =
    match pu_enum with
    | None -> SMap.empty
    | Some pu_enum -> pu_enum.tpu_case_types
  in
  let pum_types (sid, h) =
    let () = hint env h in
    let decl_ty = Decl_hint.hint tenv.decl_env h in
    let (env, locl_ty) = Phase.localize_with_self tenv decl_ty in
    let case_ty = SMap.find_opt (snd sid) pu_enum_case_types in
    let ety_env = Phase.env_with_self env in
    match case_ty with
    | None ->
      (* error already caught by naming *)
      ()
    | Some (_, { tp_name = (p, x); tp_constraints = cstrl; _ }) ->
      let r = Reason.Rwitness p in
      List.iter cstrl ~f:(fun (ck, cstr_ty) ->
          let (env, cstr_ty) = Phase.localize ~ety_env env cstr_ty in
          let (_ : Typing_env_types.env) =
            TGenConstraint.check_constraint
              env
              ck
              locl_ty
              ~cstr_ty
              (fun ?code:_ l -> Reason.explain_generic_constraint (fst h) r x l)
          in
          ())
  in
  let pu_member pum = List.iter ~f:pum_types pum.pum_types in
  List.iter ~f:(tparam env) pu.pu_case_types;
  List.iter ~f:(fun (_, h) -> hint env h) pu.pu_case_values;
  List.iter ~f:pu_member pu.pu_members

and enum env e =
  hint env e.e_base;
  maybe hint env e.e_constraint

let typedef tenv t =
  (* We don't allow constraints on typdef parameters, but we still
     need to record their kinds in the generic var environment *)
  let tparams =
    List.map t.t_tparams (Decl_hint.aast_tparam_to_decl_tparam tenv.decl_env)
  in
  let tenv_with_typedef_tparams =
    Phase.localize_and_add_generic_parameters (fst t.t_name) tenv tparams
  in
  (* For typdefs, we do want to do the simple kind checks on the body
     (e.g., arities match up), but no constraint checks. We need to check the
     kinds of typedefs separately, because check_happly replaces all the generic
     parameters of typedefs by Tany, which makes the kind check moot *)
  maybe check_hint_wellkindedness tenv_with_typedef_tparams t.t_constraint;
  check_hint_wellkindedness tenv_with_typedef_tparams t.t_kind;
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
  hint_no_kind_check env t.t_kind
