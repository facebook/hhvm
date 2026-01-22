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
open Typing_defs_constraints
open Typing_env_types
module Reason = Typing_reason
module Env = Typing_env
module Inter = Typing_intersection
module ITySet = Internal_type_set
module TL = Typing_logic
module TUtils = Typing_utils
module Utils = Typing_solver_utils
module TySet = Typing_set
module MakeType = Typing_make_type

let log_remaining_prop env =
  let filename = Pos.filename env.genv.callable_pos in
  if not (Relative_path.is_hhi (Relative_path.prefix filename)) then (
    let prop =
      Typing_inference_env.get_nongraph_subtype_prop env.inference_env
    in
    (if TypecheckerOptions.log_inference_constraints (Env.get_tcopt env) then
      let p_as_string = Typing_print.subtype_prop env prop in
      let pos = Pos.string (Pos.to_absolute env.genv.callable_pos) in
      let size = TL.size prop in
      let n_disj = TL.n_disj prop in
      let n_conj = TL.n_conj prop in
      TypingLogger.InferenceCnstr.log p_as_string ~pos ~size ~n_disj ~n_conj);
    if (not (Diagnostics.currently_has_errors ())) && not (TL.is_valid prop)
    then
      Typing_log.log_prop
        1
        (Pos_or_decl.of_raw_pos env.genv.callable_pos)
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
let rec freshen_inside_ty env ty :
    (Typing_env_types.env * Typing_defs.locl_ty) option =
  let freshen_inside_tyl env tyl :
      (Typing_env_types.env * Typing_defs.locl_ty list) option =
    let (env, ty_either_l) =
      List.map_env env tyl ~f:(fun env ty ->
          match freshen_inside_ty env ty with
          | Some (env, ty) -> (env, Either.First ty)
          | None -> (env, Either.Second ty))
    in
    if List.for_all ty_either_l ~f:Either.is_second then
      None
    else
      Some (env, List.map ty_either_l ~f:Either.value)
  in
  let open Option.Let_syntax in
  let default () = None in
  let (env, ty) = Env.expand_type env ty in
  let (r, ty_) = deref ty in
  match ty_ with
  | Tany _
  | Tnonnull
  | Tdynamic
  | Tprim _
  | Tlabel _
  | Tneg _ ->
    default ()
  | Tgeneric _ -> default ()
  | Tdependent _ -> default ()
  (* Nullable is covariant *)
  | Toption ty ->
    let* (env, ty) = freshen_inside_ty env ty in
    return (env, mk (r, Toption ty))
  | Tunion tyl ->
    let* (env, tyl) = freshen_inside_tyl env tyl in
    return (env, mk (r, Tunion tyl))
  | Tintersection tyl ->
    let* (env, tyl) = freshen_inside_tyl env tyl in
    return (Inter.intersect_list env r tyl)
  (* Tuples are covariant *)
  | Ttuple { t_required; t_optional; t_extra } ->
    let (env, t_required) = List.map_env env t_required ~f:freshen_ty in
    let (env, t_optional) = List.map_env env t_optional ~f:freshen_ty in
    let (env, t_extra) =
      match t_extra with
      | Tsplat t_splat ->
        let (env, t_splat) = freshen_ty env t_splat in
        (env, Tsplat t_splat)
      | Tvariadic t_variadic ->
        let (env, t_variadic) = freshen_ty env t_variadic in
        (env, Tvariadic t_variadic)
    in
    Some (env, mk (r, Ttuple { t_required; t_optional; t_extra }))
  (* Shape data is covariant *)
  | Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fdm } ->
    let (env, fdm) = ShapeFieldMap.map_env freshen_ty env fdm in
    (* TODO(shapes) should freshening impact unknown type? *)
    Some
      ( env,
        mk
          ( r,
            Tshape
              {
                s_origin = Missing_origin;
                s_unknown_value = shape_kind;
                s_fields = fdm;
              } ) )
  (* Functions are covariant in return type, contravariant in parameter types *)
  | Tfun ft ->
    let (env, ft_ret) = freshen_ty env ft.ft_ret in
    let (env, ft_params) =
      List.map_env env ft.ft_params ~f:(fun env p ->
          let (env, fp_type) = freshen_ty env p.fp_type in
          (env, { p with fp_type }))
    in
    Some (env, mk (r, Tfun { ft with ft_ret; ft_params }))
  | Tnewtype (name, [ty], _)
    when String.equal name Naming_special_names.Classes.cSupportDyn ->
    let* (env, ty) = freshen_inside_ty env ty in
    return (env, MakeType.supportdyn r ty)
  | Tnewtype (name, tyl, ty_bound) ->
    if List.is_empty tyl then
      default ()
    else begin
      match Env.get_typedef env name with
      | Decl_entry.Found td -> begin
        match td.td_as_constraint with
        | None -> default ()
        | Some ty -> begin
          match get_node ty with
          | Tgeneric _ -> default ()
          | _ ->
            let tparams = Env.get_class_or_typedef_tparams env name in
            if List.is_empty tparams then
              default ()
            else
              let variancel = List.map tparams ~f:(fun t -> t.tp_variance) in
              let* (env, tyl) = freshen_tparams env variancel tyl in
              return (env, mk (r, Tnewtype (name, tyl, ty_bound)))
        end
      end
      | _ -> default ()
    end
  | Tclass ((p, cid), e, tyl) ->
    if List.is_empty tyl then
      default ()
    else
      let tparams = Env.get_class_or_typedef_tparams env cid in
      if List.is_empty tparams then
        default ()
      else
        let variancel = List.map tparams ~f:(fun t -> t.tp_variance) in
        let* (env, tyl) = freshen_tparams env variancel tyl in
        return (env, mk (r, Tclass ((p, cid), e, tyl)))
  | Tvec_or_dict (ty1, ty2) ->
    let (env, ty1) = freshen_ty env ty1 in
    let (env, ty2) = freshen_ty env ty2 in
    Some (env, mk (r, Tvec_or_dict (ty1, ty2)))
  | Tvar _ -> default ()
  | Taccess (ty, ids) ->
    let (env, ty) = freshen_ty env ty in
    Some (env, mk (r, Taccess (ty, ids)))
  | Tclass_ptr ty ->
    let (env, ty) = freshen_ty env ty in
    (* TODO(T199606542) Matches classname but does it actually make sense
     * to freshen class pointer? This will disappear when we move to direct
     * strings *)
    Some (env, mk (r, Tclass_ptr ty))

