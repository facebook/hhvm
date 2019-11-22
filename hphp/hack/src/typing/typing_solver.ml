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
module Reason = Typing_reason
module Env = Typing_env
module Inter = Typing_intersection
module ITySet = Internal_type_set
module Union = Typing_union
module TL = Typing_logic
module TUtils = Typing_utils
module Cls = Decl_provider.Class
module TySet = Typing_set
module MakeType = Typing_make_type

let use_bind_to_equal_bound = ref true

let tvenv_to_prop tvenv =
  let props_per_tvar =
    IMap.mapi
      (fun id tyvar_info ->
        match tyvar_info with
        | LocalTyvar { lower_bounds; upper_bounds; _ } ->
          let tyvar = (Reason.Rnone, Tvar id) in
          let lower_bounds = ITySet.elements lower_bounds in
          let upper_bounds = ITySet.elements upper_bounds in
          let lower_bounds_props =
            List.map
              ~f:(fun ty -> TL.IsSubtype (ty, LoclType tyvar))
              lower_bounds
          in
          (* If an upper bound of variable n1 is a `Tvar n2`,
        then we have already added "Tvar n1 <: Tvar n2" when traversing
        lower bounds of n2, so we can filter out upper bounds that are Tvars. *)
          let can_be_removed = function
            | LoclType (_, Tvar n) ->
              begin
                match IMap.find_opt n tvenv with
                | Some _ -> true
                | None -> false
              end
            | _ -> false
          in
          let upper_bounds =
            List.filter ~f:(fun ty -> not (can_be_removed ty)) upper_bounds
          in
          let upper_bounds_props =
            List.map
              ~f:(fun ty -> TL.IsSubtype (LoclType tyvar, ty))
              upper_bounds
          in
          TL.conj_list (lower_bounds_props @ upper_bounds_props)
        | GlobalTyvar -> TL.conj_list [])
      tvenv
  in
  let (_ids, props) = List.unzip (IMap.bindings props_per_tvar) in
  TL.conj_list props

let env_to_prop env = TL.conj (tvenv_to_prop env.tvenv) env.subtype_prop

let log_prop env =
  let filename = Pos.filename (Pos.to_absolute env.function_pos) in
  if Str.string_match (Str.regexp {|.*\.hhi|}) filename 0 then
    ()
  else
    let prop = env_to_prop env in
    ( if TypecheckerOptions.log_inference_constraints (Env.get_tcopt env) then
      let p_as_string = Typing_print.subtype_prop env prop in
      let pos = Pos.string (Pos.to_absolute env.function_pos) in
      let size = TL.size prop in
      let n_disj = TL.n_disj prop in
      let n_conj = TL.n_conj prop in
      TypingLogger.InferenceCnstr.log p_as_string ~pos ~size ~n_disj ~n_conj );
    if (not (Errors.currently_has_errors ())) && not (TL.is_valid prop) then
      Typing_log.log_prop
        1
        env.function_pos
        "There are remaining unsolved constraints!"
        env
        prop

(* Given a type ty, replace any covariant or contravariant components of the type
 * with fresh type variables. Components replaced include
 *   covariant key and element types for tuples, arrays, and shapes
 *   covariant return type and contravariant parameter types for function types
 *   co- and contra-variant parameters to classish types and newtypes
 * Note that the variance of type variables is set explicitly to be invariant
 * because we only use this function on the lower or upper bounds of an invariant
 * type variable.
 *
 * Also note that freshening lifts through unions and nullables.
 *
 * Example 1: the type
 *   ?dict<t1,t2>
 * will be transformed to
 *   ?dict<#1,#2>
 *
 * Example 2: the contravariant/invariant type
 *   ContraInv<t1,t2>
 * will be transformed to
 *   Contra<#1,t2>
 * leaving the invariant component alone.
 *)
