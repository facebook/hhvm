(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
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
type 'a t = 'a ty -> phase_ty
let decl ty = DeclTy ty
let locl ty = LoclTy ty

type env = expand_env

let env_with_self env =
  {
    type_expansions = [];
    substs = SMap.empty;
    this_ty = Reason.none, TUtils.this_of (Env.get_self env);
    from_class = None;
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
  match dty with
  | _, (Terr | Tany | Tmixed | Tprim _ ) as x -> env, (ety_env, x)
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
      let env, ft = localize_ft ~ety_env env ft in
      env, (ety_env, (r, Tfun ft))
  | r, Tapply ((_, x), argl) when Env.is_typedef x ->
      let env, argl = List.map_env env argl (localize ~ety_env) in
      TUtils.expand_typedef ety_env env r x argl
  | r, Tapply ((p, x), _argl) when Env.is_enum env x ->
      (* if argl <> [], nastInitCheck would have raised an error *)
      if Typing_defs.has_expanded ety_env x then begin
        Errors.cyclic_enum_constraint p;
        env, (ety_env, (r, Tany))
      end else begin
        let type_expansions = (p, x) :: ety_env.type_expansions in
        let ety_env = {ety_env with type_expansions} in
        let env, cstr =
          opt (localize ~ety_env) env (Env.get_enum_constraint env x) in
        env, (ety_env, (r, Tabstract (AKenum x, cstr)))
      end
  | r, Tapply (cls, tyl) ->
      let env, tyl = List.map_env env tyl (localize ~ety_env) in
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
and localize_ft ?(instantiate_tparams=true) ?(explicit_tparams=[]) ~ety_env env ft =
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
            | (pos, Nast.Happly ((_, "_"), [])) ->
              let reason = Reason.Rwitness pos in
              TUtils.in_var env (reason, Tunresolved [])
            | _ -> hint_locl env hint in
          List.map_env env explicit_tparams type_argument
      in
      let ft_subst = Subst.make ft.ft_tparams tvarl in
      env, SMap.union ft_subst ety_env.substs
    else
      env, List.fold_left ft.ft_tparams ~f:begin fun subst (_, (_, x), _) ->
        SMap.remove x subst
      end ~init:ety_env.substs
  in
  let ety_env = {ety_env with substs = substs} in
  let env, params = List.map_env env ft.ft_params begin fun env (name, param) ->
    let env, param = localize ~ety_env env param in
    env, (name, param)
  end in
  (* Localize the constraints for a type parameter declaration *)
  let localize_tparam env (var, name, cstrl) =
    let env, cstrl = List.map_env env cstrl begin fun env (ck, ty) ->
      let env, ty = localize ~ety_env env ty in
      env, (ck, ty)
    end in
    env, (var, name, cstrl)
  in

  let localize_where_constraint env (ty1, ck, ty2) =
    let env, ty1 = localize ~ety_env env ty1 in
    let env, ty2 = localize ~ety_env env ty2 in
    env, (ty1, ck, ty2)
  in

  (* If we're instantiating the generic parameters then remove them
   * from the result. Otherwise localize them *)
  let env, tparams =
    if instantiate_tparams then env, []
    else List.map_env env ft.ft_tparams localize_tparam in

  (* Localize the 'where' constraints *)
  let env, where_constraints =
    List.map_env env ft.ft_where_constraints localize_where_constraint in

  (* If we're instantiating the generic parameters then add a deferred
   * check that constraints are satisfied under the
   * substitution [ety_env.substs].
   *)
  let env =
    if instantiate_tparams then
      let env = check_tparams_constraints ~ety_env env ft.ft_tparams in
      let env = check_where_constraints ~ety_env env ft.ft_pos
                  ft.ft_where_constraints in
      env
    else env in

  let env, arity = match ft.ft_arity with
    | Fvariadic (min, (name, var_ty)) ->
       let env, var_ty = localize ~ety_env env var_ty in
       env, Fvariadic (min, (name, var_ty))
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
and check_tparams_constraints ~ety_env env tparams =
  let check_tparam_constraints env (_var, (p, name), cstrl) =
    let r = Reason.Rwitness p in
    List.fold_left cstrl ~init:env ~f:begin fun env (ck, ty) ->
      let env, ty = localize ~ety_env env ty in
      match SMap.get name ety_env.substs with
      | Some x_ty ->
      TGenConstraint.add_check_constraint_todo env r name ck ty x_ty
      | None ->
        env
    end in
  List.fold_left tparams ~init:env ~f:check_tparam_constraints

and check_where_constraints ~ety_env env def_pos cstrl =
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
        def_pos
        ck
        ty_from_env
        ty2
        ty1
      else
      let env, ty1 = localize ~ety_env env ty1 in
      let env, ty2 = localize ~ety_env env ty2 in
      TGenConstraint.add_check_where_constraint_todo
        env
        def_pos
        ck
        ty2
        ty1
    end



(* Performs no substitutions of generics and initializes Tthis to
 * Env.get_self env
 *)
and localize_with_self env ty =
  localize env ty ~ety_env:(env_with_self env)

and hint_locl env h =
  let h = Decl_hint.hint env.Env.decl_env h in
  localize_with_self env h

(* Add generic parameters to the environment, localize their bounds, and
 * transform these into a flat list of constraints of the form (ty1,ck,ty2)
 * where ck is as, super or =
 *)
let localize_generic_parameters_with_bounds
    ~ety_env (env:Env.env) (tparams:Nast.tparam list) =
  let env = Env.add_generic_parameters env tparams in
  let localize_bound env ((_var, (pos,name), cstrl): Nast.tparam) =
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