and freshen_ty env ty =
  if TUtils.is_tyvar_error env ty then
    Env.fresh_type_error env (get_pos ty |> Pos_or_decl.unsafe_to_raw_pos)
  else
    Env.fresh_type_invariant env (get_pos ty |> Pos_or_decl.unsafe_to_raw_pos)

and freshen_tparams env variancel tyl =
  if List.for_all variancel ~f:Ast_defs.(equal_variance Invariant) then
    None
  else
    let rec freshen_tparams_ env variancel tyl =
      match (variancel, tyl) with
      | ([], []) -> (env, [])
      | (variance :: variancel, ty :: tyl) ->
        let (env, tyl) = freshen_tparams_ env variancel tyl in
        let (env, ty) =
          if Ast_defs.(equal_variance variance Invariant) then
            (env, ty)
          else
            freshen_ty env ty
        in
        (env, ty :: tyl)
      | _ -> (env, tyl)
    in
    Some (freshen_tparams_ env variancel tyl)

let bind env var (ty : locl_ty) =
  let old_env = env in
  (* If there has been a use of this type variable that led to an "unknown type"
   * error (e.g. method invocation), then record this in the reason info. We
   * can make use of this for linters and code mods that suggest annotations *)
  let ty =
    map_reason ty ~f:(fun r ->
        if Env.get_tyvar_eager_solve_fail env var then
          Reason.solve_fail (Reason.to_pos r)
        else
          r)
  in
  (* Remember the T::Tx involving this variable before we erase that from
   * the env.
   *)
  let tconsts = Env.get_tyvar_type_consts env var in
  (* Update the variance *)
  let env = Env.update_variance_after_bind env var ty in
  (* Unify the variable *)
  let (ty, var_in_ty_err_opt) = Utils.err_if_var_in_ty_pure env var ty in
  let env = Env.add env var ty in
  (* Make sure we don't project from this variable if it is bound to
   * nothing, as it will lead to type holes.
   *)
  let proj_ty_err_opt =
    if TUtils.is_tyvar_error env ty then
      None
    else if TUtils.is_nothing env ty && not (SMap.is_empty tconsts) then
      let (_, ((proj_pos, tconst_name), _)) = SMap.choose tconsts
      and pos = Env.get_tyvar_pos env var in
      Some
        Typing_error.(
          primary
          @@ Primary.Unresolved_tyvar_projection { pos; tconst_name; proj_pos })
    else
      None
  in
  let ty_err_opt =
    Option.merge var_in_ty_err_opt proj_ty_err_opt ~f:Typing_error.both
  in
  (Env.log_env_change "bind" old_env env, ty_err_opt)

