(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
open Typing_defs
open Typing_dependent_type
open Utils

module Env = Typing_env
module TUtils = Typing_utils
module TGenConstraint = Typing_generic_constraint
module Subst = Decl_subst
module ShapeMap = Nast.ShapeMap

(* Here is the general problem the delayed application of the phase solves.
 * Let's say you have a function that you want to operate generically across
 * phases. In most cases when you do this you can use the 'ty' GADT and locally
 * abstract types to write code in a phase agonistic way.
 *
 *  let yell_any: type a. a ty -> string = fun ty ->
 *    match ty with
 *    | _, Tany -> "Any"
 *    | _ -> ""
 *
 * Now let's add a function that works for all phases, but whose logic is phase
 * dependent. For this we can use 'phase_ty' ADT:
 *
 *  let yell_locl phase_ty =
 *     match phase_ty with
 *     | DeclTy ty -> ""
 *     | LoclTy ty -> "Locl"
 *
 * Let's say you want to write a function that has behavior that works across
 * phases, but needs to invoke a function that is phase dependent. Our options
 * are as follows.
 *
 *  let yell_any_or_locl phase_ty =
 *    let ans = yell_locl phase_ty in
 *    match phase_ty with
 *    | DeclTy ty -> ans ^ (yell_any ty)
 *    | LoclTy ty -> ans ^ (yell_any ty)
 *
 * This would lead to code duplication since we can't generically operate on the
 * underlying 'ty' GADT. If we want to eliminate this code duplication there are
 * two options.
 *
 *  let generic_ty: type a. phase_ty -> a ty = function
 *    | DeclTy ty -> ty
 *    | LoclTy ty -> ty
 *
 *  let yell_any_or_locl phase_ty =
 *    let ans = yell_locl phase_ty in
 *    ans ^ (yell_any (generic_ty phase_ty))
 *
 * generic_ty allows us to extract a generic value which we can use. This
 * approach is limiting because we lose all information about what phase 'a ty
 * is.
 *
 * The other approach is to pass in a function that goes from 'a ty -> phase_ty'
 *
 *  let yell_any_or_locl phase ty =
 *    let ans = yell_locl (phase ty) in
 *    ans ^ (yell_any ty)
 *
 * Here we can use 'ty' generically (without losing information about what phase
 * 'a ty' is), and we rely on the caller passing in an appropriate function that
 * converts into the 'phase_ty' when we need to hop into phase specific code.
 *)
let decl ty = DeclTy ty
let locl ty = LoclTy ty

type env = expand_env

let env_with_self env =
  {
    type_expansions = [];
    substs = SMap.empty;
    this_ty = Reason.none, TUtils.this_of (Env.get_self env);
    from_class = None;
    validate_dty = None;
  }
(*****************************************************************************)
(* Transforms a declaration phase type into a localized type. This performs
 * common operations that are necessary for this operation, specifically:
 *   > Expand newtype/types
 *   > Resolves the "this" type
 *   > Instantiate generics
 *   > ...
 *
 * When keep track of additional information while localizing a type such as
 * what type defs were expanded to detect potentially recursive definitions..
 *)
(*****************************************************************************)

let rec localize_with_env ~ety_env env (dty: decl ty) =
  Option.iter ety_env.validate_dty (fun validate_dty -> validate_dty dty);
  match dty with
  | r, Terr ->
      env, (ety_env, (r, TUtils.terr env))
  | r, Tany ->
      env, (ety_env, (r, TUtils.tany env))
  | _, (Tnonnull | Tprim _ | Tdynamic) as x ->
      env, (ety_env, x)
  | r, Tmixed ->
      env, (ety_env, (r, TUtils.desugar_mixed r))
  | r, Tthis ->
      let ty = match ety_env.this_ty with
        | Reason.Rnone, ty -> r, ty
        | Reason.Rexpr_dep_type (_, pos, s), ty ->
            Reason.Rexpr_dep_type (r, pos, s), ty
        | reason, ty when ety_env.from_class <> None -> reason, ty
        | reason, ty ->
            Reason.Rinstantiate (reason, SN.Typehints.this, r), ty in
      let env, ty =
        match ety_env.from_class with
        | Some cid -> ExprDepTy.make env cid ty
        | _ -> env, ty in
      env, (ety_env, ty)
  | r, Tarray (ty1, ty2) ->
      let env, ty = match ty1, ty2 with
        | None, None -> env, Tarraykind AKany
        | Some tv, None ->
            let env, tv = localize ~ety_env env tv in
            env, Tarraykind (AKvec tv)
        | Some tk, Some tv ->
            let env, tk = localize ~ety_env env tk in
            let env, tv = localize ~ety_env env tv in
            env, Tarraykind (AKmap (tk, tv))
        | None, Some _ ->
            failwith "Invalid array declaration type" in
      env, (ety_env, (r, ty))
  | r, Tdarray (tk, tv) ->
      let env, tk = localize ~ety_env env tk in
      let env, tv = localize ~ety_env env tv in
      let ty = Tarraykind (AKdarray (tk, tv)) in
      env, (ety_env, (r, ty))
  | r, Tvarray tv ->
      let env, tv = localize ~ety_env env tv in
      let ty = Tarraykind (AKvarray tv) in
      env, (ety_env, (r, ty))
  | r, Tvarray_or_darray tv ->
      let env, tv = localize ~ety_env env tv in
      let ty = Tarraykind (AKvarray_or_darray tv) in
      env, (ety_env, (r, ty))
  | r, Tgeneric x ->
      begin match SMap.get x ety_env.substs with
      | Some x_ty ->
        env, (ety_env, (Reason.Rinstantiate (fst x_ty, x, r), snd x_ty))
      | None ->
        env, (ety_env, (r, Tabstract (AKgeneric x, None)))
    end
  | r, Toption ty ->
       let env, ty = localize ~ety_env env ty in
       let ty_ =
         if TUtils.is_option env ty then
           snd ty
         else
           Toption ty in
       env, (ety_env, (r, ty_))
  | r, Tfun ft ->
      let env, ft = localize_ft ~use_pos:ft.ft_pos ~ety_env env ft in
      env, (ety_env, (r, Tfun ft))
  | r, Tapply ((_, x), argl) when Env.is_typedef x ->
      let env, argl = List.map_env env argl (localize ~ety_env) in
      TUtils.expand_typedef ety_env env r x argl
  | r, Tapply ((p, x), _argl) when Env.is_enum env x ->
      (* if argl <> [], nastInitCheck would have raised an error *)
      if Typing_defs.has_expanded ety_env x then begin
        Errors.cyclic_enum_constraint p;
        env, (ety_env, (r, Typing_utils.tany env))
      end else begin
        let type_expansions = (p, x) :: ety_env.type_expansions in
        let ety_env = {ety_env with type_expansions} in
        let env, cstr =
          opt (localize ~ety_env) env (Env.get_enum_constraint env x) in
        env, (ety_env, (r, Tabstract (AKenum x, cstr)))
      end
  | r, Tapply ((_, cid) as cls, tyl) ->
      let env, tyl =
        if not (tyl_contains_wildcard tyl)
        then List.map_env env tyl (localize ~ety_env)
        else match Env.get_class env cid with
          | None ->
            List.map_env env tyl (localize ~ety_env)
          | Some class_info ->
            let tparams = class_info.tc_tparams in
            localize_tparams ~ety_env env (Reason.to_pos r) tyl tparams
      in
      env, (ety_env, (r, Tclass (cls, tyl)))
  | r, Ttuple tyl ->
      let env, tyl = List.map_env env tyl (localize ~ety_env) in
      env, (ety_env, (r, Ttuple tyl))
  | r, Taccess (root_ty, ids) ->
      let env, root_ty = localize ~ety_env env root_ty in
      TUtils.expand_typeconst ety_env env r root_ty ids
  | r, Tshape (fields_known, tym) ->
      let env, tym = ShapeFieldMap.map_env (localize ~ety_env) env tym in
      env, (ety_env, (r, Tshape (fields_known, tym)))

and localize_tparams ~ety_env env pos tyl tparams =
  let length = min (List.length tyl) (List.length tparams) in
  let tyl, tparams = List.take tyl length, List.take tparams length in
  let ety_env = { ety_env with validate_dty = None; } in
  let (env, _), tyl = List.map2_env (env, ety_env) tyl tparams (localize_tparam pos) in
  env, tyl

and localize_tparam pos (env, ety_env) ty (_, (_, name), cstrl, _) =
  match ty with
    | r, Tapply ((_, x), _argl) when x = SN.Typehints.wildcard ->
      let env, new_name = Env.add_fresh_generic_parameter env name in
      let ty_fresh = (r, Tabstract (AKgeneric new_name, None)) in
      let env = List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
        let env, ty = localize ~ety_env env ty in
        TUtils.add_constraint pos env ck ty_fresh ty) in
      (* Substitute fresh type parameters for original formals in constraint *)
      let substs = SMap.add name ty_fresh ety_env.substs in
      let ety_env = { ety_env with substs; } in
      (env, ety_env), ty_fresh
    | _ ->
      let env, ty = localize ~ety_env env ty in
      (env, ety_env), ty

