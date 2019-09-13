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
module Reason = Typing_reason
module Env = Typing_env
module Inter = Typing_intersection
module Union = Typing_union
module TUtils = Typing_utils
module Cls = Decl_provider.Class
module TySet = Typing_set
module MakeType = Typing_make_type

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
let rec freshen_inside_ty env ((r, ty_) as ty) =
  let default () = (env, ty) in
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
  | Tdestructure tyl ->
    let (env, tyl) = List.map_env env tyl freshen_ty in
    (env, (r, Tdestructure tyl))
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
      | AKany
      | AKempty ->
        default ()
      | AKvarray ty ->
        let (env, ty) = freshen_ty env ty in
        (env, (r, Tarraykind (AKvarray ty)))
      | AKvec ty ->
        let (env, ty) = freshen_ty env ty in
        (env, (r, Tarraykind (AKvec ty)))
      | AKvarray_or_darray ty ->
        let (env, ty) = freshen_ty env ty in
        (env, (r, Tarraykind (AKvarray_or_darray ty)))
      | AKdarray (ty1, ty2) ->
        let (env, ty1) = freshen_ty env ty1 in
        let (env, ty2) = freshen_ty env ty2 in
        (env, (r, Tarraykind (AKdarray (ty1, ty2))))
      | AKmap (ty1, ty2) ->
        let (env, ty1) = freshen_ty env ty1 in
        let (env, ty2) = freshen_ty env ty2 in
        (env, (r, Tarraykind (AKmap (ty1, ty2))))
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
      if variance = Ast_defs.Invariant then
        (env, ty)
      else
        freshen_ty env ty
    in
    (env, ty :: tyl)
  | _ -> (env, tyl)

let var_occurs_in_ty env var ty =
  let finder =
    object
      inherit [env * bool] Type_visitor.locl_type_visitor

      method! on_tvar (env, occurs) _r v =
        if occurs then
          (env, occurs)
        else
          let (env, v) = Env.get_var env v in
          (env, v = var)
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
  (* Remove the variable from the environment *)
  let env = Env.remove_tyvar env var in
  env

let var_as_ty var = (Reason.Rnone, Tvar var)

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
    | (_, Tvar v) when v = var -> (env, MakeType.nothing r)
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
  in
  remove env lower_bound

let remove_tyvar_from_lower_bounds env var r lower_bounds =
  TySet.fold
    (fun lower_bound (env, acc) ->
      let (env, lower_bound) =
        remove_tyvar_from_lower_bound env var r lower_bound
      in
      (env, TySet.add lower_bound acc))
    lower_bounds
    (env, TySet.empty)

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
    | (_, Tvar v) when v = var -> (env, MakeType.mixed r)
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
  in
  remove env upper_bound

let remove_tyvar_from_upper_bounds env var r upper_bounds =
  TySet.fold
    (fun upper_bound (env, acc) ->
      let (env, upper_bound) =
        remove_tyvar_from_upper_bound env var r upper_bound
      in
      (env, TySet.add upper_bound acc))
    upper_bounds
    (env, TySet.empty)

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

(* Solve type variable var by assigning it to the union of its lower bounds.
 * If freshen=true, first freshen the covariant and contravariant components of
 * the bounds.
 *)
let bind_to_lower_bound ~freshen env r var lower_bounds on_error =
  Env.log_env_change "bind_to_lower_bound" env
  @@
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
      let env = Typing_subtype.sub_type env ty newty on_error in
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
        | (env, (_, Tvar v)) -> Env.remove_tyvar_upper_bound env v var
        | (env, _) -> env)
      lower_bounds
      env
  in
  (* Now actually make the assignment var := ty, and remove var from tvenv *)
  bind env var ty

let bind_to_upper_bound env r var upper_bounds =
  Env.log_env_change "bind_to_upper_bound" env
  @@
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
    | (env, (_, Tvar v)) -> Env.remove_tyvar_lower_bound env v var
    | (env, _) -> env
  in
  bind env var ty

