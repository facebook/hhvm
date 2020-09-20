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
module Utils = Typing_solver_utils
module Cls = Decl_provider.Class
module TySet = Typing_set
module MakeType = Typing_make_type

let log_remaining_prop env =
  let filename = Pos.filename env.function_pos in
  if not (Relative_path.is_hhi (Relative_path.prefix filename)) then (
    let prop =
      Typing_inference_env.get_nongraph_subtype_prop env.inference_env
    in
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
  )

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
  let (env, ty) = Env.expand_type env ty in
  let (r, ty_) = deref ty in
  match ty_ with
  | Tany _
  | Tnonnull
  | Terr
  | Tdynamic
  | Tobject
  | Tprim _ ->
    default ()
  | Tgeneric (name, tyl) ->
    if List.is_empty tyl then
      default ()
    else
      (* TODO(T69931993) Replace Invariant here once we support arbitrary variances
        on HK generics *)
      let variancel =
        List.replicate ~num:(List.length tyl) Ast_defs.Invariant
      in
      let (env, tyl) = freshen_tparams env variancel tyl in
      (env, mk (r, Tnewtype (name, tyl, ty)))
  | Tdependent _ -> default ()
  (* Nullable is covariant *)
  | Toption ty ->
    let (env, ty) = freshen_inside_ty env ty in
    (env, mk (r, Toption ty))
  | Tunion tyl ->
    let (env, tyl) = List.map_env env tyl freshen_inside_ty in
    (env, mk (r, Tunion tyl))
  | Tintersection tyl ->
    let (env, tyl) = List.map_env env tyl freshen_inside_ty in
    Inter.intersect_list env r tyl
  (* Tuples are covariant *)
  | Ttuple tyl ->
    let (env, tyl) = List.map_env env tyl freshen_ty in
    (env, mk (r, Ttuple tyl))
  (* Shape data is covariant *)
  | Tshape (shape_kind, fdm) ->
    let (env, fdm) = ShapeFieldMap.map_env freshen_ty env fdm in
    (env, mk (r, Tshape (shape_kind, fdm)))
  (* Functions are covariant in return type, contravariant in parameter types *)
  | Tfun ft ->
    let (env, ft_ret) = freshen_possibly_enforced_ty env ft.ft_ret in
    let (env, ft_params) =
      List.map_env env ft.ft_params (fun env p ->
          let (env, fp_type) = freshen_possibly_enforced_ty env p.fp_type in
          (env, { p with fp_type }))
    in
    (env, mk (r, Tfun { ft with ft_ret; ft_params }))
  | Tnewtype (name, tyl, ty) ->
    if List.is_empty tyl then
      default ()
    else begin
      match Env.get_typedef env name with
      | None -> default ()
      | Some td ->
        let variancel = List.map td.td_tparams (fun t -> t.tp_variance) in
        let (env, tyl) = freshen_tparams env variancel tyl in
        (env, mk (r, Tnewtype (name, tyl, ty)))
    end
  | Tclass ((p, cid), e, tyl) ->
    if List.is_empty tyl then
      default ()
    else begin
      match Env.get_class env cid with
      | None -> default ()
      | Some cls ->
        let variancel = List.map (Cls.tparams cls) (fun t -> t.tp_variance) in
        let (env, tyl) = freshen_tparams env variancel tyl in
        (env, mk (r, Tclass ((p, cid), e, tyl)))
    end
  | Tvarray ty ->
    let (env, ty) = freshen_ty env ty in
    (env, mk (r, Tvarray ty))
  | Tvarray_or_darray (ty1, ty2) ->
    let (env, ty1) = freshen_ty env ty1 in
    let (env, ty2) = freshen_ty env ty2 in
    (env, mk (r, Tvarray_or_darray (ty1, ty2)))
  | Tdarray (ty1, ty2) ->
    let (env, ty1) = freshen_ty env ty1 in
    let (env, ty2) = freshen_ty env ty2 in
    (env, mk (r, Tdarray (ty1, ty2)))
  | Tvar _ -> default ()
  | Tpu _
  | Tpu_type_access _ ->
    (* TODO(T36532263) suggested by Catherine, might be updated next *)
    default ()
  | Tunapplied_alias _ -> default ()

and freshen_ty env ty = Env.fresh_invariant_type_var env (get_pos ty)

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

