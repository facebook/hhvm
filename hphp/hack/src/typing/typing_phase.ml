(*
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
  explicit_targs: decl_ty list;
}

let env_with_self env =
  let this_ty = (Reason.none, TUtils.this_of (Env.get_self env)) in
  { type_expansions = []; substs = SMap.empty; this_ty; from_class = None }

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
  match dty with
  | (r, Terr) -> (env, (r, TUtils.terr env))
  | (r, Tany _) -> (env, (r, TUtils.tany env))
  | (r, Tvar var) ->
    (Env.create_global_tyvar env var (Reason.to_pos r), (r, Tvar var))
  | (_, (Tnonnull | Tprim _ | Tdynamic)) as x -> (env, x)
  | (r, Tmixed) -> (env, MakeType.mixed r)
  | (r, Tnothing) -> (env, MakeType.nothing r)
  | (r, Tthis) ->
    let ty =
      match ety_env.this_ty with
      | (Reason.Rnone, ty) -> (r, ty)
      | (Reason.Rexpr_dep_type (_, pos, s), ty) ->
        (Reason.Rexpr_dep_type (r, pos, s), ty)
      | (reason, ty) when ety_env.from_class <> None -> (reason, ty)
      | (reason, ty) -> (Reason.Rinstantiate (reason, SN.Typehints.this, r), ty)
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
      | (None, None) -> (env, Tarraykind AKany)
      | (Some tv, None) ->
        let (env, tv) = localize ~ety_env env tv in
        (env, Tarraykind (AKvec tv))
      | (Some tk, Some tv) ->
        let (env, tk) = localize ~ety_env env tk in
        let (env, tv) = localize ~ety_env env tv in
        (env, Tarraykind (AKmap (tk, tv)))
      | (None, Some _) -> failwith "Invalid array declaration type"
    in
    (env, (r, ty))
  | (r, Tdarray (tk, tv)) ->
    let (env, tk) = localize ~ety_env env tk in
    let (env, tv) = localize ~ety_env env tv in
    let ty = Tarraykind (AKdarray (tk, tv)) in
    (env, (r, ty))
  | (r, Tvarray tv) ->
    let (env, tv) = localize ~ety_env env tv in
    let ty = Tarraykind (AKvarray tv) in
    (env, (r, ty))
  | (r, Tvarray_or_darray tv) ->
    let (env, tv) = localize ~ety_env env tv in
    let ty = Tarraykind (AKvarray_or_darray tv) in
    (env, (r, ty))
  | (r, Tgeneric x) ->
    begin
      match SMap.get x ety_env.substs with
      | Some x_ty -> (env, (Reason.Rinstantiate (fst x_ty, x, r), snd x_ty))
      | None -> (env, (r, Tabstract (AKgeneric x, None)))
    end
  | (r, Toption ty) ->
    let (env, ty) = localize ~ety_env env ty in
    TUtils.union env (MakeType.null r) ty
  | (r, Tlike ty) ->
    let (env, ty) = localize ~ety_env env ty in
    let (env, lty) = TUtils.union env (MakeType.dynamic r) ty in
    (env, lty)
  | (r, Tfun ft) ->
    let (env, ft) =
      localize_ft
        ~ety_env
        ~instantiation:
          { use_pos = ft.ft_pos; use_name = "function"; explicit_targs = [] }
        env
        ft
    in
    (env, (r, Tfun ft))
  | (r, Tapply ((_, x), argl)) when Env.is_typedef x ->
    let (env, argl) = List.map_env env argl (localize ~ety_env) in
    TUtils.expand_typedef ety_env env r x argl
  | (r, Tapply ((p, x), _argl)) when Env.is_enum env x ->
    (* if argl <> [], nastInitCheck would have raised an error *)
    if Typing_defs.has_expanded ety_env x then (
      Errors.cyclic_enum_constraint p;
      (env, (r, Typing_utils.tany env))
    ) else
      let type_expansions = (p, x) :: ety_env.type_expansions in
      let ety_env = { ety_env with type_expansions } in
      let (env, cstr) =
        match Env.get_enum_constraint env x with
        (* If not specified, default bound is arraykey *)
        | None ->
          ( env,
            MakeType.arraykey (Reason.Rimplicit_upper_bound (p, "arraykey")) )
        | Some ty -> localize ~ety_env env ty
      in
      (env, (r, Tabstract (AKnewtype (x, []), Some cstr)))
  | (r, Tapply (((_, cid) as cls), tyl)) ->
    let can_infer_tparams =
      GlobalOptions.InferMissing.can_infer_params
      @@ TypecheckerOptions.infer_missing (Env.get_tcopt env)
    in
    let (env, tyl) =
      if (not (tyl_contains_wildcard tyl)) && List.length tyl <> 0 then
        List.map_env env tyl (localize ~ety_env)
      else
        match Env.get_class env cid with
        | None -> List.map_env env tyl (localize ~ety_env)
        | Some class_info ->
          let tparams = Cls.tparams class_info in
          if List.length tparams <> List.length tyl && can_infer_tparams then
            (* In this case we will infer the missing type parameters *)
            localize_missing_tparams_class env r cls class_info tyl
          else
            localize_tparams ~ety_env env (Reason.to_pos r) tyl tparams
    in
    (env, (r, Tclass (cls, Nonexact, tyl)))
  | (r, Ttuple tyl) ->
    let (env, tyl) = List.map_env env tyl (localize ~ety_env) in
    (env, (r, Ttuple tyl))
  | (r, Taccess (root_ty, ids)) ->
    let (env, root_ty) = localize ~ety_env env root_ty in
    let (env, (expansion_reason, ty)) =
      List.fold ids ~init:(env, root_ty) ~f:(fun (env, root_ty) id ->
          TUtils.expand_typeconst ety_env env root_ty id)
    in
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
        match fst root_ty with
        | Reason.Rexpr_dep_type (_, p, e) -> Reason.Rexpr_dep_type (r, p, e)
        | _ -> r
      in
      Reason.Rtype_access (expand_reason, [(reason, taccess_string)])
    in
    (env, (elaborate_reason expansion_reason, ty))
  | (r, Tshape (shape_kind, tym)) ->
    let (env, tym) = ShapeFieldMap.map_env (localize ~ety_env) env tym in
    (env, (r, Tshape (shape_kind, tym)))
  | (r, Tpu_access (base, sid)) ->
    let (env, base) = localize ~ety_env env base in
    (env, (r, Tpu_access (base, sid)))