let tyvar_is_solved env var =
  match snd @@ snd @@ Env.expand_type env (var_as_ty var) with
  | Tvar var' when var' = var -> false
  | _ -> true

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
let ty_equal_shallow ty1 ty2 =
  match (snd ty1, snd ty2) with
  | (Tany _, Tany _)
  | (Tnonnull, Tnonnull)
  | (Terr, Terr)
  | (Tdynamic, Tdynamic)
  | (Tobject, Tobject)
  | (Ttuple _, Ttuple _)
  | (Tarraykind (AKvarray _), Tarraykind (AKvarray _))
  | (Tarraykind (AKvec _), Tarraykind (AKvec _))
  | (Tarraykind (AKvarray_or_darray _), Tarraykind (AKvarray_or_darray _))
  | (Tarraykind (AKdarray _), Tarraykind (AKdarray _))
  | (Tarraykind (AKmap _), Tarraykind (AKmap _)) ->
    true
  | (Tprim p1, Tprim p2) -> p1 = p2
  | (Tclass (x_sub, exact_sub, _), Tclass (x_super, exact_super, _)) ->
    snd x_sub = snd x_super && exact_sub = exact_super
  | (Tfun fty1, Tfun fty2) ->
    fty1.ft_is_coroutine = fty2.ft_is_coroutine
    && fty1.ft_arity = fty2.ft_arity
    && fty1.ft_reactive = fty2.ft_reactive
    && fty1.ft_return_disposable = fty2.ft_return_disposable
    && fty1.ft_mutability = fty2.ft_mutability
  | (Tshape (shape_kind1, fdm1), Tshape (shape_kind2, fdm2)) ->
    shape_kind1 = shape_kind2
    && List.compare
         (fun (k1, v1) (k2, v2) ->
           match Ast_defs.ShapeField.compare k1 k2 with
           | 0 -> compare v1.sft_optional v2.sft_optional
           | n -> n)
         (ShapeFieldMap.elements fdm1)
         (ShapeFieldMap.elements fdm2)
       = 0
  | (Tabstract (AKnewtype (n1, _), _), Tabstract (AKnewtype (n2, _), _)) ->
    n1 = n2
  | (Tabstract (ak1, _), Tabstract (ak2, _)) -> ak1 = ak2
  | _ -> false

let union_any_if_any_in_lower_bounds env ty lower_bounds =
  let r = Reason.none in
  let any = (r, Typing_defs.make_tany ()) and err = (r, Terr) in
  let (env, ty) =
    match TySet.find_opt any lower_bounds with
    | Some any -> Union.union env ty any
    | None -> (env, ty)
  in
  let (env, ty) =
    match TySet.find_opt err lower_bounds with
    | Some err -> Union.union env ty err
    | None -> (env, ty)
  in
  (env, ty)

let try_bind_to_equal_bound ~freshen env r var on_error =
  if tyvar_is_solved env var then
    env
  else
    Env.log_env_change "bind_to_equal_bound" env
    @@
    let env = remove_tyvar_from_bounds env r var in
    let expand_all tyset =
      Typing_set.map
        (fun ty ->
          let (_, ty) = Env.expand_type env ty in
          ty)
        tyset
    in
    let tyvar_info = Env.get_tyvar_info env var in
    let lower_bounds = expand_all tyvar_info.lower_bounds in
    let upper_bounds = expand_all tyvar_info.upper_bounds in
    let equal_bounds = Typing_set.inter lower_bounds upper_bounds in
    let r = Reason.none in
    let any = (r, Typing_defs.make_tany ()) and err = (r, Terr) in
    let equal_bounds = equal_bounds |> TySet.remove any |> TySet.remove err in
    match Typing_set.choose_opt equal_bounds with
    | Some ty ->
      let (env, ty) = union_any_if_any_in_lower_bounds env ty lower_bounds in
      bind env var ty
    | None ->
      if not freshen then
        env
      else
        Typing_set.fold
          (fun upper_bound env ->
            Typing_set.fold
              (fun lower_bound env ->
                if tyvar_is_solved env var then
                  let (env, ty) = Env.expand_type env (var_as_ty var) in
                  let env =
                    Typing_subtype.sub_type env lower_bound ty on_error
                  in
                  let env =
                    Typing_subtype.sub_type env ty upper_bound on_error
                  in
                  env
                else if ty_equal_shallow lower_bound upper_bound then
                  let (env, ty) = freshen_inside_ty env lower_bound in
                  let env =
                    Typing_subtype.sub_type env lower_bound ty on_error
                  in
                  let env =
                    Typing_subtype.sub_type env ty upper_bound on_error
                  in
                  let (env, ty) =
                    union_any_if_any_in_lower_bounds env ty lower_bounds
                  in
                  bind env var ty
                else
                  env)
              lower_bounds
              env)
          upper_bounds
          env