and tyl_contains_wildcard tyl =
  List.exists tyl begin function
    | _, Tapply ((_, x), _) -> x = SN.Typehints.wildcard
    | _ -> false
  end

and localize ~ety_env env ty =
  let env, (_, ty) = localize_with_env ~ety_env env ty in
  env, ty

(* For the majority of cases when we localize a function type we instantiate
 * the function's type parameters to be a Tunresolved wrapped in a Tvar so the
 * type can grow. There are two cases where we do not do this.

 * 1) In Typing_subtype.subtype_method. See the comment for that function for why
 * this is necessary.
 * 2) When the type arguments are explicitly specified, in which case we instantiate
 * the type parameters to the provided types.
 *)
and localize_ft ~use_pos ?(instantiate_tparams=true) ?(explicit_tparams=[]) ~ety_env env ft =
  (* set reactivity to Nonreactive to prevent occasional setting
     of condition types when expanding type constants *)
  let saved_r = Env.env_reactivity env in
  let env = Env.set_env_reactive env Nonreactive in
  (* If no explicit type parameters are provided, set the instantiated type parameter to
   * initially point to unresolved, so that it can grow and eventually be a subtype of
   * something like "mixed".
   * If explicit type parameters are provided, just instantiate tvarl to them.
   *)
  let env, substs =
    if instantiate_tparams
    then
      let default () = List.map_env env ft.ft_tparams TUtils.unresolved_tparam in
      let env, tvarl =
        if List.length explicit_tparams = 0
        then default ()
        else if List.length explicit_tparams <> List.length ft.ft_tparams
        then begin
          Errors.expected_tparam ft.ft_pos (List.length ft.ft_tparams);
          default ()
        end
        else
          let type_argument env hint =
            match hint with
            | (pos, Nast.Happly ((_, id), [])) when id = SN.Typehints.wildcard ->
              let reason = Reason.Rwitness pos in
              TUtils.in_var env (reason, Tunresolved [])
            | _ -> localize_hint_with_self env hint in
          List.map_env env explicit_tparams type_argument
      in
      let ft_subst = Subst.make ft.ft_tparams tvarl in
      env, SMap.union ft_subst ety_env.substs
    else
      env, List.fold_left ft.ft_tparams ~f:begin fun subst (_, (_, x), _, _) ->
        SMap.remove x subst
      end ~init:ety_env.substs
  in
  let ety_env = {ety_env with substs = substs} in
  let env, params = List.map_env env ft.ft_params begin fun env param ->
    let env, ty = localize ~ety_env env param.fp_type in
    env, { param with fp_type = ty }
  end in
  (* restore reactivity *)
  let env =
    if saved_r <> Env.env_reactivity env
    then Env.set_env_reactive env saved_r
    else env
  in

  (* Localize the constraints for a type parameter declaration *)
  let localize_tparam env (var, name, cstrl, reified) =
    let env, cstrl = List.map_env env cstrl begin fun env (ck, ty) ->
      let env, ty = localize ~ety_env env ty in
      let name_str = snd name in
      (* In order to access type constants on generics on where clauses,
        we need to add the constraints from the type parameters into the
        environment before localizing the where clauses with them. Temporarily
        add them to the environment here, and reset the environment later.
      *)
      let env = match ck with
      | Ast.Constraint_as ->
        Env.add_upper_bound env name_str ty
      | Ast.Constraint_super ->
        Env.add_lower_bound env name_str ty
      | Ast.Constraint_eq ->
        Env.add_upper_bound (Env.add_lower_bound env name_str ty) name_str ty in
      env, (ck, ty)
    end in
    env, (var, name, cstrl, reified)
  in

  let localize_where_constraint env (ty1, ck, ty2) =
    let env, ty1 = localize ~ety_env env ty1 in
    let env, ty2 = localize ~ety_env env ty2 in
    env, (ty1, ck, ty2)
  in

  (* Grab and store the old tpenvs *)
  let old_tpenv = env.Env.lenv.Env.tpenv in
  let old_global_tpenv = env.Env.global_tpenv in

  (* If we're instantiating the generic parameters then remove them
   * from the result. Otherwise localize them *)
  let env, tparams =
    if instantiate_tparams then env, []
    else List.map_env env ft.ft_tparams localize_tparam in

  (* Localize the 'where' constraints *)
  let env, where_constraints =
    List.map_env env ft.ft_where_constraints localize_where_constraint in

  (* Remove the constraints we added for localizing where constraints  *)
  let env = Env.env_with_tpenv env old_tpenv in
  let env = Env.env_with_global_tpenv env old_global_tpenv in

  (* If we're instantiating the generic parameters then add a deferred
   * check that constraints are satisfied under the
   * substitution [ety_env.substs].
   *)
  let env =
    if instantiate_tparams then
      let env = check_tparams_constraints ~use_pos ~ety_env env ft.ft_tparams in
      let env = check_where_constraints ~use_pos ~definition_pos:ft.ft_pos ~ety_env env
                  ft.ft_where_constraints in
      env
    else env in

  let env, arity = match ft.ft_arity with
    | Fvariadic (min, ({ fp_type = var_ty; _ } as param)) ->
       let env, var_ty = localize ~ety_env env var_ty in
       env, Fvariadic (min, { param with fp_type = var_ty })
    | Fellipsis _ | Fstandard (_, _) as x -> env, x in
  let env, ret = localize ~ety_env env ft.ft_ret in
  env, { ft with ft_arity = arity; ft_params = params;
                 ft_ret = ret; ft_tparams = tparams;
                 ft_where_constraints = where_constraints }