and localize_tparams ~ety_env env pos tyl tparams =
  let length = min (List.length tyl) (List.length tparams) in
  let (tyl, tparams) = (List.take tyl length, List.take tparams length) in
  let ((env, _), tyl) =
    List.map2_env (env, ety_env) tyl tparams (localize_tparam pos)
  in
  (env, tyl)

and localize_tparam pos (env, ety_env) ty tparam =
  match ty with
  | (r, Tapply ((_, x), _argl)) when x = SN.Typehints.wildcard ->
    let tparam =
      Typing_enforceability.pessimize_tparam_constraints env tparam
    in
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
      Attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes
    in
    let newable =
      Attributes.mem SN.UserAttributes.uaNewable tp_user_attributes
    in
    let (env, new_name) =
      Env.add_fresh_generic_parameter env name ~reified ~enforceable ~newable
    in
    let ty_fresh = (r, Tabstract (AKgeneric new_name, None)) in
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

and tyl_contains_wildcard tyl =
  List.exists tyl (function
      | (_, Tapply ((_, x), _)) -> x = SN.Typehints.wildcard
      | _ -> false)

(* Recursive localizations of function types do not make judgements about enforceability *)
and localize_possibly_enforced_ty ~ety_env env ety =
  let (env, et_type) = localize ~ety_env env ety.et_type in
  (env, { ety with et_type })

and localize_cstr_ty ~ety_env env ty tp_name =
  let (env, (r, ty_)) = localize ~ety_env env ty in
  let ty = (Reason.Rcstr_on_generics (Reason.to_pos r, tp_name), ty_) in
  (env, ty)

(* For the majority of cases when we localize a function type we instantiate
 * the function's type parameters to be a Tunion wrapped in a Tvar so the
 * type can grow. There are two cases where we do not do this.

 * 1) In Typing_subtype.subtype_method. See the comment for that function for why
 * this is necessary.
 * 2) When the type arguments are explicitly specified, in which case we instantiate
 * the type parameters to the provided types.
 *)
