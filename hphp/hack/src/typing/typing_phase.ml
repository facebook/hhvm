(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Typing_defs
open Typing_env_types
open Typing_dependent_type
module Env = Typing_env
module TUtils = Typing_utils
module TGenConstraint = Typing_generic_constraint
module Subst = Decl_subst
module MakeType = Typing_make_type
module Cls = Decl_provider.Class

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

type method_instantiation = {
  use_pos: Pos.t;
  use_name: string;
  explicit_targs: Tast.targ list;
}

let env_with_self ?pos ?(quiet = false) env =
  let this_ty = mk (Reason.none, TUtils.this_of (Env.get_self env)) in
  {
    type_expansions = [];
    substs = SMap.empty;
    this_ty;
    from_class = None;
    quiet;
    on_error =
      (match pos with
      | None -> Errors.unify_error
      | Some pos -> Errors.unify_error_at pos);
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

let rec localize ~ety_env env (dty : decl_ty) =
  let tvar_or_localize ~ety_env env r ty ~i =
    if
      GlobalOptions.tco_global_inference env.genv.tcopt
      && (is_any ty || is_tyvar ty)
    then
      Env.new_global_tyvar env ~i r
    else
      localize ~ety_env env ty
  in
  match deref dty with
  | (r, Terr) -> (env, TUtils.terr env r)
  | (r, Tany _) -> (env, mk (r, TUtils.tany env))
  | (r, Tvar _var) -> Env.new_global_tyvar env r
  | (r, ((Tnonnull | Tprim _ | Tdynamic) as x)) -> (env, mk (r, x))
  | (r, Tmixed) -> (env, MakeType.mixed r)
  | (r, Tthis) ->
    let ty =
      match deref ety_env.this_ty with
      | (Reason.Rnone, ty) -> mk (r, ty)
      | (Reason.Rexpr_dep_type (_, pos, s), ty) ->
        mk (Reason.Rexpr_dep_type (r, pos, s), ty)
      | (reason, ty) when Option.is_some ety_env.from_class -> mk (reason, ty)
      | (reason, ty) ->
        mk (Reason.Rinstantiate (reason, SN.Typehints.this, r), ty)
    in
    let (env, ty) =
      match ety_env.from_class with
      | Some cid -> ExprDepTy.make env cid ty
      | _ -> (env, ty)
    in
    (env, ty)
  | (r, Tarray (ty1, ty2)) ->
    let (env, ty) =
      match (ty1, ty2) with
      | (None, None) ->
        let tk = MakeType.arraykey Reason.(Rvarray_or_darray_key (to_pos r)) in
        let (env, tv) =
          if GlobalOptions.tco_global_inference env.genv.tcopt then
            Env.new_global_tyvar env r
          else
            (env, mk (r, TUtils.tany env))
        in
        (env, Tarraykind (AKvarray_or_darray (tk, tv)))
      | (Some tv, None) ->
        let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
        (env, Tarraykind (AKvarray tv))
      | (Some tk, Some tv) ->
        let (env, tk) = tvar_or_localize ~ety_env env r tk ~i:0 in
        let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
        (env, Tarraykind (AKdarray (tk, tv)))
      | (None, Some _) -> failwith "Invalid array declaration type"
    in
    (env, mk (r, ty))
  | (r, Tdarray (tk, tv)) ->
    let (env, tk) = tvar_or_localize ~ety_env env r tk ~i:0 in
    let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
    let ty = Tarraykind (AKdarray (tk, tv)) in
    (env, mk (r, ty))
  | (r, Tvarray tv) ->
    let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:0 in
    let ty = Tarraykind (AKvarray tv) in
    (env, mk (r, ty))
  | (r, Tvarray_or_darray (tk, tv)) ->
    let (env, tk) =
      match tk with
      | Some tk -> tvar_or_localize ~ety_env env r tk ~i:0
      | None ->
        (env, MakeType.arraykey Reason.(Rvarray_or_darray_key (to_pos r)))
    in
    let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
    (env, MakeType.varray_or_darray r tk tv)
  | (r, Tgeneric x) ->
    begin
      match SMap.find_opt x ety_env.substs with
      | Some x_ty ->
        let (env, x_ty) = Env.expand_type env x_ty in
        (env, mk (Reason.Rinstantiate (get_reason x_ty, x, r), get_node x_ty))
      | None -> (env, mk (r, Tgeneric x))
    end
  | (r, Toption ty) ->
    let (env, ty) = localize ~ety_env env ty in
    TUtils.union env (MakeType.null r) ty
  | (r, Tlike ty) ->
    let (env, ty) = localize ~ety_env env ty in
    let (env, lty) = TUtils.union env (MakeType.dynamic r) ty in
    (env, lty)
  | (r, Tfun ft) ->
    let pos = Reason.to_pos r in
    let (env, ft) =
      localize_ft
        ~ety_env
        ~def_pos:pos
        ~instantiation:
          { use_pos = pos; use_name = "function"; explicit_targs = [] }
        env
        ft
    in
    (env, mk (r, Tfun ft))
  | (r, Tapply ((_, x), [arg]))
    when String.equal x Naming_special_names.FB.cIncorrectType
         && Env.is_typedef env x ->
    localize ~ety_env env (mk (r, Tlike arg))
  | (r, Tapply ((_, x), argl)) when Env.is_typedef env x ->
    let (env, argl) = List.map_env env argl (localize ~ety_env) in
    TUtils.expand_typedef ety_env env r x argl
  | (r, Tapply (((p, cid) as cls), argl)) ->
    begin
      match Env.get_class env cid with
      | None ->
        let (env, tyl) = List.map_env env argl (localize ~ety_env) in
        (env, mk (r, Tclass (cls, Nonexact, tyl)))
      | Some class_info ->
        if Option.is_some (Cls.enum_type class_info) then
          (* if argl <> [], nastInitCheck would have raised an error *)
          if Typing_defs.has_expanded ety_env cid then (
            Errors.cyclic_enum_constraint p;
            (env, mk (r, Typing_utils.tany env))
          ) else
            let type_expansions = cls :: ety_env.type_expansions in
            let ety_env = { ety_env with type_expansions } in
            let (env, cstr) =
              match Env.get_enum_constraint env cid with
              (* If not specified, default bound is arraykey *)
              | None ->
                ( env,
                  MakeType.arraykey
                    (Reason.Rimplicit_upper_bound (p, "arraykey")) )
              | Some ty -> localize ~ety_env env ty
            in
            (env, mk (r, Tnewtype (cid, [], cstr)))
        else
          let tparams = Cls.tparams class_info in
          let (env, tyl) =
            if
              TypecheckerOptions.global_inference (Env.get_tcopt env)
              && (not (List.is_empty tparams))
              && List.is_empty argl
            then
              (* In this case we will infer the missing type parameters *)
              localize_missing_tparams_class env r cls class_info
            else
              localize_tparams ~ety_env env (Reason.to_pos r) argl tparams
          in
          (env, mk (r, Tclass (cls, Nonexact, tyl)))
    end
  | (r, Ttuple tyl) ->
    let (env, tyl) = List.map_env env tyl (localize ~ety_env) in
    (env, mk (r, Ttuple tyl))
  | (r, Tunion tyl) ->
    let (env, tyl) = List.map_env env tyl (localize ~ety_env) in
    (env, mk (r, Tunion tyl))
  | (r, Tintersection tyl) ->
    let (env, tyl) = List.map_env env tyl (localize ~ety_env) in
    (env, mk (r, Tintersection tyl))
  | (r, Taccess (root_ty, ids)) ->
    (* Sometimes, Tthis and Tgeneric are not expanded to Tabstract, so we need
    to allow accessing abstract type constants here. *)
    let allow_abstract_tconst =
      match get_node root_ty with
      | Tthis
      | Tgeneric _ ->
        true
      | _ -> false
    in
    let (env, root_ty) = localize ~ety_env env root_ty in
    let (env, ety) =
      List.fold ids ~init:(env, root_ty) ~f:(fun (env, root_ty) id ->
          TUtils.expand_typeconst
            ety_env
            env
            root_ty
            id
            ~on_error:ety_env.on_error
            ~allow_abstract_tconst)
    in
    let (expansion_reason, ty) = deref ety in
    (* Elaborate reason with information about expression dependent types and
     * the original location of the Taccess type
     *)
    let elaborate_reason expand_reason =
      (* First convert into a string of root_ty::ID1::ID2::IDn *)
      let taccess_string =
        String.concat
          ~sep:"::"
          (Typing_print.full_strip_ns env root_ty :: List.map ~f:snd ids)
      in
      (* If the root is an expression dependent type, change the primary
       * reason to be for the full Taccess type to preserve the position where
       * the expression dependent type was derived from.
       *)
      let reason =
        match get_reason root_ty with
        | Reason.Rexpr_dep_type (_, p, e) -> Reason.Rexpr_dep_type (r, p, e)
        | _ -> r
      in
      Reason.Rtype_access (expand_reason, [(reason, taccess_string)])
    in
    (env, mk (elaborate_reason expansion_reason, ty))
  | (r, Tshape (shape_kind, tym)) ->
    let (env, tym) = ShapeFieldMap.map_env (localize ~ety_env) env tym in
    (env, mk (r, Tshape (shape_kind, tym)))
  | (r, Tpu_access _) ->
    (* We explicitly forbid the syntax C:@E:X in here, since it brings
       more complexity to the type checker and do not allow anything
       interesting at the moment. If one need a variable ranging over PU
       member, one should use "C:@E" instead *)
    let rec build_access dty =
      match get_node dty with
      | Tpu_access (base, sid, pu_loc) ->
        let (base, trailing) = build_access base in
        (base, (sid, pu_loc) :: trailing)
      | _ -> (dty, [])
    in
    let (base, trailing) = build_access dty in
    let (env, base) = localize ~ety_env env base in
    (match trailing with
    | [(enum, _)] -> (env, mk (r, Tpu (base, enum)))
    | [(ty, _); member; (enum, _)] ->
      (* Try to expand the dependent type *)
      TUtils.expand_pocket_universes env ~ety_env r base enum member ty
    (* Invalid number of :@, report an error *)
    | _ ->
      Errors.pu_localize_unknown
        (Reason.to_pos r)
        (Typing_print.full_decl (Typing_env.get_ctx env) dty);
      (env, TUtils.terr env r))

and localize_tparams ~ety_env env pos tyl tparams =
  let length = min (List.length tyl) (List.length tparams) in
  let (tyl, tparams) = (List.take tyl length, List.take tparams length) in
  let ((env, _), tyl) =
    List.map2_env (env, ety_env) tyl tparams (localize_tparam pos)
  in
  (env, tyl)

and localize_tparam pos (env, ety_env) ty tparam =
  match deref ty with
  | (r, Tapply ((_, x), _argl)) when String.equal x SN.Typehints.wildcard ->
    let {
      tp_name = (_, name);
      tp_constraints = cstrl;
      tp_reified = reified;
      tp_user_attributes;
      _;
    } =
      tparam
    in
    let enforceable =
      Naming_attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes
    in
    let newable =
      Naming_attributes.mem SN.UserAttributes.uaNewable tp_user_attributes
    in
    let (env, new_name) =
      Env.add_fresh_generic_parameter env name ~reified ~enforceable ~newable
    in
    let ty_fresh = mk (r, Tgeneric new_name) in
    (* Substitute fresh type parameters for original formals in constraint *)
    let substs = SMap.add name ty_fresh ety_env.substs in
    let ety_env = { ety_env with substs } in
    let env =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
          let (env, ty) = localize ~ety_env env ty in
          TUtils.add_constraint pos env ck ty_fresh ty)
    in
    ((env, ety_env), ty_fresh)
  | _ ->
    let (env, ty) = localize ~ety_env env ty in
    ((env, ety_env), ty)

(* Recursive localizations of function types do not make judgements about enforceability *)
and localize_possibly_enforced_ty ~ety_env env ety =
  let (env, et_type) = localize ~ety_env env ety.et_type in
  (env, { ety with et_type })

and localize_cstr_ty ~ety_env env ty tp_name =
  let (env, ety) = localize ~ety_env env ty in
  let (r, ty_) = deref ety in
  let ty = mk (Reason.Rcstr_on_generics (Reason.to_pos r, tp_name), ty_) in
  (env, ty)

(* Localize an explicit type argument to a constructor or function. We
 * support the use of wildcards at the top level only *)
and localize_targ env hint =
  let ty = Decl_hint.hint env.decl_env hint in
  (* For explicit type arguments we support a wildcard syntax `_` for which
  * Hack will generate a fresh type variable *)
  match deref ty with
  | (r, Tapply ((_, id), [])) when String.equal id SN.Typehints.wildcard ->
    let (env, ty) = Env.fresh_type env (Reason.to_pos r) in
    (env, (ty, hint))
  | _ ->
    let (env, ty) = localize_with_self env ty in
    (env, (ty, hint))

(* See signature in .mli file for details *)
and localize_targs ~is_method ~def_pos ~use_pos ~use_name env tparaml targl =
  let tparam_count = List.length tparaml in
  let targ_count = List.length targl in
  (* If there are explicit type arguments but too few or too many then
   * report an error *)
  if Int.( <> ) targ_count 0 && Int.( <> ) tparam_count targ_count then
    if is_method then
      Errors.expected_tparam ~definition_pos:def_pos ~use_pos tparam_count None
    else
      Errors.type_arity
        use_pos
        def_pos
        ~expected:tparam_count
        ~actual:targ_count;

  (* Declare and localize the explicit type arguments *)
  (* TODO? Drop surplus explicit type arguments *)
  let (env, explicit_targs) = List.map_env env targl localize_targ in
  (* Generate fresh type variables for the remainder *)
  let (env, implicit_targs) =
    List.map_env env (List.drop tparaml targ_count) (fun env tparam ->
        let (env, tvar) =
          Env.fresh_type_reason
            env
            (Reason.Rtype_variable_generics
               (use_pos, snd tparam.tp_name, use_name))
        in
        Typing_log.log_tparam_instantiation env use_pos tparam tvar;
        ( env,
          (tvar, (use_pos, Aast.Happly ((Pos.none, SN.Typehints.wildcard), [])))
        ))
  in
  (env, explicit_targs @ implicit_targs)

(* For the majority of cases when we localize a function type we instantiate
 * the function's type parameters to be a Tunion wrapped in a Tvar so the
 * type can grow. There are two cases where we do not do this.

 * 1) In Typing_subtype.subtype_method. See the comment for that function for why
 * this is necessary.
 * 2) When the type arguments are explicitly specified, in which case we instantiate
 * the type parameters to the provided types.
 *)
