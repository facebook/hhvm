(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This module performs checks after the naming has been done.
   Basically any check that doesn't fall in the typing category. *)
(* Check of type application arity *)
(* Check no `new AbstractClass` (or trait, or interface) *)
(* Check no top-level break / continue *)

(* NOTE: since the typing environment does not generally contain
   information about non-Hack code, some of these checks can
   only be done in strict (i.e. "everything is Hack") mode. Use
   `if Env.is_strict env.tenv` style checks accordingly.
*)

open Core_kernel
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

type env = {
  typedef_tparams: Nast.tparam list;
  tenv: Typing_env_types.env;
}

let rec fun_ tenv f =
  let env = { typedef_tparams = []; tenv } in
  let (p, _) = f.f_name in
  (* Add type parameters to typing environment and localize the bounds
     and where constraints *)
  let ety_env = Phase.env_with_self env.tenv in
  let f_tparams : decl_tparam list =
    List.map
      f.f_tparams
      ~f:(Decl_hint.aast_tparam_to_decl_tparam env.tenv.decl_env)
  in
  let (tenv, constraints) =
    Phase.localize_generic_parameters_with_bounds env.tenv f_tparams ~ety_env
  in
  let tenv = add_constraints p tenv constraints in
  let tenv =
    Phase.localize_where_constraints ~ety_env tenv f.f_where_constraints
  in
  let env = { env with tenv } in
  maybe hint env (hint_of_type_hint f.f_ret);

  List.iter f.f_tparams (tparam env);
  List.iter f.f_params (fun_param env)

and tparam env t = List.iter t.tp_constraints (fun (_, h) -> hint env h)

and where_constr env (h1, _, h2) =
  hint env h1;
  hint env h2

and hint env (p, h) = hint_ env p h

and hint_ env p = function
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
  | Harray (ty1, ty2) ->
    maybe hint env ty1;
    maybe hint env ty2
  | Hdarray (ty1, ty2) ->
    hint env ty1;
    hint env ty2
  | Hvarray_or_darray ty
  | Hvarray ty ->
    hint env ty
  | Htuple hl -> List.iter hl (hint env)
  | Hoption h
  | Hsoft h
  | Hlike h ->
    hint env h
  | Hfun (_, _, hl, _, _, variadic_hint, h, _) ->
    List.iter hl (hint env);
    hint env h;
    Option.iter variadic_hint (hint env)
  | Happly ((_, x), hl) as h when Env.is_typedef x ->
    begin
      match Decl_provider.get_typedef x with
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
    let compute_hint_for_shape_field_info { sfi_hint; _ } =
      hint env sfi_hint
    in
    List.iter ~f:compute_hint_for_shape_field_info nsi_field_map
  | Hpu_access (h, _) -> hint env h

and check_happly unchecked_tparams env h =
  let decl_ty = Decl_hint.hint env.decl_env h in
  let unchecked_tparams =
    List.map unchecked_tparams (fun t ->
        let cstrl =
          List.map t.tp_constraints (fun (ck, cstr) ->
              let cstr = Decl_hint.hint env.decl_env cstr in
              (ck, cstr))
        in
        {
          Typing_defs.tp_variance = t.tp_variance;
          tp_name = t.tp_name;
          tp_constraints = cstrl;
          tp_reified = t.tp_reified;
          tp_user_attributes = t.tp_user_attributes;
        })
  in
  let tyl =
    List.map unchecked_tparams (fun t ->
        (Reason.Rwitness (fst t.tp_name), Typing_defs.make_tany ()))
  in
  let subst = Inst.make_subst unchecked_tparams tyl in
  let decl_ty = Inst.instantiate subst decl_ty in
  match decl_ty with
  | (_, Tapply _) ->
    let (env, locl_ty) = Phase.localize_with_self env decl_ty in
    begin
      match TUtils.get_base_type env locl_ty with
      | (_, Tclass (cls, _, tyl)) ->
        (match Env.get_class env (snd cls) with
        | Some cls ->
          let tc_tparams = Cls.tparams cls in
          (* We want to instantiate the class type parameters with the
           * type list of the class we are localizing. We do not want to
           * add any more constraints when we localize the constraints
           * stored in the class_type since it may lead to infinite
           * recursion
           *)
          let tc_tparams =
            List.map
              tc_tparams
              ~f:(Typing_enforceability.pessimize_tparam_constraints env)
          in
          let ety_env =
            {
              (Phase.env_with_self env) with
              substs = Subst.make tc_tparams tyl;
            }
          in
          iter2_shortest
            begin
              fun { tp_name = (p, x); tp_constraints = cstrl; _ } ty ->
              List.iter cstrl (fun (ck, cstr_ty) ->
                  let r = Reason.Rwitness p in
                  let (env, cstr_ty) = Phase.localize ~ety_env env cstr_ty in
                  ignore
                  @@ Errors.try_
                       (fun () ->
                         TGenConstraint.check_constraint env ck ty ~cstr_ty)
                       (fun l ->
                         Reason.explain_generic_constraint (fst h) r x l;
                         env))
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
  let c_tparam_list : decl_tparam list =
    List.map
      c.c_tparams.c_tparam_list
      ~f:(Decl_hint.aast_tparam_to_decl_tparam env.tenv.decl_env)
  in
  let (tenv, constraints) =
    Phase.localize_generic_parameters_with_bounds
      tenv
      c_tparam_list
      ~ety_env:(Phase.env_with_self tenv)
  in
  let tenv = add_constraints (fst c.c_name) tenv constraints in
  (* When where clauses are present, we need instantiate those type
   * parameters before checking base class *)
  let tenv =
    Phase.localize_where_constraints
      ~ety_env:(Phase.env_with_self tenv)
      tenv
      c.c_where_constraints
  in
  let env = { env with tenv } in
  let (c_constructor, c_statics, c_methods) = split_methods c in
  let (c_static_vars, c_vars) = split_vars c in
  if not (c.c_kind = Ast_defs.Cinterface) then maybe method_ env c_constructor;
  List.iter c.c_tparams.c_tparam_list (tparam env);
  List.iter c.c_where_constraints (where_constr env);
  List.iter c.c_extends (hint env);
  List.iter c.c_implements (hint env);
  List.iter c.c_typeconsts (typeconst (env, c.c_tparams.c_tparam_list));
  List.iter c_static_vars (class_var env);
  List.iter c_vars (class_var env);
  List.iter c_statics (method_ env);
  List.iter c_methods (method_ env)

and typeconst (env, _) tconst =
  maybe hint env tconst.c_tconst_type;
  maybe hint env tconst.c_tconst_constraint

and class_var env cv = maybe hint env cv.cv_type

and add_constraint pos tenv (ty1, ck, ty2) =
  Typing_subtype.add_constraint pos tenv ck ty1 ty2

and add_constraints pos tenv (cstrs : locl_where_constraint list) =
  List.fold_left cstrs ~init:tenv ~f:(add_constraint pos)

and method_ env m =
  (* Add method type parameters to environment and localize the bounds
     and where constraints *)
  let ety_env = Phase.env_with_self env.tenv in
  let m_tparams : decl_tparam list =
    List.map
      m.m_tparams
      ~f:(Decl_hint.aast_tparam_to_decl_tparam env.tenv.decl_env)
  in
  let (tenv, constraints) =
    Phase.localize_generic_parameters_with_bounds env.tenv m_tparams ~ety_env
  in
  let tenv = add_constraints (fst m.m_name) tenv constraints in
  let tenv =
    Phase.localize_where_constraints ~ety_env tenv m.m_where_constraints
  in
  let env = { env with tenv } in
  List.iter m.m_params (fun_param env);
  List.iter m.m_tparams (tparam env);
  maybe hint env (hint_of_type_hint m.m_ret)

and fun_param env param =
  maybe hint env (hint_of_type_hint param.param_type_hint)

let typedef tenv t =
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
  maybe hint env t.t_constraint;
  hint env t.t_kind