(* Given a list of generic parameters [tparams] and a substitution
 * in [ety_env.substs] whose domain is at least these generic parameters,
 * check that the types satisfy
 * the constraints on the corresponding generic parameter.
 *
 * Note that the constraints may contain occurrences of the generic
 * parameters, but the subsitution will be applied to them. e.g. if tparams is
 *   <Tu as MyCovariant<Tu>, Tv super Tu>
 * and ety_env.substs is
 *   Tu :-> C
 *   Tv :-> I
 * with
 *   class C extends MyContravariant<I> implements I { ... }
 * Then the constraints are satisfied, because
 *   C is a subtype of MyContravariant<C>
 *   I is a supertype of C
 *
 * In fact, the constraint checking isn't done immediately, but rather pushed
 * onto the env.todo list. Typically we haven't resolved types sufficiently
 * (e.g. we have completely unresolved type variables) and so the actual
 * constraint checking is deferred until we have finished checking a
 * function's body.
 *)
and check_tparams_constraints ~use_pos ~ety_env env tparams =
  let check_tparam_constraints env (_variance, id, cstrl, _) =
    List.fold_left cstrl ~init:env ~f:begin fun env (ck, ty) ->
      let env, ty = localize ~ety_env env ty in
      match SMap.get (snd id) ety_env.substs with
      | Some x_ty ->
        begin
          Typing_log.log_types 1 use_pos env
          [Typing_log.Log_sub ("check_tparams_constraints: add_check_constraint_todo",
            [Typing_log.Log_type ("ty", ty);
            Typing_log.Log_type ("x_ty", x_ty)])];
          TGenConstraint.add_check_constraint_todo env ~use_pos id ck ty x_ty
        end
      | None ->
        env
    end in
  List.fold_left tparams ~init:env ~f:check_tparam_constraints