let rec freshen_inside_ty env ty =
  let default () = (env, ty) in
  let (env, (r, ty_)) = Env.expand_type env ty in
  match ty_ with
  | Tany _
  | Tnonnull
  | Terr
  | Tdynamic
  | Tobject
  | Tprim _
  | Tanon _
  | Tabstract (_, None) ->
    default ()
  (* Nullable is covariant *)
  | Toption ty ->
    let (env, ty) = freshen_inside_ty env ty in
    (env, (r, Toption ty))
  | Tunion tyl ->
    let (env, tyl) = List.map_env env tyl freshen_inside_ty in
    (env, (r, Tunion tyl))
  | Tintersection tyl ->
    let (env, tyl) = List.map_env env tyl freshen_inside_ty in
    Inter.intersect_list env r tyl
  (* Tuples are covariant *)
  | Ttuple tyl ->
    let (env, tyl) = List.map_env env tyl freshen_ty in
    (env, (r, Ttuple tyl))
  (* Shape data is covariant *)
  | Tshape (shape_kind, fdm) ->
    let (env, fdm) = ShapeFieldMap.map_env freshen_ty env fdm in
    (env, (r, Tshape (shape_kind, fdm)))
  (* Functions are covariant in return type, contravariant in parameter types *)
  | Tfun ft ->
    let (env, ft_ret) = freshen_possibly_enforced_ty env ft.ft_ret in
    let (env, ft_params) =
      List.map_env env ft.ft_params (fun env p ->
          let (env, fp_type) = freshen_possibly_enforced_ty env p.fp_type in
          (env, { p with fp_type }))
    in
    (env, (r, Tfun { ft with ft_ret; ft_params }))
  | Tabstract (AKnewtype (name, tyl), tyopt) ->
    begin
      match Env.get_typedef env name with
      | None -> default ()
      | Some td ->
        let variancel = List.map td.td_tparams (fun t -> t.tp_variance) in
        let (env, tyl) = freshen_tparams env variancel tyl in
        (env, (r, Tabstract (AKnewtype (name, tyl), tyopt)))
    end
  | Tabstract _ -> default ()
  | Tclass ((p, cid), e, tyl) ->
    begin
      match Env.get_class env cid with
      | None -> default ()
      | Some cls ->
        let variancel = List.map (Cls.tparams cls) (fun t -> t.tp_variance) in
        let (env, tyl) = freshen_tparams env variancel tyl in
        (env, (r, Tclass ((p, cid), e, tyl)))
    end
  | Tarraykind ak ->
    begin
      match ak with
      | AKempty -> default ()
      | AKvarray ty ->
        let (env, ty) = freshen_ty env ty in
        (env, (r, Tarraykind (AKvarray ty)))
      | AKvarray_or_darray (ty1, ty2) ->
        let (env, ty1) = freshen_ty env ty1 in
        let (env, ty2) = freshen_ty env ty2 in
        (env, (r, Tarraykind (AKvarray_or_darray (ty1, ty2))))
      | AKdarray (ty1, ty2) ->
        let (env, ty1) = freshen_ty env ty1 in
        let (env, ty2) = freshen_ty env ty2 in
        (env, (r, Tarraykind (AKdarray (ty1, ty2))))
    end
  | Tvar _ -> default ()
  | Tpu _
  | Tpu_access _ ->
    (* TODO(T36532263) suggested by Catherine, might be updated next *)
    default ()

and freshen_ty env ty =
  Env.fresh_invariant_type_var env (Reason.to_pos (fst ty))

and freshen_possibly_enforced_ty env ety =
  let (env, et_type) = freshen_ty env ety.et_type in
  (env, { ety with et_type })

and freshen_tparams env variancel tyl =
  match (variancel, tyl) with
  | ([], []) -> (env, [])
  | (variance :: variancel, ty :: tyl) ->
    let (env, tyl) = freshen_tparams env variancel tyl in
    let (env, ty) =
      if Ast_defs.(equal_variance variance Invariant) then
        (env, ty)
      else
        freshen_ty env ty
    in
    (env, ty :: tyl)
  | _ -> (env, tyl)

let var_occurs_in_ty env var ty =
  let finder =
    object (this)
      inherit [env * bool] Type_visitor.locl_type_visitor as super

      method! on_tvar (env, occurs) r v =
        let (env, ty) = Env.expand_var env r v in
        match ty with
        | (_, Tvar v) -> (env, Ident.equal v var)
        | ty -> this#on_type (env, occurs) ty

      method! on_type (env, occurs) ty =
        if occurs then
          (env, occurs)
        else
          super#on_type (env, occurs) ty
    end
  in
  finder#on_type (env, false) ty

let bind env var ty =
  Env.log_env_change "bind" env
  @@
  (* If there has been a use of this type variable that led to an "unknown type"
   * error (e.g. method invocation), then record this in the reason info. We
   * can make use of this for linters and code mods that suggest annotations *)
  let ty =
    if Env.get_tyvar_eager_solve_fail env var then
      (Reason.Rsolve_fail (Reason.to_pos (fst ty)), snd ty)
    else
      ty
  in
  (* Update the variance *)
  let env = Env.update_variance_after_bind env var ty in
  (* Unify the variable *)
  let (env, var_occurs_in_ty) = var_occurs_in_ty env var ty in
  let env =
    if var_occurs_in_ty then (
      Errors.unification_cycle
        (Reason.to_pos (fst ty))
        Typing_print.(with_blank_tyvars (fun () -> full_rec env var ty));
      Env.add env var (fst ty, Terr)
    ) else
      Env.add env var ty
  in
  let env = Typing_type_simplifier.simplify_occurrences env var in
  (* Remove the variable from the environment *)
  let env = Env.remove_tyvar env var in
  env