and localize_ft ?instantiation ~ety_env env ft =
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
    let (tparams, _) = ft.ft_tparams in
    match instantiation with
    | Some { explicit_targs; use_name; use_pos } ->
      let default () =
        List.map_env env tparams (fun env tparam ->
            let reason =
              Reason.Rtype_variable_generics
                (use_pos, snd tparam.tp_name, use_name)
            in
            let (env, tvar) = Env.fresh_type_reason env reason in
            Typing_log.log_tparam_instantiation env use_pos tparam tvar;
            (env, tvar))
      in
      let (env, tvarl) =
        if List.is_empty explicit_targs then
          default ()
        else if List.length explicit_targs <> List.length tparams then (
          Errors.expected_tparam
            ~definition_pos:ft.ft_pos
            ~use_pos
            (List.length tparams);
          default ()
        ) else
          let type_argument env decl_ty =
            match decl_ty with
            | (r, Tapply ((_, id), [])) when id = SN.Typehints.wildcard ->
              Env.fresh_type env (Reason.to_pos r)
            | _ -> localize_with_self env decl_ty
          in
          List.map_env env explicit_targs type_argument
      in
      let ft_subst = Subst.make tparams tvarl in
      (env, SMap.union ft_subst ety_env.substs)
    | None ->
      ( env,
        List.fold_left
          tparams
          ~f:begin
               fun subst t -> SMap.remove (snd t.tp_name) subst
             end
          ~init:ety_env.substs )
  in
  let ety_env = { ety_env with substs } in
  let (env, params) =
    List.map_env env ft.ft_params (fun env param ->
        let (env, ty) =
          localize_possibly_enforced_ty ~ety_env env param.fp_type
        in
        (env, { param with fp_type = ty }))
  in
  (* restore reactivity *)
  let env =
    if saved_r <> env_reactivity env then
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
  let (env, tparams) = List.map_env env (fst ft.ft_tparams) localize_tparam in
  let ft_tparams =
    ( tparams,
      if Option.is_some instantiation then
        FTKinstantiated_targs
      else
        FTKtparams )
  in
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
      let env =
        check_tparams_constraints ~use_pos ~ety_env env (fst ft.ft_tparams)
      in
      let env =
        check_where_constraints
          ~in_class:false
          ~use_pos
          ~definition_pos:ft.ft_pos
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
    | (Fellipsis _ | Fstandard (_, _)) as x -> (env, x)
  in
  let (env, ret) = localize_possibly_enforced_ty ~ety_env env ft.ft_ret in
  ( env,
    {
      ft with
      ft_arity = arity;
      ft_params = params;
      ft_ret = ret;
      ft_tparams;
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
 *
 * In fact, the constraint checking isn't done immediately, but rather pushed
 * onto the env.todo list. Typically we haven't resolved types sufficiently
 * (e.g. we have completely unresolved type variables) and so the actual
 * constraint checking is deferred until we have finished checking a
 * function's body.
 *)
and check_tparams_constraints ~use_pos ~ety_env env tparams =
  let check_tparam_constraints env t =
    List.fold_left t.tp_constraints ~init:env ~f:(fun env (ck, ty) ->
        let (env, ty) = localize_cstr_ty ~ety_env env ty t.tp_name in
        match SMap.get (snd t.tp_name) ety_env.substs with
        | Some x_ty ->
          Typing_log.(
            log_with_level env "generics" 1 (fun () ->
                log_types
                  use_pos
                  env
                  [
                    Log_head
                      ( "check_tparams_constraints: add_check_constraint_todo",
                        [Log_type ("ty", ty); Log_type ("x_ty", x_ty)] );
                  ]));
          TGenConstraint.add_check_constraint_todo
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
      let contains_type_access =
        match (ty1, ty2) with
        | ((_, Taccess ((_, Tgeneric _), _)), _)
        | (_, (_, Taccess ((_, Tgeneric _), _))) ->
          true
        | _ -> false
      in
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
        let (env, ty1) = localize ~ety_env env ty1 in
        let (env, ty2) = localize ~ety_env env ty2 in
        TGenConstraint.add_check_where_constraint_todo
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
and localize_with_self env ty = localize env ty ~ety_env:(env_with_self env)

and localize_possibly_enforced_with_self env ety =
  let (env, et_type) = localize_with_self env ety.et_type in
  (env, { ety with et_type })

and localize_hint_with_self env h =
  let h = Decl_hint.hint env.decl_env h in
  localize_with_self env h

and localize_hint ~ety_env env hint =
  let hint_ty = Decl_hint.hint env.decl_env hint in
  localize ~ety_env env hint_ty

and localize_missing_tparams_class env r sid class_ tyl =
  let p = Reason.to_pos r in
  let (env, _, tyl) =
    resolve_type_arguments_and_check_constraints
      ~exact:Nonexact
      ~check_constraints:false
      env
      p
      sid
      (Aast.CI sid)
      (Cls.tparams class_)
      tyl
  in
  let c_ty = (r, Tclass (sid, Nonexact, tyl)) in
  let env = Env.set_tyvar_variance env c_ty in
  let ety_env =
    {
      type_expansions = [];
      this_ty = c_ty;
      substs = Subst.make (Cls.tparams class_) tyl;
      from_class = Some (Aast.CI sid);
    }
  in
  let env =
    check_tparams_constraints ~use_pos:p ~ety_env env (Cls.tparams class_)
  in
  let env =
    check_where_constraints
      ~in_class:true
      ~use_pos:p
      ~definition_pos:(Cls.pos class_)
      ~ety_env
      env
      (Cls.where_constraints class_)
  in
  (env, tyl)

(* If there are no explicit type arguments then generate fresh type variables
* for all of them. Otherwise, check the arity, and use the explicit types. *)
and resolve_type_argument env hint =
  (* For explicit type arguments we support a wildcard syntax `_` for which
   * Hack will generate a fresh type variable *)
  match hint with
  | (r, Tapply ((_, id), [])) when id = SN.Typehints.wildcard ->
    Env.fresh_type env (Reason.to_pos r)
  | _ -> localize_with_self env hint

and resolve_type_argument_hint env hint =
  Decl_hint.hint env.decl_env hint |> resolve_type_argument env

and resolve_type_arguments env p class_id tparaml hintl =
  let length_hintl = List.length hintl in
  let length_tparaml = List.length tparaml in
  if length_hintl <> length_tparaml then
    List.map_env env tparaml (fun env tparam ->
        let (env, tvar) =
          Env.fresh_type_reason
            env
            (Reason.Rtype_variable_generics
               (p, snd tparam.tp_name, Utils.strip_ns (snd class_id)))
        in
        Typing_log.log_tparam_instantiation env p tparam tvar;
        (env, tvar))
  else
    List.map_env env hintl resolve_type_argument

(* Do all of the above, and also check any constraints associated with the type parameters.
 *)
and resolve_type_arguments_and_check_constraints
    ~exact ~check_constraints env p class_id from_class tparaml hintl =
  let (env, type_argl) = resolve_type_arguments env p class_id tparaml hintl in
  let this_ty =
    (Reason.Rwitness (fst class_id), Tclass (class_id, exact, type_argl))
  in
  let env =
    if check_constraints then
      let ety_env =
        {
          type_expansions = [];
          this_ty;
          substs = Subst.make tparaml type_argl;
          from_class = Some from_class;
        }
      in
      check_tparams_constraints ~use_pos:p ~ety_env env tparaml
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
      env ({ tp_name = (pos, name); tp_constraints = cstrl; _ } : decl_tparam)
      =
    let tparam_ty = (Reason.Rwitness pos, Tabstract (AKgeneric name, None)) in
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

(* Ensure that types are equivalent i.e. subtypes of each other *)
let unify_decl env ty1 ty2 on_error =
  let (env, ty1) = localize_with_self env ty1 in
  let (env, ty2) = localize_with_self env ty2 in
  ignore (TUtils.sub_type env ty2 ty1 on_error);
  ignore (TUtils.sub_type env ty1 ty2 on_error)

let sub_type_decl env ty1 ty2 on_error =
  let (env, ty1) = localize_with_self env ty1 in
  let (env, ty2) = localize_with_self env ty2 in
  ignore (TUtils.sub_type env ty1 ty2 on_error)

(*****************************************************************************
 * External API
 *****************************************************************************)

let localize_with_self env ty =
  let ty = Typing_enforceability.pessimize_type env ty in
  localize_with_self env ty

let localize ~ety_env env ty =
  let ty = Typing_enforceability.pessimize_type env ty in
  localize ~ety_env env ty

let localize_ft ?instantiation ~ety_env env ft =
  let ft = Typing_enforceability.pessimize_fun_type env ft in
  let instantiation =
    Option.map instantiation ~f:(fun i ->
        let explicit_targs =
          Typing_enforceability.pessimize_targs
            env
            i.explicit_targs
            (fst ft.ft_tparams)
        in
        { i with explicit_targs })
  in
  localize_ft ?instantiation ~ety_env env ft

let localize_generic_parameters_with_bounds ~ety_env env tparams =
  let tparams : decl_tparam list =
    List.map
      tparams
      ~f:(Typing_enforceability.pessimize_tparam_constraints env)
  in
  localize_generic_parameters_with_bounds ~ety_env env tparams

let check_tparams_constraints ~use_pos ~ety_env env tparams =
  let tparams =
    List.map
      ~f:(Typing_enforceability.pessimize_tparam_constraints env)
      tparams
  in
  check_tparams_constraints ~use_pos ~ety_env env tparams

(* TODO(T46211387) make the rest of the API pessimize *)

let () = TUtils.localize_with_self_ref := localize_with_self