(* Always solve a type variable. We force to the lower bounds, because it
 * produces a "more specific" type, and we don't have support for intersections
 * of upper bounds
 *)
let rec always_solve_tyvar ~freshen env r var on_error =
  (* If there is a type that is both a lower and upper bound, force to that type *)
  let env = try_bind_to_equal_bound ~freshen env r var on_error in
  if tyvar_is_solved env var then
    env
  else
    let tyvar_info = Env.get_tyvar_info env var in
    let r =
      if r = Reason.Rnone then
        Reason.Rwitness tyvar_info.tyvar_pos
      else
        r
    in
    let env =
      bind_to_lower_bound ~freshen env r var tyvar_info.lower_bounds on_error
    in
    let (env, ety) = Env.expand_var env r var in
    match ety with
    | (_, Tvar var') when var <> var' ->
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
  if tyvar_is_solved env var then
    env
  else
    let tyvar_info = Env.get_tyvar_info env var in
    let r =
      if r = Reason.Rnone then
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
    | (_, Tvar v') when v <> v' -> solve_until_concrete_ty env v'
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
  Typing_subtype.log_prop env;
  env

(* Expand an already-solved type variable, and solve an unsolved type variable
 * by binding it to the union of its lower bounds, with covariant and contravariant
 * components of the type suitably "freshened". For example,
 *    vec<C> <: #1
 * will be solved by
 *    #1 := vec<#2>  where C <: #2
 *)
let expand_type_and_solve env ~description_of_expected p ty on_error =
  let (env', ety) =
    Typing_utils.simplify_unions env ty ~on_tyvar:(fun env r v ->
        let env = always_solve_tyvar ~freshen:true env r v on_error in
        Env.expand_var env r v)
  in
  match (ty, ety) with
  | ((r, Tvar v), (_, Tunion []))
    when Env.get_tyvar_appears_invariantly env v
         || TypecheckerOptions.new_inference_lambda (Env.get_tcopt env) ->
    Errors.unknown_type
      description_of_expected
      p
      (Reason.to_string "It is unknown" r);
    let env = Env.set_tyvar_eager_solve_fail env v in
    (env, (Reason.Rsolve_fail p, TUtils.terr env))
  | _ -> (env', ety)

let expand_type_and_solve_eq env ty on_error =
  Typing_utils.simplify_unions env ty ~on_tyvar:(fun env r v ->
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
  Typing_subtype.is_sub_type_ignore_generic_params
    env
    ty
    (Reason.none, Tunion [])

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
            TySet.elements (Env.get_tyvar_lower_bounds env v)
          in
          Typing_union.union_list env r lower_bounds)
  in
  if not !has_tyvar then
    Typing_utils.simplify_unions env ty
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
          let env = Typing_subtype.sub_type env ty widened_ty on_error in
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

(* Currently, simplify_subtype doesn't look at bounds on type variables.
 * Let's at least notice when these bounds imply an equality.
 *)
let is_sub_type env ty1 ty2 =
  (* It seems weird that this can cause errors, but I'm wary of simply discarding
   *  errors here. Using unify_error for now to maintain existing behavior. *)
  let (env, ty1) = expand_type_and_solve_eq env ty1 Errors.unify_error in
  let (env, ty2) = expand_type_and_solve_eq env ty2 Errors.unify_error in
  Typing_subtype.is_sub_type env ty1 ty2

let rec push_option_out pos env ty =
  let is_option = function
    | (_, Toption _) -> true
    | _ -> false
  in
  let (env, ty) = Env.expand_type env ty in
  match ty with
  | (r, Toption ty) ->
    let (env, ty) = push_option_out pos env ty in
    ( env,
      if is_option ty then
        ty
      else
        (r, Toption ty) )
  | (r, Tprim Aast.Tnull) ->
    let ty = (r, Tunion []) in
    (env, (r, Toption ty))
  | (r, Tunion tyl) ->
    let (env, tyl) = List.map_env env tyl (push_option_out pos) in
    if List.exists tyl is_option then
      let ((env, tyl), r') =
        List.fold_right
          tyl
          ~f:
            begin
              fun ty ((env, tyl), r) ->
              match ty with
              | (r', Toption ty') -> (TUtils.flatten_unresolved env ty' tyl, r')
              | _ -> (TUtils.flatten_unresolved env ty tyl, r)
            end
          ~init:((env, []), r)
      in
      (env, (r', Toption (r, Tunion tyl)))
    else
      let (env, tyl) =
        List.fold_right
          tyl
          ~f:begin
               fun ty (env, tyl) -> TUtils.flatten_unresolved env ty tyl
             end
          ~init:(env, [])
      in
      (env, (r, Tunion tyl))
  | (r, Tintersection tyl) ->
    let (env, tyl) = List.map_env env tyl (push_option_out pos) in
    if List.for_all tyl is_option then
      let (r', tyl) =
        List.fold_map tyl ~init:Reason.none ~f:(fun r' ty ->
            match ty with
            | (r, Toption ty) -> (r, ty)
            | _ -> (r', ty))
      in
      (env, (r', Toption (r, Tintersection tyl)))
    else
      (env, (r, Tintersection tyl))
  | (r, Tabstract (ak, _)) ->
    begin
      match TUtils.get_concrete_supertypes env ty with
      | (env, [ty']) ->
        let (env, ty') = push_option_out pos env ty' in
        (match ty' with
        | (r', Toption ty') ->
          (env, (r', Toption (r, Tabstract (ak, Some ty'))))
        | _ -> (env, ty))
      | (env, _) -> (env, ty)
    end
  (* Solve type variable to lower bound if it's manifestly nullable *)
  | (_, Tvar var) ->
    let rec has_null env ty =
      match snd (Env.expand_type env ty) with
      | (_, Tprim Aast.Tnull) -> true
      | (_, Toption _) -> true
      | (_, Tabstract (_, Some ty)) -> has_null env ty
      | _ -> false
    in
    let lower_bounds =
      Typing_set.elements (Typing_env.get_tyvar_lower_bounds env var)
    in
    if List.exists lower_bounds (has_null env) then
      let (env, ty) =
        expand_type_and_solve
          env
          ~description_of_expected:"a value of known type"
          pos
          ty
          Errors.unify_error
      in
      push_option_out pos env ty
    else
      (env, ty)
  | ( _,
      ( Terr | Tany _ | Tnonnull | Tarraykind _ | Tprim _ | Tclass _ | Ttuple _
      | Tanon _ | Tfun _ | Tobject | Tshape _ | Tdynamic | Tdestructure _
      | Tpu _ | Tpu_access _ ) ) ->
    (env, ty)

(**
 * Strips away all Toption that we possible can in a type, expanding type
 * variables along the way, turning ?T -> T. This exists to avoid ??T when
 * we wrap a type in Toption while typechecking.
 *)
let non_null env pos ty =
  let (env, ty) = push_option_out pos env ty in
  match ty with
  | (_, Toption ty') -> (env, ty')
  | _ -> (env, ty)