(* Solve type variable var by assigning it to the union of its lower bounds.
 * If freshen=true, first freshen the covariant and contravariant components of
 * the bounds.
 *)
let bind_to_lower_bound ~freshen env r var lower_bounds =
  let old_env = env in
  let (env, ty_err_opt) =
    if ITySet.exists is_constraint_type lower_bounds then
      (env, None)
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
      let (env, freshen_ty_err, ty) =
        if freshen then
          match freshen_inside_ty env ty with
          | None -> (env, None, ty)
          | Some (env, newty) ->
            (* In theory, the following subtype would only fail if the lower bound
             * was already in conflict with another bound. However we don't
             * add such conflicting bounds to avoid cascading errors, so in theory,
             * the following subtype calls should not fail, and the error callback
             * should not matter. *)
            let on_error =
              Some
                (Typing_error.Reasons_callback.unify_error_at
                @@ Env.get_tyvar_pos env var)
            in
            let (env, ty_err_opt) = TUtils.sub_type env ty newty on_error in
            (env, ty_err_opt, newty)
        else
          (env, None, ty)
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
            | Tvar v -> Env.remove_tyvar_upper_bound env v var
            | _ -> env)
          lower_bounds
          env
      in
      (* In autocomplete mode, we do not solve to nothing, as it makes a lot
       * of checks in the autocompletion fail for bad reasons (especially
       * around invariant generics. Leave them be to try to get leave more
       * flexibility to type filers *)
      let autocomplete_mode =
        TypecheckerOptions.tco_autocomplete_mode env.genv.tcopt
      in
      if autocomplete_mode && TUtils.is_nothing env ty then
        (env, freshen_ty_err)
      else
        (* Now actually make the assignment var := ty, and remove var from tvenv *)
        let (env, bind_ty_err) = bind env var ty in
        let ty_err_opt =
          Option.merge freshen_ty_err bind_ty_err ~f:Typing_error.both
        in
        (env, ty_err_opt)
  in
  (Env.log_env_change "bind_to_lower_bound" old_env env, ty_err_opt)

let bind_to_upper_bound env r var upper_bounds =
  let old_env = env in
  let (env, ty_err_opt) =
    if ITySet.exists is_constraint_type upper_bounds then
      (env, None)
    else
      let (env, upper_bounds) =
        Utils.remove_tyvar_from_upper_bounds env var upper_bounds
      in
      let upper_bounds = Utils.filter_locl_types upper_bounds in
      let (env, ty) =
        Inter.intersect_list env r (TySet.elements upper_bounds)
      in
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
        | Tvar v -> Env.remove_tyvar_lower_bound env v var
        | _ -> env
      in
      bind env var ty
  in
  (Env.log_env_change "bind_to_upper_bound" old_env env, ty_err_opt)

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
  if TUtils.is_tyvar_error env ty1 && TUtils.is_tyvar_error env ty2 then
    true
  else
    match (get_node ty1, get_node ty2) with
    | (Tany _, Tany _)
    | (Tnonnull, Tnonnull)
    | (Tdynamic, Tdynamic)
    | (Ttuple _, Ttuple _) ->
      true
    | (Tprim p1, Tprim p2) -> Aast_defs.equal_tprim p1 p2
    | (Tclass (x_sub, exact_sub, _), Tclass (x_super, exact_super, _)) ->
      String.equal (snd x_sub) (snd x_super)
      && exact_equal exact_sub exact_super
    | (Tfun fty1, Tfun fty2) ->
      Typing_defs_flags.Fun.equal fty1.ft_flags fty2.ft_flags
    | ( Tshape { s_origin = _; s_unknown_value = shape_kind1; s_fields = fdm1 },
        Tshape { s_origin = _; s_unknown_value = shape_kind2; s_fields = fdm2 }
      ) ->
      Bool.equal
        (TUtils.is_nothing env shape_kind1)
        (TUtils.is_nothing env shape_kind2)
      && List.equal
           (fun (k1, v1) (k2, v2) ->
             TShapeField.equal k1 k2
             && Bool.equal v1.sft_optional v2.sft_optional)
           (ShapeFieldMap.elements fdm1)
           (ShapeFieldMap.elements fdm2)
    | (Tnewtype (n1, _, _), Tnewtype (n2, _, _)) -> String.equal n1 n2
    | (Tdependent (dep1, _), Tdependent (dep2, _)) ->
      equal_dependent_type dep1 dep2
    | _ -> false

let try_bind_to_equal_bound ~freshen env r var =
  if Env.tyvar_is_solved env var then
    (env, None)
  else
    let old_env = env in
    let env = Utils.remove_tyvar_from_bounds env var in
    let is_null ity =
      match ity with
      | LoclType ty -> begin
        match get_node ty with
        | Tprim Aast_defs.Tnull -> true
        | _ -> false
      end
      | _ -> false
    in
    let lower_bounds = Env.get_tyvar_lower_bounds env var in
    let has_null_lower_bound = ITySet.exists is_null lower_bounds in
    let expand_all ~strip_supportdyn env tyset =
      ITySet.fold
        (fun ty (env, res, sd_res) ->
          let (env, ty) = Env.expand_internal_type env ty in
          match ty with
          | LoclType ty when strip_supportdyn ->
            let (_, env, stripped_ty) = TUtils.strip_supportdyn env ty in
            let res = ITySet.add (LoclType stripped_ty) res in
            let sd_res = ITySet.add (LoclType ty) sd_res in
            let res =
              if has_null_lower_bound then
                ITySet.add (LoclType (MakeType.nullable r stripped_ty)) res
              else
                res
            in
            (env, res, sd_res)
          | _ -> (env, ITySet.add ty res, sd_res))
        tyset
        (env, ITySet.empty, ITySet.empty)
    in
    let (env, lower_bounds, sd_lower_bounds) =
      expand_all ~strip_supportdyn:true env lower_bounds
    in
    let (env, upper_bounds, _) =
      expand_all
        ~strip_supportdyn:false
        env
        (Env.get_tyvar_upper_bounds env var)
    in
    let equal_bounds = ITySet.inter lower_bounds upper_bounds in
    let equal_bounds =
      if ITySet.is_empty equal_bounds then
        ITySet.inter sd_lower_bounds upper_bounds
      else
        equal_bounds
    in
    let (env, ty_err_opt) =
      match ITySet.choose_opt equal_bounds with
      | Some (LoclType ty) -> bind env var ty
      | Some (ConstraintType _)
      | None ->
        if not freshen then
          (env, None)
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
          (match shallow_match with
          | Some (LoclType shallow_match) ->
            let (env, ty) =
              Option.value
                (freshen_inside_ty env shallow_match)
                ~default:(env, shallow_match)
            in
            let var_ty = mk (r, Tvar var) in
            (* In theory, the following subtype would only fail if the shallow match
             * we've found was already in conflict with another bound. However we don't
             * add such conflicting bounds to avoid cascading errors, so in theory,
             * the following subtype calls should not fail, and the error callback
             * should not matter. *)
            let on_error =
              Some
                (Typing_error.Reasons_callback.unify_error_at
                @@ Env.get_tyvar_pos env var)
            in
            let (env, ty_sub_err_opt) =
              TUtils.sub_type env ty var_ty on_error
            in
            let (env, ty_sup_err_opt) =
              TUtils.sub_type env var_ty ty on_error
            in
            let (env, bind_err_opt) = bind env var ty in
            let ty_err_opt =
              Typing_error.multiple_opt
              @@ List.filter_map
                   ~f:Fn.id
                   [ty_sub_err_opt; ty_sup_err_opt; bind_err_opt]
            in
            (env, ty_err_opt)
          | _ -> (env, None))
    in
    (Env.log_env_change "bind_to_equal_bound" old_env env, ty_err_opt)

(* Always solve a type variable.
   We are here because we eagerly solve a type variable to see
   whether certain operations are allowed on the type (e.g. a  method call).
   Therefore, we always force to the lower bounds (even contravariant variables),
   because it produces a "more specific" type, which is more likely to support
   the operation which we eagerly solved for in the first place.

   During autocompletion, if the set of lower bounds is empty and the
   variable appears contravariantly, try an upper bound.
*)
let rec always_solve_tyvar_down ~freshen env r var =
  let autocomplete_mode =
    TypecheckerOptions.tco_autocomplete_mode env.genv.tcopt
  in
  (* If there is a type that is both a lower and upper bound, force to that type *)
  let (env, ty_err_opt1) = try_bind_to_equal_bound ~freshen env r var in
  if Env.tyvar_is_solved env var then
    (env, ty_err_opt1)
  else
    let r =
      if Reason.Predicates.is_none r then
        Reason.witness (Env.get_tyvar_pos env var)
      else
        r
    in
    let (env, ty_err_opt2) =
      let (env, to_lower, bounds) =
        let lower_bounds = Env.get_tyvar_lower_bounds env var in
        if autocomplete_mode then
          let upper_bounds = Env.get_tyvar_upper_bounds env var in
          let (env, upper_bounds) =
            (* Filtering mixed and support_dyn<mixed> *)
            ITySet.fold
              (fun ity (env, upper_bounds) ->
                match ity with
                | ConstraintType _ -> (env, ITySet.add ity upper_bounds)
                | LoclType lty ->
                  let (_, env, lty) = TUtils.strip_supportdyn env lty in
                  (match get_node lty with
                  | Toption topt ->
                    (match get_node topt with
                    | Tnonnull -> (env, upper_bounds)
                    | _ -> (env, ITySet.add ity upper_bounds))
                  | _ -> (env, ITySet.add ity upper_bounds)))
              upper_bounds
              (env, ITySet.empty)
          in
          if
            Env.get_tyvar_appears_contravariantly env var
            && ITySet.is_empty lower_bounds
            && not (ITySet.is_empty upper_bounds)
          then
            (env, false, upper_bounds)
          else
            (env, true, lower_bounds)
        else
          (env, true, lower_bounds)
      in
      (* We cannot do more on (<expr#1> as C) than on simply C,
       * so replace expression-dependent types with their bound.
       * This avoids solving to a type so specific that it ends
       * up hurting completeness. *)
      if to_lower then
        let bounds =
          ITySet.map
            (function
              | LoclType lty as ity ->
                let (_env, lty) = Env.expand_type env lty in
                (match get_node lty with
                | Tdependent (_, bnd) -> LoclType bnd
                | _ -> ity)
              | ity -> ity)
            bounds
        in
        bind_to_lower_bound ~freshen env r var bounds
      else
        bind_to_upper_bound env r var bounds
    in
    let (env, ety) = Env.expand_var env r var in
    match get_node ety with
    | Tvar var' when not (Tvid.equal var var') ->
      let (env, ty_err_opt3) = always_solve_tyvar_down ~freshen env r var in
      let ty_err_opt =
        Typing_error.multiple_opt
        @@ List.filter_map ~f:Fn.id [ty_err_opt1; ty_err_opt2; ty_err_opt3]
      in
      (env, ty_err_opt)
    | _ ->
      let ty_err_opt =
        Option.merge ty_err_opt1 ty_err_opt2 ~f:Typing_error.both
      in
      (env, ty_err_opt)

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
let solve_tyvar_wrt_variance env r var =
  if Env.tyvar_is_solved env var then
    (env, None)
  else
    let r =
      if Reason.Predicates.is_none r then
        Reason.witness (Env.get_tyvar_pos env var)
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
       * not appear at all, or only appears covariantly, solve to lower bound.
       * If there are no lower bounds, and we've got type constant projections
       * on the type variable, then solve to the upper bound anyway, hoping that
       * this resolves the projections.
       *)
      if
        ITySet.is_empty lower_bounds
        && not (SMap.is_empty (Env.get_tyvar_type_consts env var))
      then
        bind_to_upper_bound env r var upper_bounds
      else
        bind_to_lower_bound ~freshen:false env r var lower_bounds
    | (false, true) ->
      (* As in Local Type Inference by Pierce & Turner, if type variable
       * appears only contravariantly, solve to upper bound
       *)
      bind_to_upper_bound env r var upper_bounds
    | (true, true) ->
      (* Not ready to solve yet! *)
      (env, None)