and check_where_constraints ~use_pos ~ety_env ~definition_pos env cstrl =
  List.fold_left cstrl ~init:env ~f:begin fun env (ty1, ck, ty2) ->
      let contains_type_access =
        match ty1, ty2 with
        | (_, Taccess ((_, Tgeneric _), _)), _
        | _, (_, Taccess ((_, Tgeneric _), _)) -> true
        | _ -> false in
      if contains_type_access then
      let ty_from_env = localize ~ety_env in
      TGenConstraint.add_check_tconst_where_constraint_todo
        env
        ~use_pos
        ~definition_pos
        ck
        ty_from_env
        ty2
        ty1
      else
      let env, ty1 = localize ~ety_env env ty1 in
      let env, ty2 = localize ~ety_env env ty2 in
      TGenConstraint.add_check_where_constraint_todo
        env
        ~use_pos
        ~definition_pos
        ck
        ty2
        ty1
    end



(* Performs no substitutions of generics and initializes Tthis to
 * Env.get_self env
 *)
and localize_with_self env ty =
  localize env ty ~ety_env:(env_with_self env)

and localize_with_dty_validator env ty validate_dty =
  let ety_env = {(env_with_self env) with validate_dty = Some validate_dty;} in
  localize ~ety_env env ty

and localize_hint_with_self env h =
  let h = Decl_hint.hint env.Env.decl_env h in
  localize_with_self env h