and localize_ft ?instantiation ~ety_env ~def_pos env ft =
  (* set reactivity to Nonreactive to prevent occasional setting
     of condition types when expanding type constants *)
  let saved_r = env_reactivity env in
  let env = Env.set_env_reactive env Nonreactive in
  (* If no explicit type parameters are provided, set the instantiated type parameter to
   * initially point to unresolved, so that it can grow and eventually be a subtype of
   * something like "mixed".
   * If explicit type parameters are provided, just instantiate tvarl to them.
   *)
  let (env, substs) =
    match instantiation with
    | Some { explicit_targs; use_name = _; use_pos } ->
      if
        (not (List.is_empty explicit_targs))
        && Int.( <> ) (List.length explicit_targs) (List.length ft.ft_tparams)
      then
        Errors.expected_tparam
          ~definition_pos:def_pos
          ~use_pos
          (List.length ft.ft_tparams)
          None;
      let tvarl = List.map ~f:fst explicit_targs in
      let ft_subst = Subst.make_locl ft.ft_tparams tvarl in
      (env, SMap.union ft_subst ety_env.substs)
    | None ->
      ( env,
        List.fold_left
          ft.ft_tparams
          ~f:
            begin
              fun subst t ->
              SMap.remove (snd t.tp_name) subst
            end
          ~init:ety_env.substs )
  in
  let ety_env = { ety_env with substs } in
  (* restore reactivity *)
  let env =
    if not (equal_reactivity saved_r (env_reactivity env)) then
      Env.set_env_reactive env saved_r
    else
      env
  in
  (* Localize the constraints for a type parameter declaration *)
  let localize_tparam env t =
    let (env, cstrl) =
      List.map_env env t.tp_constraints (fun env (ck, ty) ->
          let (env, ty) = localize_cstr_ty ~ety_env env ty t.tp_name in
          let name_str = snd t.tp_name in
          (* In order to access type constants on generics on where clauses,
        we need to add the constraints from the type parameters into the
        environment before localizing the where clauses with them. Temporarily
        add them to the environment here, and reset the environment later.
      *)
          let env =
            match ck with
            | Ast_defs.Constraint_as -> Env.add_upper_bound env name_str ty
            | Ast_defs.Constraint_super -> Env.add_lower_bound env name_str ty
            | Ast_defs.Constraint_eq ->
              Env.add_upper_bound
                (Env.add_lower_bound env name_str ty)
                name_str
                ty
          in
          (env, (ck, ty)))
    in
    (env, { t with tp_constraints = cstrl })
  in
  let localize_where_constraint env (ty1, ck, ty2) =
    let (env, ty1) = localize ~ety_env env ty1 in
    let (env, ty2) = localize ~ety_env env ty2 in
    (env, (ty1, ck, ty2))
  in
  (* Grab and store the old tpenvs *)
  let old_tpenv = Env.get_tpenv env in
  let old_global_tpenv = env.global_tpenv in
  (* Always localize tparams so they are available for later Tast check *)
  let (env, tparams) = List.map_env env ft.ft_tparams localize_tparam in
  (* Localize the 'where' constraints *)
  let (env, where_constraints) =
    List.map_env env ft.ft_where_constraints localize_where_constraint
  in
  (* Remove the constraints we added for localizing where constraints  *)
  let env = Env.env_with_tpenv env old_tpenv in
  let env = Env.env_with_global_tpenv env old_global_tpenv in
  (* If we're instantiating the generic parameters then add a deferred
   * check that constraints are satisfied under the
   * substitution [ety_env.substs].
   *)
  let env =
    match instantiation with
    | Some { use_pos; _ } ->
      let env = check_tparams_constraints ~use_pos ~ety_env env ft.ft_tparams in
      let env =
        check_where_constraints
          ~in_class:false
          ~use_pos
          ~definition_pos:def_pos
          ~ety_env
          env
          ft.ft_where_constraints
      in
      env
    | None -> env
  in
  let (env, arity) =
    match ft.ft_arity with
    | Fvariadic (min, ({ fp_type = var_ty; _ } as param)) ->
      let (env, var_ty) = localize ~ety_env env var_ty.et_type in
      (* HHVM does not enforce types on vararg parameters yet *)
      ( env,
        Fvariadic
          ( min,
            { param with fp_type = { et_type = var_ty; et_enforced = false } }
          ) )
    | (Fellipsis _ | Fstandard _) as x -> (env, x)
  in
  let (env, params) =
    List.map_env env ft.ft_params (fun env param ->
        let (env, ty) =
          localize_possibly_enforced_ty ~ety_env env param.fp_type
        in
        (env, { param with fp_type = ty }))
  in
  let (env, ret) = localize_possibly_enforced_ty ~ety_env env ft.ft_ret in
  let ft =
    set_ft_ftk
      ft
      ( if Option.is_some instantiation then
        FTKinstantiated_targs
      else
        FTKtparams )
  in
  ( env,
    {
      ft with
      ft_arity = arity;
      ft_params = params;
      ft_ret = ret;
      ft_tparams = tparams;
      ft_where_constraints = where_constraints;
    } )

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
 *)