let solve_to_equal_bound_or_wrt_variance env r var =
  Typing_log.(
    log_with_level env "prop" ~level:2 (fun () ->
        log_types
          (Reason.to_pos r)
          env
          [
            Log_head
              ( Printf.sprintf
                  "Typing_subtype.solve_to_equal_bound_or_wrt_variance #%s"
                  (Tvid.show var),
                [] );
          ]));

  (* If there is a type that is both a lower and upper bound, force to that type *)
  let (env, ty_err_opt1) = try_bind_to_equal_bound ~freshen:false env r var in
  let (env, ty_err_opt2) = solve_tyvar_wrt_variance env r var in
  let ty_err_opt = Option.merge ty_err_opt1 ty_err_opt2 ~f:Typing_error.both in
  (env, ty_err_opt)

let solve_to_equal_bound_or_wrt_variance env r var =
  let rec solve_until_concrete_ty env ty_errs v =
    let (env, ty_errs) =
      match solve_to_equal_bound_or_wrt_variance env r v with
      | (env, Some ty_err) -> (env, ty_err :: ty_errs)
      | (env, _) -> (env, ty_errs)
    in
    let (env, ety) = Env.expand_var env r v in
    match get_node ety with
    | Tvar v' when not (Tvid.equal v v') ->
      solve_until_concrete_ty env ty_errs v'
    | _ ->
      let ty_err_opt = Typing_error.multiple_opt ty_errs in
      (env, ty_err_opt)
  in
  solve_until_concrete_ty env [] var