(** If a type variable appear in one of its own lower bounds under a combination
of unions and intersections, it can be simplified away from this lower bound by
replacing any of its occurences with nothing.
E.g.
- if #1 has lower bound (#1 | A), the lower bound can be simplified to
(nothing | A) = A.
- if #1 has lower bound (#1 & A), the lower bound can be simplified to
(nothing & A) = nothing.
*)
let remove_tyvar_from_lower_bound env var r lower_bound =
  let is_nothing = ty_equal (MakeType.nothing r) in
  let rec remove env ty =
    let (env, ty) = Env.expand_type env ty in
    match ty with
    | (_, Tvar v) when Ident.equal v var -> (env, MakeType.nothing r)
    | (r, Toption ty) ->
      let (env, ty) = remove env ty in
      (env, MakeType.nullable_locl r ty)
    | (r, Tunion tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:remove in
      let tyl = List.filter tyl ~f:(fun ty -> not (is_nothing ty)) in
      (env, MakeType.union r tyl)
    | (r, Tintersection tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:remove in
      let ty =
        if List.exists tyl ~f:is_nothing then
          MakeType.nothing r
        else
          MakeType.intersection r tyl
      in
      (env, ty)
    | _ -> (env, ty)
  and remove_i env ty =
    match ty with
    | LoclType ty ->
      let (env, ty) = remove env ty in
      (env, LoclType ty)
    | _ -> (env, ty)
  in
  remove_i env lower_bound

let remove_tyvar_from_lower_bounds env var r lower_bounds =
  ITySet.fold
    (fun lower_bound (env, acc) ->
      let (env, lower_bound) =
        remove_tyvar_from_lower_bound env var r lower_bound
      in
      (env, ITySet.add lower_bound acc))
    lower_bounds
    (env, ITySet.empty)

(** If a type variable appear in one of its own upper bounds under a combination
of unions and intersections, it can be simplified away from this upper bound by
replacing any of its occurences with mixed.
E.g.
- if #1 has upper bound (#1 & A), the upper bound can be simplified to
(mixed & A) = A.
- if #1 has upper bound (#1 | A), the upper bound can be simplified to
(mixed | A) = mixed
*)
let remove_tyvar_from_upper_bound env var r upper_bound =
  let is_mixed ty =
    ty_equal ty (MakeType.mixed r) || ty_equal ty (MakeType.intersection r [])
  in
  let rec remove env ty =
    let (env, ty) = Env.expand_type env ty in
    match ty with
    | (_, Tvar v) when Ident.equal v var -> (env, MakeType.mixed r)
    | (r, Toption ty) ->
      let (env, ty) = remove env ty in
      (env, MakeType.nullable_locl r ty)
    | (r, Tunion tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:remove in
      let ty =
        if List.exists tyl ~f:is_mixed then
          MakeType.mixed r
        else
          MakeType.union r tyl
      in
      (env, ty)
    | (r, Tintersection tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:remove in
      let tyl = List.filter tyl ~f:(fun ty -> not (is_mixed ty)) in
      (env, MakeType.intersection r tyl)
    | _ -> (env, ty)
  and remove_i env ty =
    match ty with
    | LoclType ty ->
      let (env, ty) = remove env ty in
      (env, LoclType ty)
    | _ -> (env, ty)
  in
  remove_i env upper_bound

let remove_tyvar_from_upper_bounds env var r upper_bounds =
  ITySet.fold
    (fun upper_bound (env, acc) ->
      let (env, upper_bound) =
        remove_tyvar_from_upper_bound env var r upper_bound
      in
      (env, ITySet.add upper_bound acc))
    upper_bounds
    (env, ITySet.empty)

(** Remove a type variable from its upper and lower bounds. More precisely,
if a type variable appears in one of its bounds under any combination of unions
and intersections, it can be simplified away from the bound.
For example,
- if #1 has lower bound (#1 | A), the lower bound can be simplified to A
- if #1 has upper bound (#1 | B), the upper bound can be simplified to mixed
and dually for intersections.
*)
let remove_tyvar_from_bounds env r var =
  Env.log_env_change "remove_tyvar_from_bounds" ~level:3 env
  @@
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let (env, lower_bounds) =
    remove_tyvar_from_lower_bounds env var r lower_bounds
  in
  let (env, upper_bounds) =
    remove_tyvar_from_upper_bounds env var r upper_bounds
  in
  let env = Env.set_tyvar_lower_bounds env var lower_bounds in
  let env = Env.set_tyvar_upper_bounds env var upper_bounds in
  env

let filter_locl_types types =
  ITySet.fold
    (fun ty types ->
      match ty with
      | LoclType ty -> TySet.add ty types
      | _ -> types)
    types
    TySet.empty

(* Solve type variable var by assigning it to the union of its lower bounds.
 * If freshen=true, first freshen the covariant and contravariant components of
 * the bounds.
 *)
let bind_to_lower_bound ~freshen env r var lower_bounds on_error =
  Env.log_env_change "bind_to_lower_bound" env
  @@
  if ITySet.exists is_constraint_type lower_bounds then
    env
  else
    let lower_bounds = filter_locl_types lower_bounds in
    let (env, ty) = TUtils.union_list env r (TySet.elements lower_bounds) in
    (* Freshen components of the types in the union wrt their variance.
     * For example, if we have
     *   Cov<C>, Contra<D> <: v
     * then we actually construct the union
     *   Cov<#1> | Contra<#2> with C <: #1 and #2 <: D
     *)
    let (env, ty) =
      if freshen then
        let (env, newty) = freshen_inside_ty env ty in
        let env = Typing_utils.sub_type env ty newty on_error in
        (env, newty)
      else
        (env, ty)
    in
    (* If any of the components of the union are type variables, then remove
  * var from their upper bounds. Why? Because if we construct
  *   v1 , ... , vn , t <: var
  * for type variables v1, ..., vn and non-type variable t
  * then necessarily we must have var as an upper bound on each of vi
  * so after binding var we end up with redundant bounds
  *   vi <: v1 | ... | vn | t
  *)
    let env =
      TySet.fold
        (fun ty env ->
          match Env.expand_type env ty with
          | (env, (_, Tvar v)) when not (Env.is_global_tyvar env v) ->
            Env.remove_tyvar_upper_bound env v var
          | (env, _) -> env)
        lower_bounds
        env
    in
    (* Now actually make the assignment var := ty, and remove var from tvenv *)
    bind env var ty

let bind_to_upper_bound env r var upper_bounds =
  Env.log_env_change "bind_to_upper_bound" env
  @@
  if ITySet.exists is_constraint_type upper_bounds then
    env
  else
    let upper_bounds = filter_locl_types upper_bounds in
    let (env, ty) = Inter.intersect_list env r (TySet.elements upper_bounds) in
    (* If ty is a variable (in future, if any of the types in the list are variables),
    * then remove var from their lower bounds. Why? Because if we construct
    *   var <: v1 , ... , vn , t
    * for type variables v1 , ... , vn and non-type variable t
    * then necessarily we must have var as a lower bound on each of vi
    * so after binding var we end up with redundant bounds
    *   v1 & ... & vn & t <: vi
    *)
    let env =
      match Env.expand_type env ty with
      | (env, (_, Tvar v)) when not (Env.is_global_tyvar env v) ->
        Env.remove_tyvar_lower_bound env v var
      | (env, _) -> env
    in
    bind env var ty

(* Is the outer skeleton of the types the same (everything that isn't a nested type)?
 * e.g. C<string> same as  C<int>,
 *      dict<string,bool> same as dict<int,string>
 *      shape('a' => int) same as shape('a' => bool)
 * but shape('a' => int) not same as shape('b' => int)
 * shape(?'a' => int) not same as shape('a' => int)
 *
 * Expected invariant:
 *   if ty_equal_shallow ty1 ty2
 *   then freshen_inside_ty ty1 is identical to freshen_inside_ty ty2
 *   up to a choice of fresh type variables.
 * (Or: there is some instantiation of the fresh type variables that
 * makes freshen_inside_ty ty1 and freshen_inside_ty ty2 the same.)
 *)
let ty_equal_shallow env ty1 ty2 =
  let (env, ty1) = Env.expand_type env ty1 in
  let (_env, ty2) = Env.expand_type env ty2 in
  match (snd ty1, snd ty2) with
  | (Tany _, Tany _)
  | (Tnonnull, Tnonnull)
  | (Terr, Terr)
  | (Tdynamic, Tdynamic)
  | (Tobject, Tobject)
  | (Ttuple _, Ttuple _)
  | (Tarraykind (AKvarray _), Tarraykind (AKvarray _))
  | (Tarraykind (AKvarray_or_darray _), Tarraykind (AKvarray_or_darray _))
  | (Tarraykind (AKdarray _), Tarraykind (AKdarray _)) ->
    true
  | (Tprim p1, Tprim p2) -> Aast_defs.equal_tprim p1 p2
  | (Tclass (x_sub, exact_sub, _), Tclass (x_super, exact_super, _)) ->
    String.equal (snd x_sub) (snd x_super) && equal_exact exact_sub exact_super
  | (Tfun fty1, Tfun fty2) ->
    Bool.equal fty1.ft_is_coroutine fty2.ft_is_coroutine
    && equal_locl_fun_arity fty1.ft_arity fty2.ft_arity
    && equal_reactivity fty1.ft_reactive fty2.ft_reactive
    && Bool.equal fty1.ft_return_disposable fty2.ft_return_disposable
    && Option.equal equal_param_mutability fty1.ft_mutability fty2.ft_mutability
  | (Tshape (shape_kind1, fdm1), Tshape (shape_kind2, fdm2)) ->
    equal_shape_kind shape_kind1 shape_kind2
    && List.equal
         ~equal:(fun (k1, v1) (k2, v2) ->
           Ast_defs.ShapeField.equal k1 k2
           && Bool.equal v1.sft_optional v2.sft_optional)
         (ShapeFieldMap.elements fdm1)
         (ShapeFieldMap.elements fdm2)
  | (Tabstract (AKnewtype (n1, _), _), Tabstract (AKnewtype (n2, _), _)) ->
    String.equal n1 n2
  | (Tabstract (ak1, _), Tabstract (ak2, _)) -> equal_abstract_kind ak1 ak2
  | _ -> false

let union_any_if_any_in_lower_bounds env ty lower_bounds =
  let r = Reason.none in
  let any = LoclType (r, Typing_defs.make_tany ())
  and err = LoclType (r, Terr) in
  let (env, ty) =
    match ITySet.find_opt any lower_bounds with
    | Some (LoclType any) -> Union.union env ty any
    | _ -> (env, ty)
  in
  let (env, ty) =
    match ITySet.find_opt err lower_bounds with
    | Some (LoclType err) -> Union.union env ty err
    | _ -> (env, ty)
  in
  (env, ty)

let try_bind_to_equal_bound ~freshen env r var on_error =
  if (not !use_bind_to_equal_bound) || Env.tyvar_is_solved env var then
    env
  else
    Env.log_env_change "bind_to_equal_bound" env
    @@
    let env = remove_tyvar_from_bounds env r var in
    let expand_all tyset =
      ITySet.map
        (fun ty ->
          let (_, ty) = Env.expand_internal_type env ty in
          ty)
        tyset
    in
    let tyvar_info = Env.get_tyvar_info env var in
    let lower_bounds = expand_all tyvar_info.lower_bounds in
    let upper_bounds = expand_all tyvar_info.upper_bounds in
    let equal_bounds = ITySet.inter lower_bounds upper_bounds in
    let r = Reason.none in
    let any = LoclType (r, Typing_defs.make_tany ())
    and err = LoclType (r, Terr) in
    let equal_bounds = equal_bounds |> ITySet.remove any |> ITySet.remove err in
    match ITySet.choose_opt equal_bounds with
    | Some (LoclType ty) ->
      let (env, ty) = union_any_if_any_in_lower_bounds env ty lower_bounds in
      bind env var ty
    | Some (ConstraintType _)
    | None ->
      if not freshen then
        env
      else
        ITySet.fold
          (fun upper_bound env ->
            ITySet.fold
              (fun lower_bound env ->
                if Env.tyvar_is_solved env var then
                  let (env, ty) = Env.expand_var env r var in
                  let ty = LoclType ty in
                  let env =
                    Typing_utils.sub_type_i env lower_bound ty on_error
                  in
                  let env =
                    Typing_utils.sub_type_i env ty upper_bound on_error
                  in
                  env
                else
                  match (lower_bound, upper_bound) with
                  | (LoclType lower_bound, LoclType upper_bound)
                    when ty_equal_shallow env lower_bound upper_bound ->
                    let (env, ty) = freshen_inside_ty env lower_bound in
                    let env =
                      Typing_utils.sub_type env lower_bound ty on_error
                    in
                    let env =
                      Typing_utils.sub_type env ty upper_bound on_error
                    in
                    let (env, ty) =
                      union_any_if_any_in_lower_bounds env ty lower_bounds
                    in
                    bind env var ty
                  | _ -> env)
              lower_bounds
              env)
          upper_bounds
          env

(* Always solve a type variable.
We are here because we eagerly solve a type variable to see
whether certain operations are allowed on the type (e.g. a  method call),
or because we are at the end of a function body and we solve
the remaining invariant type variables.
Therefore, we always force to the lower bounds (even contravariant variables),
because it produces a "more specific" type, which is more likely to support
the operation which we eagerly solved for in the first place.
*)
let rec always_solve_tyvar ~freshen env r var on_error =
  (* If there is a type that is both a lower and upper bound, force to that type *)
  let env = try_bind_to_equal_bound ~freshen env r var on_error in
  if Env.tyvar_is_solved env var then
    env
  else
    let tyvar_info = Env.get_tyvar_info env var in
    let r =
      if Reason.(equal r Rnone) then
        Reason.Rwitness tyvar_info.tyvar_pos
      else
        r
    in
    let env =
      bind_to_lower_bound ~freshen env r var tyvar_info.lower_bounds on_error
    in
    let (env, ety) = Env.expand_var env r var in
    match ety with
    | (_, Tvar var') when not (Ident.equal var var') ->
      always_solve_tyvar ~freshen env r var on_error
    | _ -> env

(* Use the variance information about a type variable to force a solution.
 *   (1) If the type variable is bounded by t1, ..., tn <: v and it appears only
 *   covariantly or not at all in the expression type then
 *   we can minimize it i.e. set v := t1 | ... | tn.
 *   (2) If the type variable is bounded by v <: t1, ..., tn and it appears only
 *   contravariantly in the expression type then we can maximize it. Ideally we
 *   would use an intersection v := t1 & ... & tn so for now we only solve
 *   if there is a single upper bound.
 *   (3) If the type variable is bounded by t1, ..., tm <: v <: u1, ..., un and
 *   u1 & ... & un == t1 | ... | tm then we can set v to either of these
 *   equivalent types.  Because we don't have intersections, for now we check if
 *   there exist i, j such that uj <: ti, which implies ti == uj and allows
 *   us to set v := ti.
 *)
let solve_tyvar_wrt_variance env r var on_error =
  Typing_log.(
    log_with_level env "prop" 2 (fun () ->
        log_types
          (Reason.to_pos r)
          env
          [
            Log_head
              ( Printf.sprintf "Typing_subtype.solve_tyvar_wrt_variance #%d" var,
                [] );
          ]));

  (* If there is a type that is both a lower and upper bound, force to that type *)
  let env = try_bind_to_equal_bound ~freshen:false env r var on_error in
  if Env.tyvar_is_solved env var then
    env
  else
    let tyvar_info = Env.get_tyvar_info env var in
    let r =
      if Reason.equal r Reason.Rnone then
        Reason.Rwitness tyvar_info.tyvar_pos
      else
        r
    in
    let lower_bounds = tyvar_info.lower_bounds
    and upper_bounds = tyvar_info.upper_bounds in
    match
      (tyvar_info.appears_covariantly, tyvar_info.appears_contravariantly)
    with
    | (true, false)
    | (false, false) ->
      (* As in Local Type Inference by Pierce & Turner, if type variable does
       * not appear at all, or only appears covariantly, solve to lower bound
       *)
      bind_to_lower_bound ~freshen:false env r var lower_bounds on_error
    | (false, true) ->
      (* As in Local Type Inference by Pierce & Turner, if type variable
       * appears only contravariantly, solve to upper bound
       *)
      bind_to_upper_bound env r var upper_bounds
    | (true, true) ->
      (* Not ready to solve yet! *)
      env

let solve_tyvar_wrt_variance env r var on_error =
  let rec solve_until_concrete_ty env v =
    let env = solve_tyvar_wrt_variance env r v on_error in
    let (env, ety) = Env.expand_var env r v in
    match ety with
    | (_, Tvar v') when not (Ident.equal v v') -> solve_until_concrete_ty env v'
    | _ -> env
  in
  solve_until_concrete_ty env var

(* Force solve all type variables in the environment *)
let solve_all_unsolved_tyvars env on_error =
  let env =
    Env.log_env_change "solve_all_unsolved_tyvars" env
    @@ IMap.fold
         (fun tyvar _ env ->
           always_solve_tyvar ~freshen:false env Reason.Rnone tyvar on_error)
         env.tvenv
         env
  in
  log_prop env;
  env

let solve_all_unsolved_tyvars_gi env make_on_error =
  IMap.fold
    (fun tyvar _ env ->
      always_solve_tyvar
        ~freshen:false
        env
        Reason.Rnone
        tyvar
        (make_on_error tyvar))
    env.tvenv
    env

let unsolved_invariant_tyvars_under_union_and_intersection env ty =
  let rec find_tyvars (env, tyvars) ty =
    let (env, ty) = Env.expand_type env ty in
    match ty with
    | (r, Tvar v) ->
      let tyvars =
        if
          Env.get_tyvar_appears_invariantly env v
          || TypecheckerOptions.new_inference_lambda (Env.get_tcopt env)
        then
          (r, v) :: tyvars
        else
          tyvars
      in
      (env, tyvars)
    | (_, Toption ty) -> find_tyvars (env, tyvars) ty
    | (_, Tunion tyl)
    | (_, Tintersection tyl) ->
      List.fold tyl ~init:(env, tyvars) ~f:find_tyvars
    | ( _,
        ( Terr | Tany _ | Tdynamic | Tnonnull | Tprim _ | Tclass _ | Tobject
        | Tabstract _ | Tarraykind _ | Ttuple _ | Tshape _ | Tfun _ | Tanon _
        | Tpu _ | Tpu_access _ ) ) ->
      (env, tyvars)
  in
  find_tyvars (env, []) ty

(* Expand an already-solved type variable, and solve an unsolved type variable
 * by binding it to the union of its lower bounds, with covariant and contravariant
 * components of the type suitably "freshened". For example,
 *    vec<C> <: #1
 * will be solved by
 *    #1 := vec<#2>  where C <: #2
 *)
let expand_type_and_solve env ~description_of_expected p ty on_error =
  let (env, unsolved_invariant_tyvars) =
    unsolved_invariant_tyvars_under_union_and_intersection env ty
  in
  let (env', ety) =
    Typing_utils.simplify_unions env ty ~on_tyvar:(fun env r v ->
        let env = always_solve_tyvar ~freshen:true env r v on_error in
        Env.expand_var env r v)
  in
  let (env', ety) = Env.expand_type env' ety in
  match (unsolved_invariant_tyvars, ety) with
  | (_ :: _, (_, Tunion [])) ->
    let env =
      List.fold unsolved_invariant_tyvars ~init:env ~f:(fun env (r, v) ->
          Errors.unknown_type
            description_of_expected
            p
            (Reason.to_string "It is unknown" r);
          Env.set_tyvar_eager_solve_fail env v)
    in
    (env, (Reason.Rsolve_fail p, TUtils.terr env))
  | _ -> (env', ety)

let expand_type_and_solve_eq env ty on_error =
  (fun (env, ty) -> Env.expand_type env ty)
  @@ Typing_utils.simplify_unions env ty ~on_tyvar:(fun env r v ->
         let env = try_bind_to_equal_bound ~freshen:true env r v on_error in
         Env.expand_var env r v)

(* When applied to concrete types (typically classes), the `widen_concrete_type`
 * function should produce the largest supertype that is valid for an operation.
 * For example, if we have an expression $x->f for $x:v and exact C <: v, then
 * we widen `exact C` to `B`, if `B` is the base class from which `C` inherits
 * field f.
 *
 * The `widen` function extends this to nullables and abstract types.
 * General unions have been dealt with already.
 *)
let widen env widen_concrete_type ty =
  let rec widen env ty =
    let (env, ty) = Env.expand_type env ty in
    match ty with
    | (r, Tunion tyl) -> widen_all env r tyl
    | (r, Toption ty) -> widen_all env r [(r, Tprim Aast.Tnull); ty]
    (* Don't widen the `this` type, because the field type changes up the hierarchy
     * so we lose precision
     *)
    | (_, Tabstract (AKdependent DTthis, _)) -> (env, ty)
    (* For other abstract types, just widen to the bound, if possible *)
    | (_, Tabstract (_, Some ty)) -> widen env ty
    | _ ->
      begin
        match widen_concrete_type env ty with
        | (env, Some ty) -> (env, ty)
        | (env, None) -> (env, (Reason.none, Tunion []))
      end
  and widen_all env r tyl =
    let (env, tyl) = List.fold_map tyl ~init:env ~f:widen in
    Typing_union.union_list env r tyl
  in
  widen env ty

let is_nothing env ty =
  Typing_utils.is_sub_type_ignore_generic_params env ty (Reason.none, Tunion [])

(* Using the `widen_concrete_type` function to compute an upper bound,
 * narrow the constraints on a type that are valid for an operation.
 * For example, if we have an expression $x->f for $x:#1 and exact C <: #1, then
 * we can add #1 <: B if C inherits the property f from base class B.
 * Likewise, if we have an expression $x[3] for $x:#2 and vec<string> <: #2, then
 * we can add #2 <: KeyedCollection<int,#3> because that is the largest type
 * consistent with vec for which indexing is valid.
 *
 * Note that if further information arises on the type variable, then
 * this approach is not complete. For example, we might have an unrelated
 * exact A <: #1, for which A does not inherit from base class B.
 *
 * But in general, narrowing is a useful technique for delaying the complete
 * solving of a constraint whilst supporting checking of operations such as
 * member access and array indexing.
 *
 * The optional `default` parameter is used to solve a type variable
 * if `widen_concrete_type` does not produce a result.
 *)
let expand_type_and_narrow
    env ?default ~description_of_expected widen_concrete_type p ty on_error =
  (fun (env, ty) -> Env.expand_type env ty)
  @@
  let (env, ty) = expand_type_and_solve_eq env ty on_error in
  (* Deconstruct the type into union elements (if it's a union). For variables,
   * take the lower bounds. If there are no variables, then we have a concrete
   * type so just return expanded type
   *)
  let has_tyvar = ref false in
  let seen_tyvars = ref ISet.empty in
  (* Simplify unions in ty, but when we encounter a type variable in the process,
  recursively replace it with the union of its lower bounds, effectively getting
  rid of all unsolved type variables in the union. *)
  let (env, concretized_ty) =
    Typing_union.simplify_unions env ty ~on_tyvar:(fun env r v ->
        has_tyvar := true;
        if ISet.mem v !seen_tyvars then
          (env, (r, Tunion []))
        else
          let () = seen_tyvars := ISet.add v !seen_tyvars in
          let lower_bounds =
            TySet.elements
            @@ filter_locl_types
            @@ Env.get_tyvar_lower_bounds env v
          in
          Typing_union.union_list env r lower_bounds)
  in
  if not !has_tyvar then
    (env, ty)
  else
    let (env, widened_ty) = widen env widen_concrete_type concretized_ty in
    let widened_ty =
      match (is_nothing env widened_ty, default) with
      | (true, Some t) -> t
      | _ -> widened_ty
    in
    (* We really don't want to just guess `nothing` if none of the types can be widened *)
    if
      is_nothing env widened_ty
      (* Default behaviour is currently to force solve *)
    then
      expand_type_and_solve env ~description_of_expected p ty on_error
    else
      Errors.try_
        (fun () ->
          let env = Typing_utils.sub_type env ty widened_ty on_error in
          (env, widened_ty))
        (fun _ ->
          if Option.is_some default then
            (env, widened_ty)
          else
            expand_type_and_solve env ~description_of_expected p ty on_error)

(* Solve type variables on top of stack, without losing completeness (by using
 * their variance), and pop variables off the stack
 *)
let close_tyvars_and_solve env on_error =
  Env.log_env_change "close_tyvars_and_solve" env
  @@
  let tyvars = Env.get_current_tyvars env in
  let env = Env.close_tyvars env in
  List.fold_left tyvars ~init:env ~f:(fun env tyvar ->
      solve_tyvar_wrt_variance env Reason.Rnone tyvar on_error)

let close_tyvars_and_solve_gi env make_on_error =
  let tyvars = Env.get_current_tyvars env in
  let env = Env.close_tyvars env in
  List.fold_left tyvars ~init:env ~f:(fun env tyvar ->
      solve_tyvar_wrt_variance env Reason.Rnone tyvar (make_on_error tyvar))

(* Currently, simplify_subtype doesn't look at bounds on type variables.
 * Let's at least notice when these bounds imply an equality.
 *)
let is_sub_type env ty1 ty2 =
  (* It seems weird that this can cause errors, but I'm wary of simply discarding
   *  errors here. Using unify_error for now to maintain existing behavior. *)
  let (env, ty1) = expand_type_and_solve_eq env ty1 Errors.unify_error in
  let (env, ty2) = expand_type_and_solve_eq env ty2 Errors.unify_error in
  Typing_utils.is_sub_type env ty1 ty2

(**
 * Strips away all Toption that we possible can in a type, expanding type
 * variables along the way, turning ?T -> T. This exists to avoid ??T when
 * we wrap a type in Toption while typechecking.
 *)
let rec non_null env pos ty =
  (* This is to mimic the previous behaviour of non_null on Tabstract, but
  is hacky. We basically non_nullify the concrete supertypes of abstract
  types. *)
  let make_concrete_super_types_nonnull =
    object
      inherit Type_mapper.union_inter_type_mapper as super

      method! on_tabstract env r ak cstr =
        let ty = (r, Tabstract (ak, cstr)) in
        match TUtils.get_concrete_supertypes env ty with
        | (env, [ty'])
          when Typing_utils.is_sub_type_for_union
                 env
                 (MakeType.null Reason.none)
                 ty' ->
          let (env, ty') = non_null env pos ty' in
          (env, (r, Tabstract (ak, Some ty')))
        | (env, _) -> super#on_tabstract env r ak cstr
    end
  in
  let (env, ty) = make_concrete_super_types_nonnull#on_type env ty in
  let r = Reason.Rwitness pos in
  Inter.intersect env r ty (MakeType.nonnull r)

(**
 * During global inference we want to remove any reference to local tyvars in
 * the global tyvars.
 * This function will expand all lower bounds and upper bounds of the global
 * tvenv, which will have as effect to remove local tyvars
 *)
let expand_bounds_of_global_tyvars env =
  Env.log_env_change "expand_bounds_of_global_tyvars" env
  @@ {
       env with
       global_tvenv =
         IMap.map
           (fun tyvar_info ->
             let upper_bounds =
               ITySet.map
                 (Typing_expand.fully_expand_i env)
                 tyvar_info.upper_bounds
             in
             let lower_bounds =
               ITySet.map
                 (Typing_expand.fully_expand_i env)
                 tyvar_info.lower_bounds
             in
             { tyvar_info with upper_bounds; lower_bounds })
           env.global_tvenv;
     }