and check_tparams_constraints ~use_pos ~ety_env env tparams =
  let check_tparam_constraints env t =
    List.fold_left t.tp_constraints ~init:env ~f:(fun env (ck, ty) ->
        let (env, ty) = localize_cstr_ty ~ety_env env ty t.tp_name in
        match SMap.find_opt (snd t.tp_name) ety_env.substs with
        | Some x_ty ->
          Typing_log.(
            log_with_level env "generics" 1 (fun () ->
                log_types
                  use_pos
                  env
                  [
                    Log_head
                      ( "check_tparams_constraints: check_tparams_constraint",
                        [Log_type ("ty", ty); Log_type ("x_ty", x_ty)] );
                  ]));
          TGenConstraint.check_tparams_constraint
            env
            ~use_pos
            t.tp_name
            ck
            ty
            x_ty
        | None -> env)
  in
  List.fold_left tparams ~init:env ~f:check_tparam_constraints

and check_where_constraints
    ~in_class ~use_pos ~ety_env ~definition_pos env cstrl =
  List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
      let (env, ty1) = localize ~ety_env env ty1 in
      let (env, ty2) = localize ~ety_env env ty2 in
      TGenConstraint.check_where_constraint
        ~in_class
        env
        ~use_pos
        ~definition_pos
        ck
        ty2
        ty1)