let always_solve_tyvar env r var =
  let (env, ty_err_opt1) = solve_to_equal_bound_or_wrt_variance env r var in
  let (env, ty_err_opt2) = always_solve_tyvar_down ~freshen:false env r var in
  let ty_err_opt = Option.merge ty_err_opt1 ty_err_opt2 ~f:Typing_error.both in
  (env, ty_err_opt)

(* Force solve all type variables in the environment *)
let solve_all_unsolved_tyvars env =
  let old_env = env in
  let (env, ty_errs) =
    List.fold
      (Env.get_all_tyvars env)
      ~init:(env, [])
      ~f:(fun (env, ty_errs) var ->
        match always_solve_tyvar env Reason.none var with
        | (env, Some ty_err) -> (env, ty_err :: ty_errs)
        | (env, _) -> (env, ty_errs))
  in
  let env = Env.log_env_change "solve_all_unsolved_tyvars" old_env env in
  log_remaining_prop env;
  let ty_err_opt = Typing_error.multiple_opt ty_errs in
  (env, ty_err_opt)

let is_tyvar_error tvid ~env:Typing_env_types.{ inference_env; _ } =
  Typing_inference_env.is_error inference_env tvid

(* Expand an already-solved type variable, and solve an unsolved type variable
 * by binding it to the union of its lower bounds, with covariant and contravariant
 * components of the type suitably "freshened". For example,
 *    vec<C> <: #1
 * will be solved by
 *    #1 := vec<#2>  where C <: #2
 * The optional `default` parameter is used to solve a type variable
 * if `widen_concrete_type` does not produce a result.
 *)