let bind env var (ty : locl_ty) =
  Env.log_env_change "bind" env
  @@
  (* If there has been a use of this type variable that led to an "unknown type"
   * error (e.g. method invocation), then record this in the reason info. We
   * can make use of this for linters and code mods that suggest annotations *)
  let ty =
    map_reason ty ~f:(fun r ->
        if Env.get_tyvar_eager_solve_fail env var then
          Reason.Rsolve_fail (Reason.to_pos r)
        else
          r)
  in
  (* Update the variance *)
  let env = Env.update_variance_after_bind env var ty in
  (* Unify the variable *)
  let ty = Utils.err_if_var_in_ty env var ty in
  let env = Env.add env var ty in
  env

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
    let (env, lower_bounds) =
      Utils.remove_tyvar_from_lower_bounds env var lower_bounds
    in
    let lower_bounds = Utils.filter_locl_types lower_bounds in
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
          let (env, ty) = Env.expand_type env ty in
          match get_node ty with
          | Tvar v when not (Env.is_global_tyvar env v) ->
            Env.remove_tyvar_upper_bound env v var
          | _ -> env)
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
    let (env, upper_bounds) =
      Utils.remove_tyvar_from_upper_bounds env var upper_bounds
    in
    let upper_bounds = Utils.filter_locl_types upper_bounds in
    let (env, ty) = Inter.intersect_list env r (TySet.elements upper_bounds) in
    (* If ty is a variable (in future, if any of the types in the list are variables),
    * then remove var from their lower bounds. Why? Because if we construct
    *   var <: v1 , ... , vn , t
    * for type variables v1 , ... , vn and non-type variable t
    * then necessarily we must have var as a lower bound on each of vi
    * so after binding var we end up with redundant bounds
    *   v1 & ... & vn & t <: vi
    *)
    let (env, ty) = Env.expand_type env ty in
    let env =
      match get_node ty with
      | Tvar v when not (Env.is_global_tyvar env v) ->
        Env.remove_tyvar_lower_bound env v var
      | _ -> env
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
  match (get_node ty1, get_node ty2) with
  | (Tany _, Tany _)
  | (Tnonnull, Tnonnull)
  | (Terr, Terr)
  | (Tdynamic, Tdynamic)
  | (Tobject, Tobject)
  | (Ttuple _, Ttuple _)
  | (Tvarray _, Tvarray _)
  | (Tvarray_or_darray _, Tvarray_or_darray _)
  | (Tdarray _, Tdarray _) ->
    true
  | (Tprim p1, Tprim p2) -> Aast_defs.equal_tprim p1 p2
  | (Tclass (x_sub, exact_sub, _), Tclass (x_super, exact_super, _)) ->
    String.equal (snd x_sub) (snd x_super) && equal_exact exact_sub exact_super
  | (Tfun fty1, Tfun fty2) ->
    equal_locl_fun_arity fty1 fty2
    && equal_reactivity fty1.ft_reactive fty2.ft_reactive
    && Int.equal fty1.ft_flags fty2.ft_flags
  | (Tshape (shape_kind1, fdm1), Tshape (shape_kind2, fdm2)) ->
    equal_shape_kind shape_kind1 shape_kind2
    && List.equal
         ~equal:(fun (k1, v1) (k2, v2) ->
           Ast_defs.ShapeField.equal k1 k2
           && Bool.equal v1.sft_optional v2.sft_optional)
         (ShapeFieldMap.elements fdm1)
         (ShapeFieldMap.elements fdm2)
  | (Tnewtype (n1, _, _), Tnewtype (n2, _, _)) -> String.equal n1 n2
  | (Tdependent (dep1, _), Tdependent (dep2, _)) ->
    equal_dependent_type dep1 dep2
  | _ -> false

let union_any_if_any_in_lower_bounds env ty lower_bounds =
  let r = Reason.none in
  let any = LoclType (mk (r, Typing_defs.make_tany ()))
  and err = LoclType (MakeType.err r) in
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
  if Env.tyvar_is_solved_or_skip_global env var then
    env
  else
    Env.log_env_change "bind_to_equal_bound" env
    @@
    let env = Utils.remove_tyvar_from_bounds env var in
    let expand_all tyset =
      ITySet.map
        (fun ty ->
          let (_, ty) = Env.expand_internal_type env ty in
          ty)
        tyset
    in
    let lower_bounds = expand_all (Env.get_tyvar_lower_bounds env var) in
    let upper_bounds = expand_all (Env.get_tyvar_upper_bounds env var) in
    let equal_bounds = ITySet.inter lower_bounds upper_bounds in
    let any = LoclType (mk (Reason.none, Typing_defs.make_tany ()))
    and err = LoclType (MakeType.err Reason.none) in
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
        (* Search for lower bound and upper bound pair that shallowly match.
         * Then freshen the inner types on the matched type. We then want
         * to make var equal to the matched type so we add the constraints
         *  var <: matched_ty and matched_ty <: var
         * Finally we bind var to matched_ty
         *)
        let shallow_match =
          ITySet.find_first_opt
            (fun lower_bound ->
              ITySet.exists
                (fun upper_bound ->
                  match (lower_bound, upper_bound) with
                  | (LoclType lower_bound, LoclType upper_bound) ->
                    ty_equal_shallow env lower_bound upper_bound
                  | _ -> false)
                upper_bounds)
            lower_bounds
        in
        let env =
          match shallow_match with
          | Some (LoclType shallow_match) ->
            let (env, ty) = freshen_inside_ty env shallow_match in
            let var_ty = mk (r, Tvar var) in
            let env = Typing_utils.sub_type env ty var_ty on_error in
            let env = Typing_utils.sub_type env var_ty ty on_error in
            let (env, ty) =
              union_any_if_any_in_lower_bounds env ty lower_bounds
            in
            bind env var ty
          | _ -> env
        in
        env