and localize_hint ~ety_env env hint =
  let hint_ty = Decl_hint.hint env.Env.decl_env hint in
  localize ~ety_env env hint_ty

(* Add generic parameters to the environment, localize their bounds, and
 * transform these into a flat list of constraints of the form (ty1,ck,ty2)
 * where ck is as, super or =
 *)
let localize_generic_parameters_with_bounds
    ~ety_env (env:Env.env) (tparams:Nast.tparam list) =
  let env = Env.add_generic_parameters env tparams in
  let localize_bound env ((_var, (pos,name), cstrl, _): Nast.tparam) =
    let tparam_ty = (Reason.Rwitness pos, Tabstract(AKgeneric name, None)) in
    List.map_env env cstrl (fun env (ck, h) ->
      let env, ty = localize env (Decl_hint.hint env.Env.decl_env h) ~ety_env in
      env, (tparam_ty, ck, ty)) in
  let env, cstrss = List.map_env env tparams localize_bound in
  env, List.concat cstrss

(* Helper functions *)

(* Ensure that types are equivalent i.e. subtypes of each other *)
let unify_decl env ty1 ty2 =
  let env, ty1 = localize_with_self env ty1 in
  let env, ty2 = localize_with_self env ty2 in
  ignore (TUtils.sub_type env ty2 ty1);
  ignore (TUtils.sub_type env ty1 ty2)

let sub_type_decl env ty1 ty2 =
  let env, ty1 = localize_with_self env ty1 in
  let env, ty2 = localize_with_self env ty2 in
  ignore (TUtils.sub_type env ty1 ty2)