let expand_type_and_solve env ?(freshen = true) ~description_of_expected p ty =
  (* If we're checking an SDT method or function under dynamic assumptions,
   * then attempt to solve to dynamic if we're left with a type variable *)
  let default =
    if Tast.is_under_dynamic_assumptions env.Typing_env_types.checked then
      Some (MakeType.dynamic (Reason.witness p))
    else
      None
  in
  (* TODO: rather than writing to a ref cell from inside the `on_tyvar`
     function, modify `simplify_unions` to return the accumulated result
     from this function, if it is provided *)
  let vars_solved_to_nothing = ref [] in
  let ty_errs = ref [] in
  let old_pos = Typing_inference_env.get_pos env.inference_env in
  let put_pos_in_env env pos =
    {
      env with
      inference_env = Typing_inference_env.set_pos env.inference_env pos;
    }
  in
  let env = put_pos_in_env env (Some p) in
  let (env', ety) =
    TUtils.simplify_unions env ty ~on_tyvar:(fun env r v ->
        let (env, ty_err_opt) = always_solve_tyvar_down ~freshen env r v in
        Option.iter ty_err_opt ~f:(fun ty_err -> ty_errs := ty_err :: !ty_errs);
        let (env, ety) = Env.expand_var env r v in
        (match get_node ety with
        | Tunion [] ->
          vars_solved_to_nothing := (r, v) :: !vars_solved_to_nothing
        | _ -> ());
        (env, ety))
  in
  let (env', ety) = Env.expand_type env' ety in
  let ((env', err), ty) =
    match (!vars_solved_to_nothing, get_node ety) with
    | (_ :: _, Tunion []) -> begin
      match default with
      | Some default_ty ->
        let res =
          TUtils.sub_type env ety default_ty
          @@ Some (Typing_error.Reasons_callback.unify_error_at p)
        in
        (res, default_ty)
      | None ->
        let env =
          List.fold !vars_solved_to_nothing ~init:env ~f:(fun env (r, v) ->
              if is_tyvar_error v ~env then
                env
              else
                let ty_err =
                  Typing_error.(
                    primary
                    @@ Primary.Unknown_type
                         {
                           expected = description_of_expected;
                           pos = p;
                           reason = lazy (Reason.to_string "It is unknown" r);
                         })
                in
                ty_errs := ty_err :: !ty_errs;
                Env.set_tyvar_eager_solve_fail env v)
        in
        let ty_err_opt = Typing_error.multiple_opt !ty_errs in
        let (env, ty) = Env.fresh_type_error env p in
        ((env, ty_err_opt), ty)
    end
    | _ ->
      let ty_err_opt = Typing_error.multiple_opt !ty_errs in
      ((env', ty_err_opt), ety)
  in
  ((put_pos_in_env env' old_pos, err), ty)

let expand_type_and_solve_eq env ty =
  let ty_err_opts = ref [] in
  let (env, ty) =
    TUtils.simplify_unions env ty ~on_tyvar:(fun env r v ->
        let (env, ty_err_opt) = try_bind_to_equal_bound ~freshen:true env r v in
        ty_err_opts := ty_err_opt :: !ty_err_opts;
        Env.expand_var env r v)
  in
  let (env, ty) = Env.expand_type env ty in
  let ty_err_opt =
    Typing_error.multiple_opt @@ List.filter_map ~f:Fn.id !ty_err_opts
  in
  ((env, ty_err_opt), ty)

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
  let ty_nothing = MakeType.nothing Reason.none in
  let rec widen env ty =
    TUtils.map_supportdyn env ty (fun env ty ->
        let (env, ty) = Env.expand_type env ty in
        match deref ty with
        | (r, Tunion tyl) -> widen_all env r tyl
        | (r, Toption ty) -> widen_all env r [MakeType.null r; ty]
        (* Don't widen the `this` type, because the field type changes up the hierarchy
         * so we lose precision
         *)
        | (_, Tgeneric "this") -> (env, ty)
        (* For abstract types, just widen to the bound, if possible *)
        | (_, Tdependent (_, ty)) -> widen env ty
        | (r, Tnewtype (n, tyl, _)) ->
          let (env, ty) = Typing_utils.get_newtype_super env r n tyl in
          widen env ty
        | _ ->
          let (env, ty_opt) = widen_concrete_type env ty in
          let ty = Option.value ~default:ty_nothing ty_opt in
          (env, ty))
  and widen_all env r tyl =
    let (env, tyl) = List.map_env env tyl ~f:widen in
    Typing_union.union_list env r tyl
  in
  widen env ty

let is_nothing env ty =
  TUtils.is_sub_type_ignore_generic_params env ty (MakeType.nothing Reason.none)

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
    env
    ?default
    ?(allow_nothing = false)
    ?(force_solve = true)
    ~description_of_expected
    widen_concrete_type
    p
    ty =
  (* If we're checking an SDT method or function under dynamic assumptions,
   * then attempt to solve to dynamic if we're left with a type variable *)
  let default =
    if Option.is_some default then
      default
    else if Tast.is_under_dynamic_assumptions env.Typing_env_types.checked then
      Some (MakeType.dynamic (Reason.witness p))
    else
      None
  in
  let ((env, ty_err_opt), ty) =
    let ((env, ty_err_opt1), ty) = expand_type_and_solve_eq env ty in
    (* Deconstruct the type into union elements (if it's a union). For variables,
     * take the lower bounds. If there are no variables, then we have a concrete
     * type so just return expanded type
     *)
    let has_tyvar = ref false in
    let seen_tyvars = ref Tvid.Set.empty in
    (* Simplify unions in ty, but when we encounter a type variable in the process,
       recursively replace it with the union of its lower bounds, effectively getting
       rid of all unsolved type variables in the union. *)
    let (env, concretized_ty) =
      Typing_union.simplify_unions env ty ~on_tyvar:(fun env r v ->
          has_tyvar := true;
          if Tvid.Set.mem v !seen_tyvars then
            (env, MakeType.nothing r)
          else
            let () = seen_tyvars := Tvid.Set.add v !seen_tyvars in
            let lower_bounds =
              TySet.elements
              @@ Utils.filter_locl_types
              @@ Env.get_tyvar_lower_bounds env v
            in
            Typing_union.union_list env r lower_bounds)
    in
    if not !has_tyvar then
      ((env, ty_err_opt1), ty)
    else
      let (env, widened_ty) = widen env widen_concrete_type concretized_ty in
      let ((env, ty_err_opt2), ty) =
        match
          ((not allow_nothing) && is_nothing env widened_ty, default, widened_ty)
        with
        | (true, None, _) ->
          if force_solve then
            expand_type_and_solve env ~description_of_expected p ty
          else
            ((env, None), ty)
        | (true, Some widened_ty, _)
        | (false, _, widened_ty) ->
          (* We really don't want to just guess `nothing` if none of the types can be widened *)
          let res =
            TUtils.sub_type env ty widened_ty
            @@ Some (Typing_error.Reasons_callback.unify_error_at p)
          in
          (match res with
          | (env, None) ->
            let supportdyn = Typing_utils.is_supportdyn env ty in
            let res2 =
              if supportdyn then begin
                TUtils.sub_type
                  env
                  widened_ty
                  (MakeType.supportdyn_mixed (Reason.witness p))
                @@ Some (Typing_error.Reasons_callback.unify_error_at p)
              end else
                (env, None)
            in
            (res2, widened_ty)
          | _ ->
            if force_solve then
              expand_type_and_solve env ~description_of_expected p ty
            else
              ((env, None), ty))
      in
      let ty_err_opt =
        Typing_error.union_opt
        @@ List.filter_map ~f:Fn.id [ty_err_opt1; ty_err_opt2]
      in
      ((env, ty_err_opt), ty)
  in

  let (env, ty) = Env.expand_type env ty in
  ((env, ty_err_opt), ty)

(* Solve type variables on top of stack, without losing completeness (by using
 * their variance), and pop variables off the stack
 *)
let close_tyvars_and_solve env =
  let old_env = env in
  let tyvars = Env.get_current_tyvars env in
  let env = Env.close_tyvars env in
  let (env, ty_errs) =
    List.fold_left tyvars ~init:(env, []) ~f:(fun (env, ty_errs) tyvar ->
        (*Printf.printf "tyvar = #%s\n" (Tvid.show tyvar);*)
        match solve_to_equal_bound_or_wrt_variance env Reason.none tyvar with
        | (env, Some ty_err) -> (env, ty_err :: ty_errs)
        | (env, _) -> (env, ty_errs))
  in
  let ty_err_opt = Typing_error.multiple_opt ty_errs in
  (Env.log_env_change "close_tyvars_and_solve" old_env env, ty_err_opt)

(* Currently, simplify_subtype doesn't look at bounds on type variables.
 * Let's at least notice when these bounds imply an equality.
 *)
let is_sub_type env ty1 ty2 =
  let ((env, ty_err_opt1), ty1) = expand_type_and_solve_eq env ty1 in
  let ((env, ty_err_opt2), ty2) = expand_type_and_solve_eq env ty2 in
  let ty_err_opt = Option.merge ty_err_opt1 ty_err_opt2 ~f:Typing_error.both in
  (TUtils.is_sub_type env ty1 ty2, ty_err_opt)

let try_bind_to_equal_bound env v =
  (* The reason we pass here doesn't matter since it's used only when `freshen` is true. *)
  try_bind_to_equal_bound ~freshen:false env Reason.none v

let bind_to_like_fresh env r var lower_bounds upper_bounds =
  let (env, fresh_ty) =
    Env.fresh_type_invariant
      env
      (Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos)
  in
  let like_ty = MakeType.locl_like r fresh_ty in
  let (env, _bind_ty_err) = bind env var like_ty in
  match get_node fresh_ty with
  | Tvar var' ->
    let env = Env.set_tyvar_lower_bounds env var' lower_bounds in
    let env = Env.set_tyvar_upper_bounds env var' upper_bounds in
    (env, like_ty)
  | _ -> failwith "fresh_type_invariant did not return a Tvar"

let expand_type_for_strip_dynamic env ty =
  let (env, ty) = Env.expand_type env ty in
  match deref ty with
  | (r, Tvar var) ->
    let is_locl_dynamic ty =
      match ty with
      | LoclType t -> is_dynamic t
      | _ -> false
    in
    let lower_bounds = Env.get_tyvar_lower_bounds env var in
    let upper_bounds = Env.get_tyvar_upper_bounds env var in
    let dynamic_is_lower_bound =
      ITySet.exists is_locl_dynamic lower_bounds
      && not (ITySet.for_all is_locl_dynamic lower_bounds)
    in
    if dynamic_is_lower_bound then begin
      (* We have dynamic, t1, ..., tn <: #0 <: u
       * Let's simplify to #0 := dynamic | #1
       * with t1, ..., tn <: #1 <: u
       *)
      bind_to_like_fresh
        env
        r
        var
        (ITySet.filter (fun t -> not (is_locl_dynamic t)) lower_bounds)
        upper_bounds
    end else
      (env, ty)
  | _ -> (env, ty)