(* Always solve a type variable.
We are here because we eagerly solve a type variable to see
whether certain operations are allowed on the type (e.g. a  method call).
Therefore, we always force to the lower bounds (even contravariant variables),
because it produces a "more specific" type, which is more likely to support
the operation which we eagerly solved for in the first place.
*)
let rec always_solve_tyvar_down ~freshen env r var on_error =
  (* If there is a type that is both a lower and upper bound, force to that type *)
  let env = try_bind_to_equal_bound ~freshen env r var on_error in
  if Env.tyvar_is_solved_or_skip_global env var then
    env
  else
    let r =
      if Reason.(equal r Rnone) then
        Reason.Rwitness (Env.get_tyvar_pos env var)
      else
        r
    in
    let env =
      let lower_bounds = Env.get_tyvar_lower_bounds env var in
      bind_to_lower_bound ~freshen env r var lower_bounds on_error
    in
    let (env, ety) = Env.expand_var env r var in
    match get_node ety with
    | Tvar var' when not (Ident.equal var var') ->
      always_solve_tyvar_down ~freshen env r var on_error
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
  if Env.tyvar_is_solved_or_skip_global env var then
    env
  else
    let r =
      if Reason.equal r Reason.Rnone then
        Reason.Rwitness (Env.get_tyvar_pos env var)
      else
        r
    in
    let lower_bounds = Env.get_tyvar_lower_bounds env var
    and upper_bounds = Env.get_tyvar_upper_bounds env var
    and appears_covariantly = Env.get_tyvar_appears_covariantly env var
    and appears_contravariantly =
      Env.get_tyvar_appears_contravariantly env var
    in
    match (appears_covariantly, appears_contravariantly) with
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

let solve_to_equal_bound_or_wrt_variance env r var on_error =
  Typing_log.(
    log_with_level env "prop" 2 (fun () ->
        log_types
          (Reason.to_pos r)
          env
          [
            Log_head
              ( Printf.sprintf
                  "Typing_subtype.solve_to_equal_bound_or_wrt_variance #%d"
                  var,
                [] );
          ]));

  (* If there is a type that is both a lower and upper bound, force to that type *)
  let env = try_bind_to_equal_bound ~freshen:false env r var on_error in
  solve_tyvar_wrt_variance env r var on_error

let solve_to_equal_bound_or_wrt_variance env r var on_error =
  let rec solve_until_concrete_ty env v =
    let env = solve_to_equal_bound_or_wrt_variance env r v on_error in
    let (env, ety) = Env.expand_var env r v in
    match get_node ety with
    | Tvar v' when not (Ident.equal v v') -> solve_until_concrete_ty env v'
    | _ -> env
  in
  solve_until_concrete_ty env var

let always_solve_tyvar env r var on_error =
  let env = solve_to_equal_bound_or_wrt_variance env r var on_error in
  let env = always_solve_tyvar_down ~freshen:false env r var on_error in
  env

let always_solve_tyvar_wrt_variance_or_down env r var ~on_error =
  let env = solve_tyvar_wrt_variance env r var on_error in
  let env = always_solve_tyvar_down ~freshen:false env r var on_error in
  env

(* Force solve all type variables in the environment *)
let solve_all_unsolved_tyvars env on_error =
  let env =
    Env.log_env_change "solve_all_unsolved_tyvars" env
    @@ List.fold (Env.get_all_tyvars env) ~init:env ~f:(fun env var ->
           if Env.is_global_tyvar env var then
             env
           else
             always_solve_tyvar env Reason.Rnone var on_error)
  in
  log_remaining_prop env;
  env

let solve_all_unsolved_tyvars_gi env make_on_error =
  let old_allow_solve_globals = Env.get_allow_solve_globals env in
  let env = Env.set_allow_solve_globals env true in
  let env =
    List.fold (Env.get_all_tyvars env) ~init:env ~f:(fun env tyvar ->
        always_solve_tyvar_wrt_variance_or_down
          env
          Reason.Rnone
          tyvar
          ~on_error:(make_on_error tyvar))
  in
  let env = Env.set_allow_solve_globals env old_allow_solve_globals in
  env