(* Performs no substitutions of generics and initializes Tthis to
 * Env.get_self env
 *)
and localize_with_self env ?pos ?(quiet = false) ty =
  let ety_env = env_with_self env ?pos ~quiet in
  localize env ty ~ety_env

and localize_possibly_enforced_with_self env ety =
  let (env, et_type) = localize_with_self env ety.et_type in
  (env, { ety with et_type })

and localize_hint_with_self env h =
  let h = Decl_hint.hint env.decl_env h in
  localize_with_self env h

and localize_hint ~ety_env env hint =
  let hint_ty = Decl_hint.hint env.decl_env hint in
  localize ~ety_env env hint_ty

and localize_missing_tparams_class env r sid class_ =
  let use_pos = Reason.to_pos r in
  let use_name = Utils.strip_ns (snd sid) in
  let ((env, _i), tyl) =
    List.fold_map (Cls.tparams class_) ~init:(env, 0) ~f:(fun (env, i) tparam ->
        let (env, ty) =
          Env.new_global_tyvar
            env
            ~i
            (Reason.Rtype_variable_generics
               (use_pos, snd tparam.tp_name, use_name))
        in
        ((env, i + 1), ty))
  in
  let c_ty = mk (r, Tclass (sid, Nonexact, tyl)) in
  let ety_env =
    {
      type_expansions = [];
      this_ty = c_ty;
      substs = Subst.make_locl (Cls.tparams class_) tyl;
      from_class = Some (Aast.CI sid);
      quiet = false;
      on_error = Errors.unify_error_at use_pos;
    }
  in
  let env =
    check_tparams_constraints ~use_pos ~ety_env env (Cls.tparams class_)
  in
  let env =
    check_where_constraints
      ~in_class:true
      ~use_pos
      ~definition_pos:(Cls.pos class_)
      ~ety_env
      env
      (Cls.where_constraints class_)
  in
  (env, tyl)

(* Do all of the above, and also check any constraints associated with the type parameters.
 *)
and resolve_type_arguments_and_check_constraints
    ~exact
    ~check_constraints
    ~def_pos
    ~use_pos
    env
    class_id
    from_class
    tparaml
    hintl =
  let (env, type_argl) =
    localize_targs
      ~is_method:false
      ~def_pos
      ~use_pos
      ~use_name:(Utils.strip_ns (snd class_id))
      env
      tparaml
      hintl
  in
  let this_ty =
    mk
      ( Reason.Rwitness (fst class_id),
        Tclass (class_id, exact, List.map ~f:fst type_argl) )
  in
  let env =
    if check_constraints then
      let ety_env =
        {
          type_expansions = [];
          this_ty;
          substs = Subst.make_locl tparaml (List.map ~f:fst type_argl);
          from_class = Some from_class;
          quiet = false;
          on_error = Errors.unify_error_at use_pos;
        }
      in
      check_tparams_constraints ~use_pos ~ety_env env tparaml
    else
      env
  in
  (env, this_ty, type_argl)

(* Add generic parameters to the environment, localize their bounds, and
 * transform these into a flat list of constraints of the form (ty1,ck,ty2)
 * where ck is as, super or =
 *)
let localize_generic_parameters_with_bounds
    ~ety_env (env : env) (tparams : decl_tparam list) =
  let env = Env.add_generic_parameters env tparams in
  let localize_bound
      env ({ tp_name = (pos, name); tp_constraints = cstrl; _ } : decl_tparam) =
    let tparam_ty = mk (Reason.Rwitness pos, Tgeneric name) in
    List.map_env env cstrl (fun env (ck, cstr) ->
        let (env, ty) = localize env cstr ~ety_env in
        (env, (tparam_ty, ck, ty)))
  in
  let (env, cstrss) = List.map_env env tparams localize_bound in
  (env, List.concat cstrss)

let localize_where_constraints ~ety_env (env : env) where_constraints =
  let add_constraint env (h1, ck, h2) =
    let (env, ty1) = localize env (Decl_hint.hint env.decl_env h1) ~ety_env in
    let (env, ty2) = localize env (Decl_hint.hint env.decl_env h2) ~ety_env in
    TUtils.add_constraint (fst h1) env ck ty1 ty2
  in
  List.fold_left where_constraints ~f:add_constraint ~init:env

(* Helper functions *)

let sub_type_decl env ty1 ty2 on_error =
  let (env, ty1) = localize_with_self env ty1 in
  let (env, ty2) = localize_with_self env ty2 in
  let env = TUtils.sub_type env ty1 ty2 on_error in
  env

let () = TUtils.localize_with_self_ref := localize_with_self