let unsolved_invariant_tyvars_under_union_and_intersection env ty =
  let rec find_tyvars (env, tyvars) ty =
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tvar v -> (env, (get_reason ty, v) :: tyvars)
    | Toption ty -> find_tyvars (env, tyvars) ty
    | Tunion tyl
    | Tintersection tyl ->
      List.fold tyl ~init:(env, tyvars) ~f:find_tyvars
    | Terr
    | Tany _
    | Tdynamic
    | Tnonnull
    | Tprim _
    | Tclass _
    | Tobject
    | Tgeneric _
    | Tnewtype _
    | Tdependent _
    | Tvarray _
    | Tdarray _
    | Tvarray_or_darray _
    | Ttuple _
    | Tshape _
    | Tfun _
    | Tpu _
    | Tpu_type_access _
    | Tunapplied_alias _ ->
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
let expand_type_and_solve
    env ?(freshen = true) ~description_of_expected p ty on_error =
  let (env, unsolved_invariant_tyvars) =
    unsolved_invariant_tyvars_under_union_and_intersection env ty
  in
  let (env', ety) =
    Typing_utils.simplify_unions env ty ~on_tyvar:(fun env r v ->
        let env = always_solve_tyvar_down ~freshen env r v on_error in
        Env.expand_var env r v)
  in
  let (env', ety) = Env.expand_type env' ety in
  match (unsolved_invariant_tyvars, get_node ety) with
  | (_ :: _, Tunion []) ->
    let env =
      List.fold unsolved_invariant_tyvars ~init:env ~f:(fun env (r, v) ->
          Errors.unknown_type
            description_of_expected
            p
            (Reason.to_string "It is unknown" r);
          Env.set_tyvar_eager_solve_fail env v)
    in
    (env, TUtils.terr env (Reason.Rsolve_fail p))
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
    match deref ty with
    | (r, Tunion tyl) -> widen_all env r tyl
    | (r, Toption ty) -> widen_all env r [MakeType.null r; ty]
    (* Don't widen the `this` type, because the field type changes up the hierarchy
     * so we lose precision
     *)
    | (_, Tdependent (DTthis, _)) -> (env, ty)
    (* For other abstract types, just widen to the bound, if possible *)
    | (_, Tdependent (_, ty))
    | (_, Tnewtype (_, _, ty)) ->
      widen env ty
    | _ ->
      begin
        match widen_concrete_type env ty with
        | (env, Some ty) -> (env, ty)
        | (env, None) -> (env, MakeType.nothing Reason.none)
      end
  and widen_all env r tyl =
    let (env, tyl) = List.fold_map tyl ~init:env ~f:widen in
    Typing_union.union_list env r tyl
  in
  widen env ty

let is_nothing env ty =
  Typing_utils.is_sub_type_ignore_generic_params
    env
    ty
    (MakeType.nothing Reason.none)

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
          (env, MakeType.nothing r)
        else
          let () = seen_tyvars := ISet.add v !seen_tyvars in
          let lower_bounds =
            TySet.elements
            @@ Utils.filter_locl_types
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
      solve_to_equal_bound_or_wrt_variance env Reason.Rnone tyvar on_error)

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

      method! on_tdependent env r dep cstr =
        let ty = mk (r, Tdependent (dep, cstr)) in
        match TUtils.get_concrete_supertypes env ty with
        | (env, [ty'])
          when Typing_utils.is_sub_type_for_union
                 env
                 (MakeType.null Reason.none)
                 ty' ->
          let (env, ty') = non_null env pos ty' in
          (env, mk (r, Tdependent (dep, ty')))
        | (env, _) -> super#on_tdependent env r dep cstr

      method! on_tnewtype env r x tyl cstr =
        let ty = mk (r, Tnewtype (x, tyl, cstr)) in
        match TUtils.get_concrete_supertypes env ty with
        | (env, [ty'])
          when Typing_utils.is_sub_type_for_union
                 env
                 (MakeType.null Reason.none)
                 ty' ->
          let (env, ty') = non_null env pos ty' in
          (env, mk (r, Tnewtype (x, tyl, ty')))
        | (env, _) -> super#on_tnewtype env r x tyl cstr
    end
  in
  let (env, ty) = make_concrete_super_types_nonnull#on_type env ty in
  let r = Reason.Rwitness pos in
  Inter.intersect env r ty (MakeType.nonnull r)

let try_bind_to_equal_bound env v =
  (* The error and reason we pass here doesn't matter since it's used only when `freshen` is true. *)
  try_bind_to_equal_bound ~freshen:false env Reason.none v Errors.unify_error
