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
open Utils
open Typing_defs

module Reason = Typing_reason
module Unify = Typing_unify
module Env = Typing_env
module Subst = Decl_subst
module TUtils = Typing_utils
module SN = Naming_special_names
module Phase = Typing_phase
module TL = Typing_logic
module Cls = Typing_classes_heap
module TySet = Typing_set
module MakeType = Typing_make_type

type reactivity_extra_info = {
  method_info: ((* method_name *) string * (* is_static *) bool) option;
  class_ty: phase_ty option;
  parent_class_ty: phase_ty option
}

let empty_extra_info = {
  method_info = None;
  class_ty = None;
  parent_class_ty = None
}

module ConditionTypes = struct
  let try_get_class_for_condition_type (env: Env.env) (ty: decl ty) =
    match TUtils.try_unwrap_class_type ty with
    | None -> None
    | Some (_, ((_, x) as sid), _) ->
    begin match Env.get_class env x with
    | None -> None
    | Some cls -> Some (sid, cls)
    end

  let try_get_method_from_condition_type
    (env: Env.env)
    (ty: decl ty)
    (is_static: bool)
    (method_name: string) =
    match try_get_class_for_condition_type env ty with
    | Some (_, cls) ->
      let get = if is_static then Cls.get_smethod else Cls.get_method in
      get cls method_name
    | None -> None


  let localize_condition_type (env: Env.env) (ty: decl ty): locl ty =
    (* if condition type is generic - we cannot specify type argument in attribute.
       For cases when we check if containing type is a subtype of condition type
       if condition type is generic instantiate it with TAny's *)
    let ty =
      match try_get_class_for_condition_type env ty with
      | None -> ty
      | Some (_, cls) when Cls.tparams cls = [] -> ty
      | Some (((p, _) as sid), cls) ->
      let params =
        List.map (Cls.tparams cls)
          ~f:(fun { tp_name = (p,x); _ } -> Reason.Rwitness p, Tgeneric x) in
      let subst =
        Decl_instantiate.make_subst (Cls.tparams cls) [] in
      let ty = Reason.Rwitness p, (Tapply (sid, params)) in
      Decl_instantiate.instantiate subst ty in
    let ety_env = Phase.env_with_self env in
    let _, t = Phase.localize ~ety_env env ty in
    t
end

(* Given a pair of types `ty_sub` and `ty_super` attempt to apply simplifications
 * and add to the accumulated constraints in `constraints` any necessary and
 * sufficient [(t1,ck1,u1);...;(tn,ckn,un)] such that
 *   ty_sub <: ty_super iff t1 ck1 u1, ..., tn ckn un
 * where ck is `as` or `=`. Essentially we are making solution-preserving
 * simplifications to the subtype assertion, for now, also generating equalities
 * as well as subtype assertions, for backwards compatibility with use of
 * unification.
 *
 * If `constraints = []` is returned then the subtype assertion is valid.
 *
 * If the subtype assertion is unsatisfiable then return `failed = Some f`
 * where `f` is a `unit-> unit` function that records an error message.
 * (Sometimes we don't want to call this function e.g. when just checking if
 *  a subtype holds)
 *
 * If deep=true, elide singleton unions, treat invariant generics as both-ways
 * subtypes, and actually chase hierarchy for extends and implements.
 * For now, deep=false is used by subtype solving (conservative) and
 * deep=true is used for constraint decomposition (for instanceof).
 *
 * Annoyingly, we need to pass env back too, because Typing_phase.localize
 * expands type constants. (TODO: work out a better way of handling this)
 *
 * Special cases:
 *   If assertion is valid (e.g. string <: arraykey) then
 *     result can be the empty list (i.e. nothing is added to the result)
 *   If assertion is unsatisfiable (e.g. arraykey <: string) then
 *     we record this in the failed field of the result.
 *)
let with_error (f : unit -> unit) ((env, p) : (Env.env * TL.subtype_prop))
  : Env.env * TL.subtype_prop =
  env, TL.conj p (TL.Unsat f)

(* If `b` is false then fail with error function `f` *)
let check_with b f r = if not b then with_error f r else r

let valid env : Env.env * TL.subtype_prop = env, TL.valid

let (&&&) (env, p1) (f : Env.env -> Env.env * TL.subtype_prop) =
  if TL.is_unsat p1
  then env, p1
  else
    let env, p2 = f env in
    env, TL.conj p1 p2

let (|||) (env, p1) (f : Env.env -> Env.env * TL.subtype_prop) =
  if TL.is_valid p1
  then env, p1
  else
    let env, p2 = f env in
    env, TL.disj p1 p2

let if_unsat (f : unit -> Env.env * TL.subtype_prop) (env, p) =
  if TL.is_unsat p
  then f ()
  else env, p

(** Check that a mutability type is a subtype of another mutability type *)
let check_mutability
  ~(is_receiver: bool)
  (p_sub : Pos.t)
  (mut_sub: param_mutability option)
  (p_super : Pos.t)
  (mut_super: param_mutability option)
  env =
  let str m =
    match m with
    | None -> "immutable"
    | Some Param_owned_mutable -> "owned mutable"
    | Some Param_borrowed_mutable -> "mutable"
    | Some Param_maybe_mutable -> "maybe-mutable" in
  (* maybe-mutable <------immutable
                      |
                       <--mutable <-- owned mutable  *)
  match mut_sub, mut_super with
  (* immutable is not compatible with mutable *)
  | None, Some (Param_borrowed_mutable | Param_owned_mutable)
  (* mutable is not compatible with immutable  *)
  | Some (Param_borrowed_mutable | Param_owned_mutable), None
  (* borrowed mutable is not compatible with owned mutable *)
  | Some Param_borrowed_mutable, Some Param_owned_mutable
  (* maybe-mutable is not compatible with immutable/mutable *)
  | Some Param_maybe_mutable, (None | Some (Param_borrowed_mutable | Param_owned_mutable))
    ->
    env, TL.Unsat (fun () -> Errors.mutability_mismatch
      ~is_receiver p_sub (str mut_sub) p_super (str mut_super))
  | _ ->
    valid env

let empty_seen = Some SSet.empty

let log_subtype ~this_ty ~function_name env ty_sub ty_super =
  Typing_log.(log_with_level env "sub" 2 begin fun () ->
    let types =
      [Log_type ("ty_sub", ty_sub);
       Log_type ("ty_super", ty_super)]  in
    let types = Option.value_map this_ty ~default:types
      ~f:(fun ty -> Log_type ("this_ty", ty) :: types) in
    log_types (Reason.to_pos (fst ty_sub)) env
      [Log_head (function_name, types)]
  end)

let is_final_and_not_contravariant env id =
  let class_def = Env.get_class env id in
  match class_def with
  | Some class_ty -> TUtils.class_is_final_and_not_contravariant class_ty
  | None -> false

(* Process the constraint proposition *)
let rec process_simplify_subtype_result ~this_ty ~fail env prop =
  match prop with
  | TL.Unsat f ->
    f ();
    env
  | TL.IsSubtype (ty1, ty2) -> sub_type_inner_helper env ~this_ty ty1 ty2
  | TL.IsEqual (ty1, ty2) ->
    (* These come only from invariant generics, with new_inference=false *)
    fst (Unify.unify env ty2 ty1)
  | TL.Conj props ->
    (* Evaluates list from left-to-right so preserves order of conjuncts *)
    List.fold_left
      ~init:env
      ~f:(process_simplify_subtype_result ~this_ty ~fail)
      props
  | TL.Disj [prop] ->
    process_simplify_subtype_result ~this_ty ~fail env prop

  | TL.Disj props ->
    let rec try_disj props =
      match props with
      | [] ->
        fail ();
        env
      | prop :: props ->
        Errors.try_ begin fun () ->
          process_simplify_subtype_result ~this_ty ~fail env prop end
          (fun _ -> try_disj props)
    in
      try_disj props

(* Attempt to "solve" a subtype assertion ty_sub <: ty_super.
 * Return a proposition that is equivalent, but simpler, than
 * the original assertion. Fail with Unsat error_function if
 * the assertion is unsatisfiable. Some examples:
 *   string <: arraykey  ==>  True    (represented as Conj [])
 * (For covariant C and a type variable v)
 *   C<string> <: C<v>   ==>  string <: v
 * (Assuming that C does *not* implement interface J)
 *   C <: J              ==>  Unsat _
 * (Assuming we have T <: D in tpenv, and class D implements I)
 *   vec<T> <: vec<I>    ==>  True
 * This last one would be left as T <: I if seen_generic_params is None
 *)
and simplify_subtype
  ~(seen_generic_params : SSet.t option)
  ?(deep : bool = true)
  ~(no_top_bottom : bool)
  ?(error : (Env.env -> locl ty -> locl ty -> unit) option = None)
  ?(this_ty : locl ty option = None)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  env : Env.env * TL.subtype_prop =
  log_subtype ~this_ty ~function_name:"simplify_subtype" env ty_sub ty_super;
  let new_inference = TypecheckerOptions.new_inference (Env.get_tcopt env) in
  let env, ety_super = Env.expand_type env ty_super in
  let env, ety_sub = Env.expand_type env ty_sub in
  let uerror = match error with
    | Some f ->
      fun () -> f env ety_sub ety_super
    | None ->
      fun () -> TUtils.uerror env (fst ety_super) (snd ety_super) (fst ety_sub) (snd ety_sub) in
  (* We *know* that the assertion is unsatisfiable *)
  let invalid_with f = env, TL.Unsat f in
  let invalid () = invalid_with uerror in
  let invalid_env_with env f = env, TL.Unsat f in
  let invalid_env env = invalid_env_with env uerror in
  (* We *know* that the assertion is valid *)
  let valid () = env, TL.valid in
  (* We don't know whether the assertion is valid or not *)
  let default () =
    if new_inference
    then env, TL.IsSubtype (ety_sub, ety_super)
    else env, TL.IsSubtype (ty_sub, ty_super) in
  let simplify_subtype = simplify_subtype ~no_top_bottom ~error in
  let simplify_subtype_funs = simplify_subtype_funs ~no_top_bottom ~error in
  let simplify_subtype_variance = simplify_subtype_variance ~no_top_bottom ~error in
  let simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super env =
  begin match seen_generic_params with
  | None -> default ()
  | Some seen ->
    (* If we've seen this type parameter before then we must have gone
     * round a cycle so we fail
     *)
    if SSet.mem name_sub seen
    then invalid ()
    else
    (* If the generic is actually an expression dependent type,
      we need to update this_ty
    *)
    let this_ty = if AbstractKind.is_generic_dep_ty name_sub &&
      Option.is_none this_ty then Some ety_sub else this_ty in
    let seen_generic_params = Some (SSet.add name_sub seen) in
    (* Otherwise, we collect all the upper bounds ("as" constraints) on
       the generic parameter, and check each of these in turn against
       ty_super until one of them succeeds
     *)
    let rec try_bounds tyl env =
      match tyl with
      | [] ->
        (* Try an implicit mixed = ?nonnull bound before giving up.
           This can be useful when checking T <: t, where type t is
           equivalent to but syntactically different from ?nonnull.
           E.g., if t is a generic type parameter T with nonnull as
           a lower bound.
         *)
        let r = Reason.Rimplicit_upper_bound (Reason.to_pos (fst ety_sub), "?nonnull") in
        let tmixed = MakeType.mixed r in
        env |>
        simplify_subtype ~seen_generic_params ~deep ~this_ty tmixed ty_super

      | [ty] ->
        env |>
        simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super

      | ty::tyl ->
        env |>
        try_bounds tyl |||
        simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super
    in
      env |>
      try_bounds (Option.to_list opt_sub_cstr
        @ Typing_set.elements (Env.get_upper_bounds env name_sub)) |>
      (* Turn error into a generic error about the type parameter *)
      if_unsat invalid
    end in

  let simplify_subtype_generic_super ty_sub name_super env =
    begin match seen_generic_params with
    | None -> default ()
    | Some seen ->

    (* If we've seen this type parameter before then we must have gone
     * round a cycle so we fail
     *)
    if SSet.mem name_super seen
    then invalid ()
    else
      let seen_generic_params = Some (SSet.add name_super seen) in
      (* Collect all the lower bounds ("super" constraints) on the
       * generic parameter, and check ty_sub against each of them in turn
       * until one of them succeeds *)
      let rec try_bounds tyl env =
        match tyl with
        | [] ->
          invalid ()

        | ty::tyl ->
          env |>
          simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty |||
          try_bounds tyl
      in
        (* Turn error into a generic error about the type parameter *)
        env |>
        try_bounds (Typing_set.elements (Env.get_lower_bounds env name_super)) |>
        if_unsat invalid
     end in

  match snd ety_sub, snd ety_super with
  | Tvar _ ,_ | _, Tvar _ when not new_inference -> assert false

  | Tvar var_sub, Tvar var_super when var_sub = var_super -> valid ()

  (* Internally, newtypes and dependent types are always equipped with an upper bound.
   * In the case when no upper bound is specified in source code,
   * an implicit upper bound mixed = ?nonnull is added.
   *)
  | Tabstract ((AKnewtype _ | AKdependent _), None), _
  | _, Tabstract ((AKnewtype _ | AKdependent _), None) -> assert false

  | Terr, _ | _, Terr -> if no_top_bottom then default () else valid ()

  | (Tprim Nast.(Tint | Tbool | Tfloat | Tstring | Tresource | Tnum |
                 Tarraykey | Tnoreturn) |
     Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tabstract (AKenum _, _) |
     Tanon _ | Tobject | Tclass _ | Tarraykind _),
    Tnonnull ->
    valid ()
  | (Tdynamic | Toption _ | Tprim Nast.(Tnull | Tvoid)),
    Tnonnull ->
    invalid ()

  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tnonnull ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  | Tdynamic, Tdynamic ->
    valid ()
  | (Tnonnull | Toption _ | Tprim _ | Tfun _ | Ttuple _ | Tshape _ |
     Tabstract (AKenum _, _) | Tanon _ | Tobject | Tclass _ | Tarraykind _),
    Tdynamic ->
    invalid ()
  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tdynamic ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  (* everything subtypes mixed *)
  | _, Toption (_, Tnonnull) -> valid ()
  (* null is the type of null and is a subtype of any option type. *)
  | Tprim Nast.Tnull, Toption _ -> valid ()
  (* void behaves with respect to subtyping as an abstract type with
   * an implicit upper bound ?nonnull
   *)
  | Tprim Nast.Tvoid, Toption ty_super' ->
    let r = Reason.Rimplicit_upper_bound (Reason.to_pos (fst ety_sub), "?nonnull") in
    let tmixed = MakeType.mixed r in
    env |>
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super' |||
    simplify_subtype ~seen_generic_params ~deep ~this_ty tmixed ty_super
  | Tdynamic, Toption ty_super ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super env
  (* ?ty_sub' <: ?ty_super' iff ty_sub' <: ?ty_super'. Reasoning:
   * If ?ty_sub' <: ?ty_super', then from ty_sub' <: ?ty_sub' (widening) and transitivity
   * of <: it follows that ty_sub' <: ?ty_super'.  Conversely, if ty_sub' <: ?ty_super', then
   * by covariance and idempotence of ?, we have ?ty_sub' <: ??ty_sub' <: ?ty_super'.
   * Therefore, this step preserves the set of solutions.
   *)
  | Toption ty_sub', Toption _ ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub' ty_super env
  (* If ty_sub <: ?ty_super' and ty_sub does not contain null then we
   * must also have ty_sub <: ty_super'.  The converse follows by
   * widening and transitivity.  Therefore, this step preserves the set
   * of solutions.
   *)
  | (Tprim Nast.(Tint | Tbool | Tfloat | Tstring | Tresource | Tnum |
                 Tarraykey | Tnoreturn) |
     Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject |
     Tclass _ | Tarraykind _ | Tabstract (AKenum _, _) | Tany),
    Toption ty_super' ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super' env
  | Tabstract (AKnewtype (name_sub, _), _),
    Toption (_, Tabstract (AKnewtype (name_super, _), _) as ty_super')
    when name_super = name_sub ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super' env

  | Tabstract (AKdependent d_sub, Some bound_sub),
    Tabstract (AKdependent d_super, Some bound_super) ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    (* Dependent types are identical but bound might be different *)
    if d_sub = d_super
    then simplify_subtype ~seen_generic_params ~deep ~this_ty bound_sub bound_super env
    else simplify_subtype ~seen_generic_params ~deep ~this_ty bound_sub ty_super env

  (* This is sort of a hack because our handling of Toption is highly
   * dependent on how the type is structured. When we see a bare
   * dependent type we strip it off at this point since it shouldn't be
   * relevant to subtyping any more.
   *)

  | Tabstract (AKdependent (`expr _, []), Some ty_sub), (Tclass _ | Toption _) ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super env

  (* If t1 <: ?t2 and t1 is an abstract type constrained as t1',
   * then t1 <: t2 or t1' <: ?t2.  The converse is obviously
   * true as well.  We can fold the case where t1 is unconstrained
   * into the case analysis below.
   *)
  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Toption arg_ty_super ->
    env |>
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub arg_ty_super |||
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super
  (* If t1 <: ?t2, where t1 is guaranteed not to contain null, then
   * t1 <: t2, and the converse is obviously true as well.
   *)
  | Tabstract (AKgeneric name_sub, opt_sub_cstr), Toption arg_ty_super
    when Option.is_some seen_generic_params ->
    env |>
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub arg_ty_super |||
    (* Look up *)
    simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super

  | Tprim (Nast.Tint | Nast.Tfloat), Tprim Nast.Tnum -> valid ()
  | Tprim (Nast.Tint | Nast.Tstring), Tprim Nast.Tarraykey -> valid ()
  | Tprim p1, Tprim p2 ->
    if p1 = p2 then valid () else invalid ()
  | Tabstract ((AKenum _), _), Tprim Nast.Tarraykey ->
    valid ()
  | (Tnonnull | Tdynamic | Tfun _ | Ttuple _ | Tshape _ | Tanon _ |
     Tabstract (AKenum _, None) | Tobject | Tclass _ | Tarraykind _),
    Tprim _ ->
    invalid ()
  | Toption _,
    Tprim Nast.(Tvoid | Tint | Tbool | Tfloat | Tstring | Tresource | Tnum |
                Tarraykey | Tnoreturn) ->
    invalid ()
  | Toption ty_sub', Tprim Nast.Tnull ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub' ty_super env
  | Tabstract ((AKnewtype _ | AKenum _ | AKdependent _), Some ty), Tprim _ ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Ttuple _ | Tshape _ |
     Tabstract (AKenum _, _) | Tobject | Tclass _ | Tarraykind _), Tfun _ ->
    invalid ()
  | Tfun ft_sub, Tfun ft_super ->
    let r_sub, r_super = fst ety_sub, fst ety_super in
    simplify_subtype_funs ~seen_generic_params ~deep ~check_return:true
      r_sub ft_sub r_super ft_super env
  | Tanon (anon_arity, id), Tfun ft ->
    let r_sub, r_super = fst ety_sub, fst ety_super in
    begin match Env.get_anonymous env id with
      | None ->
        invalid_with (fun () -> Errors.anonymous_recursive_call (Reason.to_pos r_sub))
      | Some (reactivity, is_coroutine, ftys, _, anon) ->
        let p_super = Reason.to_pos r_super in
        let p_sub = Reason.to_pos r_sub in
        (* Add function type to set of types seen so far *)
        ftys := TUtils.add_function_type env ety_super !ftys;
        (env, TL.valid) |>
        check_with (subtype_reactivity env reactivity ft.ft_reactive
          || TypecheckerOptions.unsafe_rx (Env.get_tcopt env))
          (fun () -> Errors.fun_reactivity_mismatch
            p_super (TUtils.reactivity_to_string env reactivity)
            p_sub (TUtils.reactivity_to_string env ft.ft_reactive)) |>
        check_with (is_coroutine = ft.ft_is_coroutine) (fun () ->
          Errors.coroutinness_mismatch ft.ft_is_coroutine p_super p_sub) |>
        check_with (Unify.unify_arities
                  ~ellipsis_is_variadic:true anon_arity ft.ft_arity)
          (fun () -> Errors.fun_arity_mismatch p_super p_sub) |>
        (fun (env, prop) ->
          let env, _, ret = anon env ft.ft_params ft.ft_arity in
          (env, prop) &&&
          simplify_subtype ~seen_generic_params ~deep ~this_ty ret ft.ft_ret)
    end
  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tfun _ ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Tshape _ |
     Tabstract (AKenum _, _) | Tanon _ | Tobject | Tclass _ | Tarraykind _),
    Ttuple _ ->
    invalid ()
  (* (t1,...,tn) <: (u1,...,un) iff t1<:u1, ... , tn <: un *)
  | Ttuple tyl_sub, Ttuple tyl_super ->
    if List.length tyl_super = List.length tyl_sub
    then
      wfold_left2 (fun res ty_sub ty_super -> res
        &&& simplify_subtype ~seen_generic_params ~deep ty_sub ty_super)
        (env, TL.valid) tyl_sub tyl_super
    else invalid ()
  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Ttuple _ ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _ |
     Tabstract (AKenum _, _) | Tanon _ | Tobject | Tclass _ | Tarraykind _),
    Tshape _ ->
    invalid ()
  | Tshape (fields_known_sub, fdm_sub), Tshape (fields_known_super, fdm_super) ->
    let r_sub, r_super = fst ety_sub, fst ety_super in
      (**
       * shape_field_type A <: shape_field_type B iff:
       *   1. A is no more optional than B
       *   2. A's type <: B.type
       *)
      let on_common_field
          (env, acc) name
          { sft_optional = optional_super; sft_ty = ty_super }
          { sft_optional = optional_sub; sft_ty = ty_sub } =
        match optional_super, optional_sub with
          | true, _ | false, false ->
            (env, acc) &&& simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super
          | false, true ->
            (env, acc) |> with_error (fun () -> Errors.required_field_is_optional
              (Reason.to_pos r_sub)
              (Reason.to_pos r_super)
              (Env.get_shape_field_name name)) in
      let on_missing_omittable_optional_field res _ _ = res in
      let on_missing_non_omittable_optional_field
          res name { sft_ty = ty_super; _ } =
        let r = Reason.Rmissing_optional_field (
          Reason.to_pos r_sub,
          TUtils.get_printable_shape_field_name name
        ) in
        res &&& simplify_subtype ~seen_generic_params ~deep ~this_ty (MakeType.mixed r) ty_super in
      TUtils.apply_shape
        ~on_common_field
        ~on_missing_omittable_optional_field
        ~on_missing_non_omittable_optional_field
        ~on_error:(fun _ f -> invalid_with f)
        (env, TL.valid)
        (r_super, fields_known_super, fdm_super)
        (r_sub, fields_known_sub, fdm_sub)
  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tshape _ ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _ | Tshape _ |
     Tabstract (AKenum _, None) | Tanon _ | Tobject | Tclass _ | Tarraykind _),
    Tabstract (AKnewtype _, _) ->
    invalid ()
  | Tabstract (AKnewtype (name_sub, tyl_sub), _),
    Tabstract (AKnewtype (name_super, tyl_super), _)
    when name_super = name_sub ->
      let td = Env.get_typedef env name_super in
      begin match td with
        | Some {td_tparams; _} ->
          let variancel = List.map td_tparams (fun t -> t.tp_variance) in
          simplify_subtype_variance ~seen_generic_params ~deep name_sub variancel tyl_sub tyl_super env
        | None ->
          invalid ()
      end
  | Tabstract ((AKnewtype _ | AKenum _ | AKdependent _), Some ty), Tabstract (AKnewtype _, _) ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  | Tabstract (AKenum e_sub, _), Tabstract (AKenum e_super, _)
    when e_sub = e_super -> valid ()
  | Tclass ((_, class_name), _, _), Tabstract (AKenum enum_name, _)
    when enum_name = class_name -> valid ()
  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _ | Tshape _ |
     Tanon _ | Tobject | Tclass _ | Tarraykind _),
    Tabstract (AKenum _, _) ->
    invalid ()
  | Tabstract (AKenum _, None), Tabstract (AKenum _, _) -> invalid ()
  | Tabstract ((AKnewtype _ | AKenum _ | AKdependent _), Some ty), Tabstract (AKenum _, _) ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  | _, Tabstract (AKdependent _, Some (_, Tclass ((_, x), _, _) as ty))
    when is_final_and_not_contravariant env x ->
    (* For final class C, there is no difference between `this as X` and `X`,
     * and `expr<#n> as X` and `X`.
     * But we need to take care with contravariant classes, since we can't
     * statically guarantee their runtime type.
     *)
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty env

  (* Primitives and other concrete types cannot be subtypes of dependent types *)
  | (Tnonnull | Tdynamic | Tprim _ | Tfun _ | Ttuple _ | Tshape _ |
     Tabstract (AKenum _, None) | Tanon _ | Tclass _ | Tobject | Tarraykind _),
    Tabstract (AKdependent (expr_dep, _), tyopt) ->
    (* If the bound is the same class try and show more explanation of the error *)
    begin match snd ty_sub, tyopt with
    | Tclass ((_, y), _, _), Some (_, (Tclass ((_, x) as id, _, _))) when y = x ->
      invalid_with (fun () ->
      Errors.try_ uerror
        (fun error ->
           let p = Reason.to_pos (fst ety_sub) in
           if expr_dep = `cls x
           then Errors.exact_class_final id p error
           else Errors.this_final id p error))
    | _ -> invalid ()
    end

  | Tabstract ((AKnewtype _ | AKenum _), Some ty), Tabstract (AKdependent _, _) ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env
  | Toption _, Tabstract (AKdependent _, Some _) ->
    invalid ()

  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Ttuple _ | Tshape _ |
     Tabstract (AKenum _, _) | Tobject | Tclass _ | Tarraykind _),
    Tanon _ ->
    invalid ()
  | Tanon (_, id1), Tanon (_, id2) -> if id1 = id2 then valid () else invalid ()
  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tanon _ ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env
  | Tfun _, Tanon _ ->
    invalid ()

  | Tobject, Tobject -> valid ()
  (* Any class type is a subtype of object *)
  | Tclass _, Tobject -> valid ()
  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _ | Tshape _ |
     Tabstract (AKenum _, _) | Tanon _ | Tarraykind _),
    Tobject ->
    invalid ()
  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tobject ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  | Tabstract (AKenum _, _), Tclass ((_, class_name), _, [ty_super'])
    when class_name = SN.Classes.cHH_BuiltinEnum ->
    env |>
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super' &&&
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_super' ty_sub
  | Tabstract (AKenum enum_name, None), Tclass ((_, class_name), exact, _) ->
    if (enum_name = class_name || class_name = SN.Classes.cXHPChild) && exact = Nonexact
    then valid ()
    else invalid ()
  | Tabstract (AKenum enum_name, Some ty), Tclass ((_, class_name), exact, _) ->
    if enum_name = class_name && exact = Nonexact
    then valid ()
    else simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env
  | Tprim Nast.Tstring, Tclass ((_, class_name), exact, _) ->
    if (class_name = SN.Classes.cStringish || class_name = SN.Classes.cXHPChild)
      && exact = Nonexact
    then valid ()
    else invalid ()
  | Tprim Nast.(Tarraykey | Tint | Tfloat | Tnum), Tclass ((_, class_name), exact, _) ->
    if class_name = SN.Classes.cXHPChild && exact = Nonexact then valid () else invalid ()
  | (Tnonnull | Tdynamic | Tprim Nast.(Tnull | Tvoid | Tbool | Tresource | Tnoreturn) |
     Toption _ | Tfun _ | Ttuple _ | Tshape _ | Tanon _),
    Tclass _ ->
    invalid ()
  (* Match what's done in unify for non-strict code *)
  | Tobject, Tclass _ ->
    if Env.is_strict env then invalid () else valid ()
  | Tclass (x_sub, exact_sub, tyl_sub), Tclass (x_super, exact_super, tyl_super) ->
    let exact_match =
      match exact_sub, exact_super with
      | Nonexact, Exact -> false
      | _, _ -> true in
    let p_sub, p_super = fst ety_sub, fst ety_super in
    let cid_super, cid_sub = (snd x_super), (snd x_sub) in
    (* This is side-effecting as it registers a dependency *)
    let class_def_sub = Env.get_class env cid_sub in
    if cid_super = cid_sub
    then
      (* If class is final then exactness is superfluous *)
      let is_final =
        match class_def_sub with
        | Some tc -> Cls.final tc
        | None -> false in
      if not (exact_match || is_final)
      then invalid ()
      else
      (* We handle the case where a generic A<T> is used as A *)
      let tyl_super =
        if List.is_empty tyl_super && not (Env.is_strict env)
        then List.map tyl_sub (fun _ -> (p_super, Tany))
        else tyl_super in
      let tyl_sub =
        if List.is_empty tyl_sub && not (Env.is_strict env)
        then List.map tyl_super (fun _ -> (p_super, Tany))
        else tyl_sub in
      if List.length tyl_sub <> List.length tyl_super
      then begin
        let n_sub = String_utils.soi (List.length tyl_sub) in
        let n_super = String_utils.soi (List.length tyl_super) in
        invalid_with (fun () ->
          Errors.type_arity_mismatch (fst x_super) n_super (fst x_sub) n_sub)
      end
      else
      if List.is_empty tyl_sub && List.is_empty tyl_super
      then valid ()
      else
        let variancel =
          match class_def_sub with
          | None ->
            List.map tyl_sub (fun _ -> Ast.Invariant)
          | Some class_sub ->
            List.map (Cls.tparams class_sub) (fun t -> t.tp_variance) in

          (* C<t1, .., tn> <: C<u1, .., un> iff
           *   t1 <:v1> u1 /\ ... /\ tn <:vn> un
           * where vi is the variance of the i'th generic parameter of C,
           * and <:v denotes the appropriate direction of subtyping for variance v
           *)
          simplify_subtype_variance ~seen_generic_params ~deep
            cid_sub variancel tyl_sub tyl_super env
    else
      if not exact_match
      then invalid ()
      else
      begin match class_def_sub with
      | None ->
        (* This should have been caught already in the naming phase *)
        valid ()

      | Some class_sub ->
        (* We handle the case where a generic A<T> is used as A *)
        let tyl_sub =
          if List.is_empty tyl_sub && not (Env.is_strict env)
          then List.map (Cls.tparams class_sub) (fun _ -> (p_sub, Tany))
          else tyl_sub in
        if List.length (Cls.tparams class_sub) <> List.length tyl_sub
        then
          invalid_with (fun () ->
          Errors.expected_tparam ~definition_pos:(Cls.pos class_sub)
            ~use_pos:(Reason.to_pos p_sub) (List.length (Cls.tparams class_sub)))
        else
          let ety_env =
          {
            type_expansions = [];
            substs = Subst.make (Cls.tparams class_sub) tyl_sub;
            (* TODO: do we need this? *)
            this_ty = Option.value this_ty ~default:ty_sub;
            from_class = None;
            validate_dty = None;
          } in
            let up_obj = Cls.get_ancestor class_sub cid_super in
            match up_obj with
            | Some up_obj ->
              let env, up_obj = Phase.localize ~ety_env env up_obj in
              simplify_subtype ~seen_generic_params ~deep ~this_ty up_obj ty_super env
            | None ->
              if Cls.kind class_sub = Ast.Ctrait || Cls.kind class_sub = Ast.Cinterface then
              let rec try_reqs reqs env =
                match Sequence.next reqs with
                | None ->
                  (* It's crucial that we don't lose updates to global_tpenv in env that were
                   * introduced by PHase.localize. TODO: avoid this requirement *)
                  invalid_env env

                | Some ((_, req_type), reqs) ->

                (* a trait is never the runtime type, but it can be used
                 * as a constraint if it has requirements for its using
                 * classes *)
                  let env, req_type = Phase.localize ~ety_env env req_type in
                  env |>
                  simplify_subtype ~seen_generic_params ~deep ~this_ty
                    (p_sub, snd req_type) ty_super
                  ||| try_reqs reqs
              in
                try_reqs (Cls.all_ancestor_reqs class_sub) env
              else invalid ()
      end
  | Tabstract ((AKnewtype _), Some ty), Tclass _ ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env
  | Tarraykind _, Tclass ((_, class_name), Nonexact, _)
    when class_name = SN.Classes.cXHPChild -> valid ()
  | Tarraykind akind, Tclass ((_, coll), Nonexact, [tv_super])
    when (coll = SN.Collections.cTraversable ||
          coll = SN.Rx.cTraversable ||
          coll = SN.Collections.cContainer) ->
      (match akind with
        (* array <: Traversable<_> and emptyarray <: Traversable<t> for any t *)
      | AKany ->
        simplify_subtype ~seen_generic_params ~deep ~this_ty (fst ety_sub, Tany) tv_super env
      | AKempty -> valid ()
      (* vec<tv> <: Traversable<tv_super>
       * iff tv <: tv_super
       * Likewise for vec<tv> <: Container<tv_super>
       *          and map<_,tv> <: Traversable<tv_super>
       *          and map<_,tv> <: Container<tv_super>
       *)
      | AKvarray tv
      | AKvec tv
      | AKdarray (_, tv)
      | AKvarray_or_darray tv
      | AKmap (_, tv) -> simplify_subtype ~seen_generic_params ~deep ~this_ty tv tv_super env
    )
  | Tarraykind akind, Tclass ((_, coll), Nonexact, [tk_super; tv_super])
    when (coll = SN.Collections.cKeyedTraversable
         || coll = SN.Rx.cKeyedTraversable
         || coll = SN.Collections.cKeyedContainer) ->
    let r = fst ety_sub in
      (match akind with
      | AKany ->
        env |>
        simplify_subtype ~seen_generic_params ~deep ~this_ty (fst ety_sub, Tany) tk_super &&&
        simplify_subtype ~seen_generic_params ~deep ~this_ty (fst ety_sub, Tany) tv_super
      | AKempty -> valid ()
      | AKvarray tv
      | AKvec tv ->
        env |>
        simplify_subtype ~seen_generic_params ~deep ~this_ty (MakeType.int r) tk_super &&&
        simplify_subtype ~seen_generic_params ~deep ~this_ty tv tv_super
      | AKvarray_or_darray tv ->
        let tk_sub = MakeType.arraykey (Reason.Rvarray_or_darray_key (Reason.to_pos r)) in
        env |>
        simplify_subtype ~seen_generic_params ~deep ~this_ty tk_sub tk_super &&&
        simplify_subtype ~seen_generic_params ~deep ~this_ty tv tv_super
      | AKdarray (tk, tv)
      | AKmap (tk, tv) ->
        env |>
        simplify_subtype ~seen_generic_params ~deep ~this_ty tk tk_super &&&
        simplify_subtype ~seen_generic_params ~deep ~this_ty tv tv_super
      )
  | Tarraykind _, Tclass ((_, coll), Nonexact, [])
    when (coll = SN.Collections.cKeyedTraversable ||
          coll = SN.Rx.cKeyedTraversable ||
          coll = SN.Collections.cKeyedContainer) ->
    (* All arrays are subtypes of the untyped KeyedContainer / Traversables *)
    valid ()
  | Tarraykind _, Tclass _ -> invalid ()
  | Tabstract (AKdependent _, Some ty), Tclass _ ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env

  (* Arrays *)
  | Ttuple _, Tarraykind AKany ->
    if TypecheckerOptions.disallow_array_as_tuple (Env.get_tcopt env)
    then invalid ()
    else valid ()
  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _ |
     Tshape _ | Tabstract (AKenum _, _) | Tanon _ | Tobject | Tclass _),
    Tarraykind _ ->
    invalid ()
  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tarraykind _ ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env
  | Tarraykind ak_sub, Tarraykind ak_super ->
    let r = fst ety_sub in
    begin match ak_sub, ak_super with
    (* An array of any kind is a subtype of an array of AKany *)
    | _, AKany ->
      valid ()

    (* An empty array is a subtype of any array type *)
    | AKempty, _ ->
      valid ()

    (* array is a subtype of varray_or_darray<_> *)
    | AKany, AKvarray_or_darray (_, Tany) ->
      valid ()

    | AKany, _ ->
      let safe_array = TypecheckerOptions.safe_array (Env.get_tcopt env) in
      if safe_array then invalid () else valid ()

    (* varray_or_darray<ty1> <: varray_or_darray<ty2> iff t1 <: ty2
       But, varray_or_darray<ty1> is never a subtype of a vect-like array *)
    | AKvarray_or_darray ty_sub, AKvarray_or_darray ty_super ->
      simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super env

    | (AKvarray ty_sub | AKvec ty_sub),
      (AKvarray ty_super | AKvec ty_super | AKvarray_or_darray ty_super) ->
      simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super env

    | (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub)),
      (AKdarray (tk_super, tv_super) | AKmap (tk_super, tv_super)) ->
      env |>
      simplify_subtype ~seen_generic_params ~deep ~this_ty tk_sub tk_super &&&
      simplify_subtype ~seen_generic_params ~deep ~this_ty tv_sub tv_super

    | (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub)),
      (AKvarray_or_darray tv_super) ->
      let tk_super = MakeType.arraykey (Reason.Rvarray_or_darray_key (Reason.to_pos (fst ety_super))) in
      env |>
      simplify_subtype ~seen_generic_params ~deep ~this_ty tk_sub tk_super &&&
      simplify_subtype ~seen_generic_params ~deep ~this_ty tv_sub tv_super

    | (AKvarray elt_ty | AKvec elt_ty), (AKdarray _ | AKmap _)
        when not (TypecheckerOptions.safe_vector_array (Env.get_tcopt env)) ->
          let int_reason = Reason.Ridx (Reason.to_pos r, Reason.Rnone) in
          let int_type = MakeType.int int_reason in
          simplify_subtype ~seen_generic_params ~deep ~this_ty
            (r, Tarraykind (AKmap (int_type, elt_ty))) ty_super env

    (* any other array subtyping is unsatisfiable *)
    | _ ->
      invalid ()
    end

  (* ty_sub <: union{ty_super'} iff ty_sub <: ty_super' *)
  | _, Tunresolved [ty_super'] when deep ->
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty_super' env

  (* t1 | ... | tn <: t
   *   if and only if
   * t1 <: t /\ ... /\ tn <: t
   * We want this even if t is a type variable e.g. consider
   *   int | v <: v
   *)
  | Tunresolved tyl, _->
    if new_inference
    then
      List.fold_left tyl ~init:(env, TL.valid) ~f:(fun res ty_sub ->
        res &&& simplify_subtype ~seen_generic_params ~deep ty_sub ty_super)
    else default ()

  (* We want to treat nullable as a union with the same rule as above.
   * This is only needed for Tvar on right; other cases are dealt with specially as
   * derived rules.
   *)
  | Toption t, Tvar _ when new_inference ->
    env |>
    simplify_subtype ~seen_generic_params ~deep ~this_ty t ty_super &&&
    simplify_subtype ~seen_generic_params ~deep ~this_ty (MakeType.null (fst ety_sub)) ty_super

  | Tvar _, _ | _, Tvar _ ->
    default ()

  (* Num is not atomic: it is equivalent to int|float. The rule below relies
   * on ty_sub not being a union e.g. consider num <: arraykey | float, so
   * we break out num first.
   *)
  | Tprim Nast.Tnum, Tunresolved _ when new_inference ->
    let r = fst ty_sub in
    env |>
    simplify_subtype ~seen_generic_params ~deep ~this_ty (MakeType.float r) ty_super &&&
    simplify_subtype ~seen_generic_params ~deep ~this_ty (MakeType.int r) ty_super

  | Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tunresolved [] ->
    if new_inference
    then simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super env
    else default ()
  | Tabstract (AKgeneric name_sub, opt_sub_cstr), Tunresolved [] ->
    if new_inference
    then simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super env
    else default ()
  | (Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _ | Tshape _ |
     Tanon _ | Tobject | Tclass _ | Tarraykind _ | Tabstract (AKenum _, _)),
    Tunresolved [] ->
    if new_inference
    then invalid ()
    else default ()

  | _, Tunresolved (_ :: _ as tyl) ->
    (* It's sound to reduce t <: t1 | t2 to (t <: t1) || (t <: t2). But
     * not complete e.g. consider (t1 | t3) <: (t1 | t2) | (t2 | t3).
     * But we deal with unions on the left first (see case above), so this
     * particular situation won't arise.
     * TODO: identify under what circumstances this reduction is complete.
     *)
    if new_inference
    then
    let rec try_each tys env =
      match tys with
      | [] ->
        invalid ()

      | ty::tys ->
        env |>
        simplify_subtype ~seen_generic_params ~deep ~this_ty ty_sub ty
        ||| try_each tys
    in
      try_each tyl env
    else default ()

  | _, Tany | Tany, _ ->
    if new_inference && not no_top_bottom then valid () else default ()

  (* If subtype and supertype are the same generic parameter, we're done *)
  | Tabstract (AKgeneric name_sub, _), Tabstract (AKgeneric name_super, _)
       when name_sub = name_super
    -> valid ()

  (* Supertype is generic parameter *and* subtype is a newtype with bound.
   * We need to make this a special case because there is a *choice*
   * of subtyping rule to apply. See details in the case of dependent type
   * against generic parameter which is similar
   *)
  | Tabstract (AKnewtype (_, _), Some ty), Tabstract (AKgeneric name_super, _)
    when Option.is_some seen_generic_params ->
    env |>
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super |||
    simplify_subtype_generic_super ty_sub name_super

  (* void behaves with respect to subtyping as an abstract type with
   * an implicit upper bound ?nonnull
   *)
  | Tprim Nast.Tvoid, Tabstract (AKgeneric name_super, _)
    when Option.is_some seen_generic_params ->
    let r = Reason.Rimplicit_upper_bound (Reason.to_pos (fst ety_sub), "?nonnull") in
    let tmixed = (r, Toption (r, Tnonnull)) in
    env |>
    simplify_subtype ~seen_generic_params ~deep ~this_ty tmixed ty_super |||
    simplify_subtype_generic_super ty_sub name_super

  (* Supertype is generic parameter *and* subtype is dependent.
   * We need to make this a special case because there is a *choice*
   * of subtyping rule to apply.
   *
   * Example. First suppose that we have the definition
   *
   *     abstract const type TC as C
   *
   * (1) Now suppose we have to check
   *       this::TC <: Tu
   *     where we have the constraint
   *       <Tu super C>.
   *     Then it's necessary to apply the rule for AKdependent first, so we
   *     reduce this problem to
   *       C <: Tu
   *     and then call sub_generic_params to deal with the type parameter.
   * (2) Alternatively, suppose we again have to check
   *       this::TC <: Tu
   *     but this time we have the constraint
   *       <Tu super this::TC>.
   *    Then if we first reduce the problem to C <: Tu we fail;
   *    but if we also try sub_generic_params then we succeed, because
   *    we end up checking this::TC <: this::TC.
  *)
  | Tabstract (AKdependent _, Some ty), Tabstract (AKgeneric name_super, _)
    when Option.is_some seen_generic_params ->
    env |>
    simplify_subtype_generic_super ty_sub name_super |||
    (let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~deep ~this_ty ty ty_super)

  (* Subtype or supertype is generic parameter
   * We delegate these cases to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
   *)
  | Tabstract (AKgeneric name_sub, opt_sub_cstr), _ ->
    simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super env

  | _, Tabstract (AKgeneric name_super, _) ->
    simplify_subtype_generic_super ty_sub name_super env

and simplify_subtype_variance
  ~(seen_generic_params : SSet.t option)
  ~(deep : bool)
  ~(no_top_bottom : bool)
  ?(error : (Env.env -> locl ty -> locl ty -> unit) option = None)
  (cid : string)
  (variancel : Ast.variance list)
  (children_tyl : locl ty list)
  (super_tyl : locl ty list)
  : Env.env -> Env.env * TL.subtype_prop
  = fun env ->
  let simplify_subtype = simplify_subtype ~no_top_bottom ~seen_generic_params
    ~deep ~error ~this_ty:None in
  let simplify_subtype_variance = simplify_subtype_variance ~no_top_bottom
    ~seen_generic_params ~deep in
  match variancel, children_tyl, super_tyl with
  | [], _, _
  | _, [], _
  | _, _, [] -> valid env
  | variance :: variancel, child :: childrenl, super :: superl ->
      begin match variance with
      | Ast.Covariant ->
        simplify_subtype child super env
      | Ast.Contravariant ->
        let super = (Reason.Rcontravariant_generic (fst super,
          Utils.strip_ns cid), snd super) in
        simplify_subtype super child env
      | Ast.Invariant ->
        let super' = (Reason.Rinvariant_generic (fst super,
          Utils.strip_ns cid), snd super) in
        if deep
        then
          env |>
          simplify_subtype child super' &&&
          simplify_subtype super' child
        else
          env, TL.IsEqual (child, super')
      end &&&
      simplify_subtype_variance cid variancel childrenl superl

and simplify_subtype_params
  ~(seen_generic_params : SSet.t option)
  ~(deep : bool)
  ~(no_top_bottom : bool)
  ?(is_method : bool = false)
  ?(check_params_reactivity = false)
  ?(check_params_mutability = false)
  (subl : locl fun_param list)
  (superl : locl fun_param list)
  (variadic_sub_ty : locl ty option)
  (variadic_super_ty : locl ty option)
  env =

  let simplify_subtype = simplify_subtype ~seen_generic_params ~no_top_bottom ~deep in
  let simplify_subtype_params = simplify_subtype_params ~seen_generic_params
    ~no_top_bottom ~deep in
  let simplify_subtype_params_with_variadic = simplify_subtype_params_with_variadic
    ~seen_generic_params ~no_top_bottom ~deep in
  let simplify_supertype_params_with_variadic = simplify_supertype_params_with_variadic
    ~seen_generic_params ~no_top_bottom ~deep in
  match subl, superl with
  (* When either list runs out, we still have to typecheck that
  the remaining portion sub/super types with the other's variadic.
  For example, if
  ChildClass {
    public function a(int $x = 0, string ... $args) // superl = [int], super_var = string
  }
  overrides
  ParentClass {
    public function a(string ... $args) // subl = [], sub_var = string
  }
  , there should be an error because the first argument will be checked against
  int, not string that is, ChildClass::a("hello") would crash,
  but ParentClass::a("hello") wouldn't.

  Similarly, if the other list is longer, aka
  ChildClass  extends ParentClass {
    public function a(mixed ... $args) // superl = [], super_var = mixed
  }
  overrides
  ParentClass {
    //subl = [string], sub_var = string
    public function a(string $x = 0, string ... $args)
  }
  It should also check that string is a subtype of mixed.
  *)
  | [], _ -> (match variadic_super_ty with
    | None -> valid env
    | Some ty -> simplify_supertype_params_with_variadic superl ty env)
  | _, [] -> (match variadic_sub_ty with
    | None -> valid env
    | Some ty -> simplify_subtype_params_with_variadic subl ty env)
  | sub :: subl, super :: superl ->
    env |>
    begin
      if check_params_reactivity
      then subtype_fun_params_reactivity sub super
      else valid
    end &&&
    begin
      if check_params_mutability
      then check_mutability
        ~is_receiver:false
        sub.fp_pos sub.fp_mutability super.fp_pos super.fp_mutability
      else valid
    end &&&

    fun env ->
    begin
      let { fp_type = ty_sub; _ } = sub in
      let { fp_type = ty_super; _ } = super in
      (* Check that the calling conventions of the params are compatible.
       * We don't currently raise an error for reffiness because function
       * hints don't support '&' annotations (enforce_ctpbr = false). *)
      Unify.unify_param_modes ~enforce_ctpbr:is_method sub super;
      Unify.unify_accept_disposable sub super;
      let env = { env with Env.pos = Reason.to_pos (fst ty_sub) } in
      match sub.fp_kind, super.fp_kind with
      | FPinout, FPinout ->
        (* Inout parameters are invariant wrt subtyping for function types. *)
        env |>
        simplify_subtype ty_super ty_sub &&&
        simplify_subtype ty_sub ty_super
      | _ ->
        env |>
        simplify_subtype ty_sub ty_super
      end &&&
    simplify_subtype_params ~is_method subl superl
      variadic_sub_ty variadic_super_ty

and simplify_subtype_params_with_variadic
  ~(seen_generic_params : SSet.t option)
  ~(deep : bool)
  ~(no_top_bottom : bool)
  (subl : locl fun_param list)
  (variadic_ty : locl ty)
  env =
  let simplify_subtype = simplify_subtype ~seen_generic_params ~no_top_bottom ~deep in
  let simplify_subtype_params_with_variadic = simplify_subtype_params_with_variadic
    ~seen_generic_params ~no_top_bottom ~deep in
  match subl with
  | [] -> valid env
  | { fp_type = sub; _ } :: subl ->
    let env = { env with Env.pos = Reason.to_pos (fst sub) } in
    env |>
    simplify_subtype sub variadic_ty &&&
    simplify_subtype_params_with_variadic subl variadic_ty

and simplify_supertype_params_with_variadic
  ~(seen_generic_params : SSet.t option)
  ~(deep : bool)
  ~(no_top_bottom : bool)
  (superl : locl fun_param list)
  (variadic_ty : locl ty)
  env =
  let simplify_subtype = simplify_subtype ~seen_generic_params ~no_top_bottom ~deep in
  let simplify_supertype_params_with_variadic = simplify_supertype_params_with_variadic
    ~seen_generic_params ~no_top_bottom ~deep in
  match superl with
  | [] -> valid env
  | { fp_type = super; _ } :: superl ->
    let env = { env with Env.pos = Reason.to_pos (fst super) } in
    env |>
    simplify_subtype variadic_ty super &&&
    simplify_supertype_params_with_variadic superl variadic_ty

and subtype_reactivity
  ?(extra_info: reactivity_extra_info option)
  ?(is_call_site = false)
  (env : Env.env)
  (r_sub : reactivity)
  (r_super : reactivity) : bool =

  let maybe_localize t =
    match t with
    | DeclTy t ->
      let ety_env = Phase.env_with_self env in
      let _, t = Phase.localize ~ety_env env t in
      t
    | LoclTy t -> t in

  let class_ty =
    Option.bind extra_info (fun { class_ty = cls; _ } ->
      Option.map cls ~f:maybe_localize) in

  (* for method declarations check if condition type for r_super includes
     reactive method with a matching name. If yes - then it will act as a guarantee
     that derived class will have to redefine the method with a shape required
     by condition type (reactivity of redefined method must be subtype of reactivity
     of method in interface) *)
  let condition_type_has_matching_reactive_method condition_type_super (method_name, is_static) =
    let m =
      ConditionTypes.try_get_method_from_condition_type
        env condition_type_super is_static method_name in
    match m with
    | Some { ce_type = lazy (_, Tfun f); _  } ->
      (* check that reactivity of interface method (effectively a promised
         reactivity of a method in derived class) is a subtype of r_super.
         NOTE: we check only for unconditional reactivity since conditional
         version does not seems to yield a lot and will requre implementing
         cycle detection for condition types *)
      begin match f.ft_reactive with
      | Reactive None | Shallow None | Local None ->
        let extra_info = {
          empty_extra_info with parent_class_ty = Some (DeclTy condition_type_super)
        } in
        subtype_reactivity ~extra_info env f.ft_reactive r_super
      | _ -> false
      end
    | _ -> false in
  match r_sub, r_super, extra_info with
  (* anything is a subtype of nonreactive functions *)
  | _, Nonreactive, _ -> true
  (* to compare two maybe reactive values we need to unwrap them *)
  | MaybeReactive sub, MaybeReactive super, _ ->
    subtype_reactivity ?extra_info ~is_call_site env sub super
  (* for explicit checks at callsites implicitly unwrap maybereactive value:
     function f(<<__AtMostRxAsFunc>> F $f)
     f(<<__RxLocal>> () ==> {... })
     here parameter will be maybereactive and argument - rxlocal
     *)
  | sub, MaybeReactive super, _ when is_call_site ->
    subtype_reactivity ?extra_info ~is_call_site env sub super
  (* if is_call_site is falst ignore maybereactive flavors.
     This usually happens during subtype checks for arguments and when target
     function is conditionally reactive we'll do the proper check
     in typing_reactivity.check_call. *)
  | _, MaybeReactive _, _ when not is_call_site -> true
  (* ok:
    class A { function f((function(): int) $f) {} }
    class B extends A {
      <<__Rx>>
      function f(<<__AtMostRxAsFunc>> (function(): int) $f);
    }
    reactivity for arguments is checked contravariantly *)
  | _, RxVar None, _
  (* ok:
     <<__Rx>>
     function f(<<__AtMostRxAsFunc>> (function(): int) $f) { return $f() }  *)
  | RxVar None, RxVar _, _ -> true
  | RxVar (Some sub), RxVar (Some super), _
  | sub, RxVar (Some super), _ ->
    subtype_reactivity ?extra_info ~is_call_site env sub super
  | RxVar _, _, _ -> false
  | (Local cond_sub | Shallow cond_sub | Reactive cond_sub), Local cond_super, _
  | (Shallow cond_sub | Reactive cond_sub), Shallow cond_super, _
  | Reactive cond_sub, Reactive cond_super, _
    when subtype_param_rx_if_impl ~is_param:false env cond_sub class_ty cond_super ->
    true
  (* function type TSub of method M with arbitrary reactivity in derive class
     can be subtype of conditionally reactive function type TSuper of method M
     defined in base class when condition type has reactive method M.
     interface Rx {
       <<__Rx>>
       public function f(): int;
     }
     class A {
       <<__RxIfImplements(Rx::class)>>
       public function f(): int { ... }
     }
     class B extends A {
       public function f(): int { ... }
     }
     This should be OK because:
     - B does not implement Rx (B::f is not compatible with Rx::f) which means
     that calling ($b : B)->f() will not be treated as reactive
     - if one of subclasses of B will decide to implement B - they will be forced
     to redeclare f which now will shadow B::f. Note that B::f will still be
     accessible as parent::f() but will be treated as non-reactive call.
     *)
  | _, (Reactive (Some t) | Shallow (Some t) | Local (Some t)), Some { method_info = Some mi; _ }
    when condition_type_has_matching_reactive_method t mi ->
    true
  (* call_site specific cases *)
  (* shallow can call into local *)
  | Local cond_sub, Shallow cond_super, _ when
    is_call_site &&
    subtype_param_rx_if_impl ~is_param:false env cond_sub class_ty cond_super ->
    true
  (* local can call into non-reactive *)
  | Nonreactive, Local _, _ when is_call_site -> true
  | _ -> false

and should_check_fun_params_reactivity
  (ft_super: locl fun_type) = ft_super.ft_reactive <> Nonreactive

(* checks condition described by OnlyRxIfImpl condition on parameter is met  *)
and subtype_param_rx_if_impl
  ~is_param
  (env: Env.env)
  (cond_type_sub: decl ty option)
  (declared_type_sub: locl ty option)
  (cond_type_super: decl ty option) =
  let cond_type_sub =
    Option.map cond_type_sub ~f:(ConditionTypes.localize_condition_type env) in
  let cond_type_super =
    Option.map cond_type_super ~f:(ConditionTypes.localize_condition_type env) in
  match cond_type_sub, cond_type_super with
  (* no condition types - do nothing *)
  | None, None -> true
  (* condition type is specified only for super - ok for receiver case (is_param is false)
    abstract class A {
      <<__RxLocal, __OnlyRxIfImpl(Rx1::class)>>
      public abstract function condlocalrx(): int;
    }
    abstract class B extends A {
      // ok to override cond local with local (if condition is not met - method
      // in base class is non-reactive )
      <<__Override, __RxLocal>>
      public function condlocalrx(): int {
        return 1;
      }
    }
    for parameters we need to verify that declared type of sub is a subtype of
    conditional type for super. Here is an example where this is violated:

    interface A {}
    interface RxA {}

    class C1 {
      <<__Rx>>
      public function f(A $a): void {
      }
    }

    class C2 extends C1 {
      // ERROR: invariant f body is reactive iff $a instanceof RxA can be violated
      <<__Rx, __AtMostRxAsArgs>>
      public function f(<<__OnlyRxIfImpl(RxA::class)>>A $a): void {
      }
    }
    here declared type of sub is A
    and cond type of super is RxA
  *)
  | None, Some _ when not is_param -> true
  | None, Some cond_type_super ->
    Option.value_map declared_type_sub
      ~default:false
      ~f:(fun declared_type_sub -> is_sub_type env declared_type_sub cond_type_super)
  (* condition types are set for both sub and super types: contravariant check
    interface A {}
    interface B extends A {}
    interface C extends B {}

    interface I1 {
      <<__Rx, __OnlyRxIfImpl(B::class)>>
      public function f(): void;
      <<__Rx>>
      public function g(<<__OnlyRxIfImpl(B::class)>> A $a): void;
    }
    interface I2 extends I1 {
      // OK since condition in I1::f covers I2::f
      <<__Rx, __OnlyRxIfImpl(A::class)>>
      public function f(): void;
      // OK since condition in I1::g covers I2::g
      <<__Rx>>
      public function g(<<__OnlyRxIfImpl(A::class)>> A $a): void;
    }
    interface I3 extends I1 {
      // Error since condition in I1::f is less strict that in I3::f
      <<__Rx, __OnlyRxIfImpl(C::class)>>
      public function f(): void;
      // Error since condition in I1::g is less strict that in I3::g
      <<__Rx>>
      public function g(<<__OnlyRxIfImpl(C::class)>> A $a): void;
    }
   *)
  | Some cond_type_sub, Some cond_type_super ->
    is_sub_type env cond_type_super cond_type_sub
  (* condition type is set for super type, check if declared type of
     subtype is a subtype of condition type
     interface Rx {
       <<__Rx>>
       public function f(int $a): void;
     }
     class A<T> {
       <<__Rx, __OnlyRxIfImpl(Rx::class)>>
       public function f(T $a): void {
       }
     }
     // B <: Rx so B::f is completely reactive
     class B extends A<int> implements Rx {
     } *)
  | Some cond_type_sub, None ->
    Option.value_map declared_type_sub
      ~default:false
      ~f:(fun declared_type_sub -> is_sub_type env declared_type_sub cond_type_sub)

(* checks reactivity conditions for function parameters *)
and subtype_fun_params_reactivity
  (p_sub: locl fun_param)
  (p_super: locl fun_param)
  env =
  match p_sub.fp_rx_annotation, p_super.fp_rx_annotation with
  (* no conditions on parameters - do nothing *)
  | None, None -> valid env
  (* both parameters are conditioned to be rx function - no need to check anything *)
  | Some Param_rx_var, Some Param_rx_var -> valid env
  | None, Some Param_rx_var ->
    (* parameter is conditionally reactive in supertype and missing condition
      in subtype - this is ok only if parameter in subtype is reactive
      <<__Rx>>
      function super((function(): int) $f);
      <<__Rx>>
      function sub(<<__AtMostRxAsFunc>> (function(): int) $f);
      We check if sub <: super. parameters are checked contravariantly
      so we need to verify that
      (function(): int) $f <: <<__AtMostRxAsFunc>> (function(): int) $f

      Suppose this is legal, then this will be allowed (in pseudo-code)

      function sub(<<__AtMostRxAsFunc>> (function(): int) $f) {
        $f(); // can call function here since it is conditionally reactive
      }
      <<__Rx>>
      function g() {
        $f: super = sub;
        // invoke non-reactive code in reactive context which is bad
        $f(() ==> { echo 1; });
      }
    }
    It will be safe if parameter in super will be completely reactive,
    hence check below *)
    let _, p_sub_type = Env.expand_type env p_sub.fp_type in
    begin match p_sub_type with
    | _, Tfun tfun when tfun.ft_reactive <> Nonreactive -> valid env
    | _, Tfun _ ->
      env, TL.Unsat (fun () -> Errors.rx_parameter_condition_mismatch
        SN.UserAttributes.uaAtMostRxAsFunc p_sub.fp_pos p_super.fp_pos)
    (* parameter type is not function - error will be reported in different place *)
    | _ -> valid env
    end
  | cond_sub, cond_super ->
    let cond_type_sub =
      match cond_sub with
      | Some (Param_rx_if_impl t) -> Some t
      | _ -> None in
    let cond_type_super =
      match cond_super with
      | Some (Param_rx_if_impl t) -> Some t
      | _ -> None in
    let ok =
      subtype_param_rx_if_impl ~is_param:true env cond_type_sub (Some p_sub.fp_type)
      cond_type_super in
    check_with ok (fun () ->
      Errors.rx_parameter_condition_mismatch
        SN.UserAttributes.uaOnlyRxIfImpl p_sub.fp_pos p_super.fp_pos) (env, TL.valid)

(* Helper function for subtyping on function types: performs all checks that
 * don't involve actual types:
 *   <<__ReturnDisposable>> attribute
 *   <<__MutableReturn>> attribute
 *   <<__Rx>> attribute
 *   <<__Mutable>> attribute
 *   whether or not the function is a coroutine
 *   variadic arity
 *)
and check_subtype_funs_attributes
  ?(extra_info: reactivity_extra_info option)
  (r_sub : Reason.t)
  (ft_sub : locl fun_type)
  (r_super : Reason.t)
  (ft_super : locl fun_type) env =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  (env, TL.valid) |>
  check_with
    (subtype_reactivity ?extra_info env ft_sub.ft_reactive ft_super.ft_reactive)
    (fun () -> Errors.fun_reactivity_mismatch
      p_super (TUtils.reactivity_to_string env ft_super.ft_reactive)
      p_sub (TUtils.reactivity_to_string env ft_sub.ft_reactive))
  |>
  check_with
    (ft_sub.ft_is_coroutine = ft_super.ft_is_coroutine)
    (fun () -> Errors.coroutinness_mismatch ft_super.ft_is_coroutine p_super p_sub) |>
  check_with
    (ft_sub.ft_return_disposable = ft_super.ft_return_disposable)
    (fun () -> Errors.return_disposable_mismatch ft_super.ft_return_disposable p_super p_sub) |>
  (* it is ok for subclass to return mutably owned value and treat it as immutable -
  the fact that value is mutably owned guarantees it has only single reference so
  as a result this single reference will be immutable. However if super type
  returns mutable value and subtype yields immutable value - this is not safe.
  NOTE: error is not reported if child is non-reactive since it does not have
  immutability-by-default behavior *)
  check_with
    (ft_sub.ft_returns_mutable = ft_super.ft_returns_mutable
    || not ft_super.ft_returns_mutable
    || ft_sub.ft_reactive = Nonreactive)
    (fun () -> Errors.mutable_return_result_mismatch ft_super.ft_returns_mutable p_super p_sub) |>
  check_with
    (ft_super.ft_reactive = Nonreactive
    || ft_super.ft_returns_void_to_rx
    || not ft_sub.ft_returns_void_to_rx)
    (fun () ->
    (*  __ReturnsVoidToRx can be omitted on subtype, in this case using subtype
       via reference to supertype in rx context will be ok since result will be
       discarded. The opposite is not true:
       class A {
         <<__Rx, __Mutable>>
         public function f(): A { return new A(); }
       }
       class B extends A {
         <<__Rx, __Mutable, __ReturnsVoidToRx>>
         public function f(): A { return $this; }
       }

       <<__Rx, __MutableReturn>>
       function f(): A { return new B(); }
       $a = HH\Rx\mutable(f());
       $a1 = $a->f(); // immutable alias to mutable reference *)
      Errors.return_void_to_rx_mismatch ~pos1_has_attribute:true p_sub p_super) |>
  (* check mutability only for reactive functions *)
  let check_params_mutability =
    ft_super.ft_reactive <> Nonreactive &&
    ft_sub.ft_reactive <> Nonreactive in
  fun (env, prop) -> (if check_params_mutability
  (* check mutability of receivers *)
  then (env, prop) &&&
    check_mutability ~is_receiver:true
      p_super ft_super.ft_mutability p_sub ft_sub.ft_mutability else env, prop) |>
  check_with
    ((arity_min ft_sub.ft_arity) <= (arity_min ft_super.ft_arity))
    (fun () -> Errors.fun_too_many_args p_sub p_super)
    |>
  fun res -> (match ft_sub.ft_arity, ft_super.ft_arity with
    | Fellipsis _, Fvariadic _ ->
      (* The HHVM runtime ignores "..." entirely, but knows about
       * "...$args"; for contexts for which the runtime enforces method
       * compatibility (currently, inheritance from abstract/interface
       * methods), letting "..." override "...$args" would result in method
       * compatibility errors at runtime. *)
      with_error (fun () -> Errors.fun_variadicity_hh_vs_php56 p_sub p_super) res
    | Fstandard (_, sub_max), Fstandard (_, super_max) ->
      if sub_max < super_max
      then with_error (fun () -> Errors.fun_too_few_args p_sub p_super) res else res
    | Fstandard _, _ -> with_error (fun () -> Errors.fun_unexpected_nonvariadic p_sub p_super) res
    | _, _ -> res
  )

(* This implements basic subtyping on non-generic function types:
 *   (1) return type behaves covariantly
 *   (2) parameter types behave contravariantly
 *   (3) special casing for variadics, and various reactivity and mutability attributes
 *)
 and simplify_subtype_funs
   ~(seen_generic_params : SSet.t option)
   ~(deep : bool)
   ~(no_top_bottom : bool)
   ~(check_return : bool)
   ?(error : (Env.env -> locl ty -> locl ty -> unit) option = None)
  ?(extra_info: reactivity_extra_info option)
  (r_sub : Reason.t)
  (ft_sub : locl fun_type)
  (r_super : Reason.t)
  (ft_super : locl fun_type)
  env
   : Env.env * TL.subtype_prop =
  let variadic_subtype = match ft_sub.ft_arity with
    | Fvariadic (_, {fp_type = var_sub; _ }) -> Some var_sub
    | _ -> None in
  let variadic_supertype =  match ft_super.ft_arity with
    | Fvariadic (_, {fp_type = var_super; _ }) -> Some var_super
    | _ -> None in

  let simplify_subtype = simplify_subtype ~seen_generic_params ~no_top_bottom ~deep ~error in
  let simplify_subtype_params = simplify_subtype_params ~seen_generic_params
    ~no_top_bottom ~deep in

  (* First apply checks on attributes, coroutine-ness and variadic arity *)
  env |>
  check_subtype_funs_attributes ?extra_info r_sub ft_sub r_super ft_super &&&

  (* Now do contravariant subtyping on parameters *)
  begin
    match variadic_subtype, variadic_supertype with
    | Some var_sub, Some var_super -> simplify_subtype var_super var_sub
    | _ -> valid
  end &&&

  begin
    let check_params_mutability =
      ft_super.ft_reactive <> Nonreactive &&
      ft_sub.ft_reactive <> Nonreactive in
    let is_method =
      (Option.map extra_info (fun i -> Option.is_some i.method_info)) = Some true in
    simplify_subtype_params
      ~is_method
      ~check_params_reactivity:(should_check_fun_params_reactivity ft_super)
      ~check_params_mutability
      ft_super.ft_params ft_sub.ft_params variadic_subtype variadic_supertype
  end &&&

  (* Finally do covariant subtryping on return type *)
  if check_return
  then simplify_subtype ft_sub.ft_ret ft_super.ft_ret
  else valid

(* One of the main entry points to this module *)
and sub_type
  ?(error : (Env.env -> locl ty -> locl ty -> unit) option = None)
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty) : Env.env =
    Env.log_env_change "sub_type" env @@
    sub_type_inner env ~error ~this_ty:None ty_sub ty_super

(* Add a new upper bound ty on var.  Apply transitivity of sutyping,
 * so if we already have tyl <: var, then check that for each ty_sub
 * in tyl we have ty_sub <: ty.
 *)
and add_tyvar_upper_bound_and_close env var ty =
  Env.log_env_change "add_tyvar_upper_bound_and_close" env @@
  let upper_bounds_before = Env.get_tyvar_upper_bounds env var in
  let env = Env.add_tyvar_upper_bound ~intersect:(try_intersect env) env var ty in
  let upper_bounds_after = Env.get_tyvar_upper_bounds env var in
  let added_upper_bounds = Typing_set.diff upper_bounds_after upper_bounds_before in
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  let env =
    Typing_set.fold (fun upper_bound env ->
      let env = Typing_subtype_tconst.make_all_type_consts_equal env var
        upper_bound ~as_tyvar_with_cnstr:true in
      Typing_set.fold (fun lower_bound env ->
        sub_type env lower_bound upper_bound)
        lower_bounds env)
      added_upper_bounds env in
  env

(* Add a new lower bound ty on var.  Apply transitivity of sutyping,
 * so if we already have var <: tyl, then check that for each ty_super
 * in tyl we have ty <: ty_super.
 *)
and add_tyvar_lower_bound_and_close env var ty =
  Env.log_env_change "add_tyvar_lower_bound_and_close" env @@
  let lower_bounds_before = Env.get_tyvar_lower_bounds env var in
  let env = Env.add_tyvar_lower_bound ~union:(try_union env) env var ty in
  let lower_bounds_after = Env.get_tyvar_lower_bounds env var in
  let added_lower_bounds = Typing_set.diff lower_bounds_after lower_bounds_before in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let env =
    Typing_set.fold (fun lower_bound env ->
      let env = Typing_subtype_tconst.make_all_type_consts_equal env var
        lower_bound ~as_tyvar_with_cnstr:false in
      Typing_set.fold (fun upper_bound env ->
        sub_type env lower_bound upper_bound)
        upper_bounds env)
      added_lower_bounds env in
  env

and props_to_env env remain props =
  match props with
  | [] ->
    env, List.rev remain
  | TL.IsSubtype (((_, Tvar var_sub) as ty_sub), ((_, Tvar var_super) as ty_super)) :: props ->
    let env = add_tyvar_upper_bound_and_close env var_sub ty_super in
    let env = add_tyvar_lower_bound_and_close env var_super ty_sub in
    props_to_env env remain props
  | TL.IsSubtype ((_, Tvar var), ty) :: props ->
    let env = add_tyvar_upper_bound_and_close env var ty in
    props_to_env env remain props
  | TL.IsSubtype (ty, (_, Tvar var)) :: props ->
    let env = add_tyvar_lower_bound_and_close env var ty in
    props_to_env env remain props
  | TL.Conj props' :: props ->
    props_to_env env remain (props' @ props)
  | TL.Disj disj_props :: conj_props ->
    (* For now, just find the first prop in the disjunction that works *)
    let rec try_disj disj_props =
      match disj_props with
      | [] ->
        (* For now let it fail later when calling
        process_simplify_subtype_result on the remaining constraints. *)
        props_to_env env (TL.Disj [] :: remain) conj_props
      | prop :: disj_props' ->
        Errors.try_
          (fun () -> props_to_env env remain (prop::conj_props))
          (fun _ -> try_disj disj_props')
    in
    try_disj disj_props
  | prop :: props ->
    props_to_env env (prop::remain) props

(* Move any top-level conjuncts of the form Tvar v <: t or t <: Tvar v to
 * the type variable environment. To do: use intersection and union to
 * simplify bounds.
 *)
and prop_to_env env prop =
  let env, props' = props_to_env env [] [prop] in
  env, TL.conj_list props'

and env_to_prop env =
  TL.conj (tvenv_to_prop env.Env.tvenv) env.Env.subtype_prop

and tvenv_to_prop tvenv =
  let props_per_tvar = IMap.mapi (
    fun id { Env.lower_bounds; Env.upper_bounds; _ } ->
      let tyvar = (Reason.Rnone, Tvar id) in
      let lower_bounds = TySet.elements lower_bounds in
      let upper_bounds = TySet.elements upper_bounds in
      let lower_bounds_props = List.map ~f:(fun ty -> TL.IsSubtype (ty, tyvar))
        lower_bounds in
      (* If an upper bound of variable n1 is a `Tvar n2`,
      then we have already added "Tvar n1 <: Tvar n2" when traversing
      lower bounds of n2, so we can filter out upper bounds that are Tvars. *)
      let can_be_removed = function
        | _, Tvar n ->
          begin match IMap.find_opt n tvenv with
          | Some _ -> true
          | None -> false
          end
        | _ -> false in
      let upper_bounds = List.filter ~f:(fun ty -> not (can_be_removed ty))
        upper_bounds in
      let upper_bounds_props = List.map ~f:(fun ty -> TL.IsSubtype (tyvar, ty))
        upper_bounds in
      TL.conj_list (lower_bounds_props @ upper_bounds_props))
    tvenv in
  let _ids, props = List.unzip (IMap.bindings props_per_tvar) in
  TL.conj_list props

and sub_type_inner
  (env : Env.env)
  ?(error : (Env.env -> locl ty -> locl ty -> unit) option = None)
  ~(this_ty : locl ty option)
  (ty_sub: locl ty)
  (ty_super: locl ty) : Env.env =
  log_subtype ~this_ty ~function_name:"sub_type_inner" env ty_sub ty_super;
  let new_inference = TypecheckerOptions.new_inference (Env.get_tcopt env) in
  let env, prop = simplify_subtype
    ~seen_generic_params:empty_seen
    ~deep:new_inference
    ~no_top_bottom:false
    ~error
    ~this_ty ty_sub ty_super env in
  let env, prop = if new_inference then prop_to_env env prop else env, prop in
  let env =
    if new_inference || TypecheckerOptions.experimental_feature_enabled
         (Env.get_tcopt env)
         TypecheckerOptions.experimental_track_subtype_prop
    then Env.add_subtype_prop env prop
    else env in
  let fail () = TUtils.uerror env (fst ty_super) (snd ty_super) (fst ty_sub) (snd ty_sub) in
  process_simplify_subtype_result ~this_ty ~fail env prop

(* Deal with the cases not dealt with by simplify_subtype *)
and sub_type_inner_helper env ~this_ty
  ty_sub ty_super =
  let env, ety_super =
    Env.expand_type env ty_super in
  let env, ety_sub =
    Env.expand_type env ty_sub in
  (* Default error *)

  let _fail () =
    TUtils.uerror env (fst ety_super) (snd ety_super) (fst ety_sub) (snd ety_sub);
    env in

  log_subtype ~this_ty ~function_name:"sub_type_inner_helper" env ty_sub ty_super;
  match ety_sub, ety_super with

  | (_, Tunresolved _), _
    when TypecheckerOptions.new_inference (Env.get_tcopt env) ->
    assert false

  | (_, Tunresolved _), (_, Tunresolved _) ->
    fst (Unify.unify env ty_super ty_sub)

(****************************************************************************)
(* ### Begin Tunresolved madness ###
 * If the supertype is a
 * Tunresolved, we allow it to keep growing, which is the desired behavior for
 * e.g. figuring out the type of a generic, but if the subtype is a
 * Tunresolved, then we check that all the members are indeed subtypes of the
 * given supertype, which is the desired behavior for e.g. checking function
 * return values. In general, if a supertype is Tunresolved, then we
 * consider it to be "not yet finalized", but if a subtype is Tunresolved
 * and the supertype isn't, we've probably hit a type annotation
 * and should consider the supertype to be definitive.
 *
 * However, sometimes we want this behavior reversed, e.g. when the type
 * annotation has a contravariant generic parameter or a `super` constraint --
 * now the definitive type is the subtype.
 *
 * I considered splitting this out into a separate function and swapping the
 * order of the super / sub types passed to it, so we would only have to handle
 * one set of cases, but it doesn't look much better since that function still
 * has to recursively call sub_type and therefore needs to remember whether its
 * arguments had been swapped.
 *)
(****************************************************************************)
  | (r_sub, _), (_, Tunresolved _) ->
      let ty_sub = (r_sub, Tunresolved [ty_sub]) in
      sub_type_inner env ~this_ty ty_sub ty_super
   (* This case is for when Tany comes from expanding an empty Tvar - it will
    * result in binding the type variable to the other type. *)
  | _, (_, Tany) ->
    fst (Unify.unify env ty_super ty_sub)

    (* If the subtype is a type variable bound to an empty unresolved, record
     * this in the todo list to be checked later. But make sure that we wrap
     * any error using the position and reason information that was supplied
     * at the entry point to subtyping.
     *)
  | (_, Tunresolved []), _
    when not (TypecheckerOptions.new_inference (Env.get_tcopt env)) ->
    begin match ty_sub with
    | (_, Tvar _) ->
      if not
        (TypecheckerOptions.experimental_feature_enabled
        (Env.get_tcopt env)
        TypecheckerOptions.experimental_unresolved_fix)
      then env
      else
        let outer_pos = env.Env.outer_pos in
        let outer_reason = env.Env.outer_reason in
        Env.add_todo env begin fun env' ->
          Errors.try_add_err outer_pos (Reason.string_of_ureason outer_reason)
          (fun () ->
            sub_type env' ty_sub ty_super, false)
          (fun _ ->
            env', true (* Remove from todo list if there was an error *)
          )
          end

    | _ ->
      env
    end

  | (_, Tunresolved tyl), _ ->
    let env =
      List.fold_left tyl ~f:begin fun env x ->
        sub_type_inner env ~this_ty
          x ty_super
      end ~init:env in
    env

(****************************************************************************)
(* ### End Tunresolved madness ### *)
(****************************************************************************)

  | _, _ ->
    (* TODO: replace this by fail() once we support all subtyping rules that
     * are implemented in unification *)
     fst (Unify.unify env ty_super ty_sub)

(* BEWARE: hack upon hack here.
 * To implement a predicate that tests whether `ty_sub` is a subtype of
 * `ty_super`, we call sub_type but handle any unification errors and
 * turn them into `false` result. Unfortunately HH_FIXME might end up
 * hiding the "error", and so we need to disable the fixme mechanism
 * before calling sub_type and then re-enable it afterwards.
 *)
and is_sub_type
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty) : bool =
  (* quick short circuit to help perf *)
  ty_equal ty_sub ty_super ||
  begin
    let f = !Errors.is_hh_fixme in
    Errors.is_hh_fixme := (fun _ _ -> false);
    let result =
      Errors.try_
        (fun () -> ignore(sub_type env ty_sub ty_super); true)
        (fun _ -> false) in
    Errors.is_hh_fixme := f;
    result
  end

and is_sub_type_alt env ~no_top_bottom ty1 ty2 =
  let _env, prop =
    simplify_subtype ~seen_generic_params:None ~deep:true ~no_top_bottom
    ~this_ty:(Some ty1) ty1 ty2 env in
  if TL.is_valid prop then Some true
  else
  if TL.is_unsat prop then Some false
  else None

(* Attempt to compute the intersection of a type with an existing list intersection.
 * If try_intersect env t [t1;...;tn] = [u1; ...; um]
 * then u1&...&um must be the greatest lower bound of t and t1&...&tn wrt subtyping.
 * For example:
 *   try_intersect nonnull [?C] = [C]
 *   try_intersect t1 [t2] = [t1]  if t1 <: t2
 * Note: it's acceptable to return [t;t1;...;tn] but the intention is that
 * we simplify (as above) wherever practical.
 * It can be assumed that the original list contains no redundancy.
 *)
and try_intersect env ty tyl =
  let is_sub_type_alt = is_sub_type_alt ~no_top_bottom:true in
  match tyl with
  | [] -> [ty]
  | ty'::tyl' ->
    if is_sub_type_alt env ty ty' = Some true
    then try_intersect env ty tyl'
    else
    if is_sub_type_alt env ty' ty = Some true
    then tyl
    else
    let nonnull_ty = (fst ty, Tnonnull) in
    match ty, ty' with
    | (_, Toption t), _ when is_sub_type_alt env ty' nonnull_ty = Some true ->
      try_intersect env t (ty'::tyl')
    | _, (_, Toption t) when is_sub_type_alt env ty nonnull_ty = Some true ->
      try_intersect env t (ty::tyl')
    | _, _ -> ty' :: try_intersect env ty tyl'

(* Attempt to compute the union of a type with an existing list union.
 * If try_union env t [t1;...;tn] = [u1;...;um]
 * then u1|...|um must be the least upper bound of t and t1|...|tn wrt subtyping.
 * For example:
 *   try_union int [float] = [num]
 *   try_union t1 [t2] = [t1] if t2 <: t1
 *
 * Notes:
 * 1. It's acceptable to return [t;t1;...;tn] but the intention is that
 *    we simplify (as above) wherever practical.
 * 2. Do not use Tunresolved for a syntactic union - the caller can do that.
 * 3. It can be assumed that the original list contains no redundancy.
 * TODO: there are many more unions to implement yet.
 *)
and try_union env ty tyl =
  let is_sub_type_alt = is_sub_type_alt ~no_top_bottom:true in
  match tyl with
  | [] -> [ty]
  | ty'::tyl' ->
    if is_sub_type_alt env ty ty' = Some true
    then tyl
    else
    if is_sub_type_alt env ty' ty = Some true
    then try_union env ty tyl'
    else match snd ty, snd ty' with
    | Tprim Nast.Tfloat, Tprim Nast.Tint
    | Tprim Nast.Tint, Tprim Nast.Tfloat ->
      let t = MakeType.num (fst ty) in
      try_union env t tyl'
    | _, _ -> ty' :: try_union env ty tyl'

let rec sub_string
  ?(allow_mixed = false)
  (p : Pos.Map.key)
  (env : Env.env)
  (ty2 : locl ty) : Env.env =
  let stringish_deprecated =
    TypecheckerOptions.disallow_stringish_magic (Env.get_tcopt env) in
  (* Under constraint-based inference, we implement sub_string as a subtype test.
   * All the cases in the legacy implementation just fall out from subtyping rules.
   * We test against ?(arraykey | bool | float | resource | object | dynamic)
   *)
  if not allow_mixed && TypecheckerOptions.new_inference (Env.get_tcopt env)
  then
    let r = Reason.Rwitness p in
    let tyl = [MakeType.arraykey r;
               MakeType.bool r;
               MakeType.float r;
               MakeType.resource r;
               MakeType.dynamic r] in
    let stringish =
      (Reason.Rwitness p, Tclass((p, SN.Classes.cStringish), Nonexact, [])) in
    let tyl =
      if stringish_deprecated
      then tyl
      else stringish::tyl in
    let stringlike = (Reason.Rwitness p, Toption (Reason.Rwitness p, Tunresolved tyl)) in
    let error env ty_sub ty_super = match snd ty_sub with
      | _ when is_sub_type_alt ~no_top_bottom:true env ty_sub stringish = Some true &&
               stringish_deprecated ->
        Errors.object_string_deprecated p
      | Tclass _ ->
        Errors.object_string p (Reason.to_pos (fst ty_sub))
      | _ ->
        TUtils.uerror env (fst ty_super) (snd ty_super) (fst ty_sub) (snd ty_sub) in
    sub_type ~error:(Some error) env ty2 stringlike
  else
  let sub_string = sub_string ~allow_mixed in
  let env, ety2 = Env.expand_type env ty2 in
  let fail () =
    TUtils.uerror env (Reason.Rwitness p) (Tprim Nast.Tstring) (fst ety2) (snd ety2);
    env in

  match ety2 with
  | (_, Toption ty2) -> sub_string p env ty2
  | (_, Tunresolved tyl) ->
      List.fold_left tyl ~f:(sub_string p) ~init:env
  | (_, Tprim _) ->
      env
  | (_, Tabstract (AKenum _, _)) ->
      (* Enums are either ints or strings, and so can always be used in a
       * stringish context *)
      env
  | (_, Tabstract _) ->
    begin match TUtils.get_concrete_supertypes env ty2 with
      | _, [] when allow_mixed ->
        env
      | _, [] ->
        fail ()
      | env, tyl ->
        List.fold_left tyl ~f:(sub_string p) ~init:env
    end
  | (r2, Tclass (x, _, _)) ->
      let class_ = Env.get_class env (snd x) in
      (match class_ with
      | None -> env
      | Some tc
          (* A Stringish is a string or an object with a __toString method
           * that will be converted to a string *)
          when Cls.name tc = SN.Classes.cStringish
          || Cls.has_ancestor tc SN.Classes.cStringish ->
        if stringish_deprecated
        then Errors.object_string_deprecated p;
        env
      | Some _ ->
        Errors.object_string p (Reason.to_pos r2);
        env
      )
  | _, (Tany | Terr | Tdynamic) ->
    env (* Tany, Terr and Tdynamic are considered Stringish by default *)
  | _, Tobject -> env
  | _, Tnonnull when allow_mixed ->
    env
  | _, (Tnonnull | Tarraykind _ | Tvar _
    | Ttuple _ | Tanon (_, _) | Tfun _ | Tshape _) ->
  fail ()

(** Check that the method with signature ft_sub can be used to override
 * (is a subtype of) method with signature ft_super.
 *
 * This goes beyond subtyping on function types because methods can have
 * generic parameters with bounds, and `where` constraints.
 *
 * Suppose ft_super is of the form
 *    <T1 csuper1, ..., Tn csupern>(tsuper1, ..., tsuperm) : tsuper where wsuper
 * and ft_sub is of the form
 *    <T1 csub1, ..., Tn csubn>(tsub1, ..., tsubm) : tsub where wsub
 * where csuperX and csubX are constraints on type parameters and wsuper and
 * wsub are 'where' constraints. Note that all types in the superclass,
 * including constraints (so csuperX, tsuperX, tsuper and wsuper) have had
 * any class type parameters instantiated appropriately according to
 * the actual arguments of the superclass. For example, suppose we have
 *
 *   class Super<T> {
 *     function foo<Tu as A<T>>(T $x) : B<T> where T super C<T>
 *   }
 *   class Sub extends Super<D> {
 *     ...override of foo...
 *   }
 * then the actual signature in the superclass that we need to check is
 *     function foo<Tu as A<D>>(D $x) : B<D> where D super C<D>
 * Note in particular the general form of the 'where' constraint.
 *
 * (Currently, this instantiation happens in
 *   Typing_extends.check_class_implements which in turn calls
 *   Decl_instantiate.instantiate_ce)
 *
 * Then for ft_sub to be a subtype of ft_super it must be the case that
 * (1) tsuper1 <: tsub1, ..., tsupern <: tsubn (under constraints
 *     T1 csuper1, ..., Tn csupern and wsuper).
 *
 *     This is contravariant subtyping on parameter types.
 *
 * (2) tsub <: tsuper (under constraints T1 csuper1, ..., Tn csupern and wsuper)
 *     This is covariant subtyping on result type. For constraints consider
 *       e.g. consider ft_super = <T super I>(): T
 *                 and ft_sub = <T>(): I
 *
 * (3) The constraints for ft_super entail the constraints for ft_sub, because
 *     we might be calling the function having checked that csuperX are
 *     satisfied but the definition of the function (e.g. consider an override)
 *     has been checked under csubX.
 *     More precisely, we must assume constraints T1 csuper1, ..., Tn csupern
 *     and wsuper, and check that T1 satisfies csub1, ..., Tn satisfies csubn
 *     and that wsub holds under those assumptions.
 *)
let subtype_method
  ~(check_return : bool)
  ~(extra_info: reactivity_extra_info)
  (env : Env.env)
  (r_sub : Reason.t)
  (ft_sub : decl fun_type)
  (r_super : Reason.t)
  (ft_super : decl fun_type) : Env.env =
  if not ft_super.ft_abstract && ft_sub.ft_abstract then
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See override_abstract_concrete.php test case for example. *)
    Errors.abstract_concrete_override ft_sub.ft_pos ft_super.ft_pos `method_;
  let ety_env =
    Phase.env_with_self env in
  let env, ft_super_no_tvars =
    Phase.localize_ft ~use_pos:ft_super.ft_pos ~ety_env ~instantiate_tparams:false env ft_super in
  let env, ft_sub_no_tvars =
    Phase.localize_ft ~use_pos:ft_sub.ft_pos ~ety_env ~instantiate_tparams:false env ft_sub in
  let old_tpenv = env.Env.lenv.Env.tpenv in

  (* We check constraint entailment and contravariant parameter/covariant result
   * subtyping in the context of the ft_super constraints. But we'd better
   * restore tpenv afterwards *)
  let add_tparams_constraints env (tparams: locl tparam list) =
    let add_bound env { tp_name = (pos, name); tp_constraints = cstrl; _ } =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
        let tparam_ty = (Reason.Rwitness pos,
          Tabstract(AKgeneric name, None)) in
        Typing_utils.add_constraint pos env ck tparam_ty ty) in
    List.fold_left tparams ~f:add_bound ~init: env in

  let p_sub = Reason.to_pos r_sub in

  let add_where_constraints env (cstrl: locl where_constraint list) =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
      Typing_utils.add_constraint p_sub env ck ty1 ty2) in

  let env =
    add_tparams_constraints env (fst ft_super_no_tvars.ft_tparams) in
  let env =
    add_where_constraints env ft_super_no_tvars.ft_where_constraints in

  let env, res =
  simplify_subtype_funs
    ~seen_generic_params:empty_seen
    ~deep:true
    ~no_top_bottom:false
    ~check_return
    ~extra_info
    r_sub ft_sub_no_tvars
    r_super ft_super_no_tvars
    env in
  let env =
    process_simplify_subtype_result ~this_ty:None
    ~fail:(fun () -> ()) env res in

  (* This is (3) above *)
  let check_tparams_constraints env tparams =
    let check_tparam_constraints env { tp_name = (p, name); tp_constraints = cstrl; _ } =
      List.fold_left cstrl ~init:env ~f:begin fun env (ck, cstr_ty) ->
        let tgeneric = (Reason.Rwitness p, Tabstract (AKgeneric name, None)) in
        Typing_generic_constraint.check_constraint env ck tgeneric ~cstr_ty
      end in
    List.fold_left tparams ~init:env ~f:check_tparam_constraints in

  let check_where_constraints env cstrl =
    List.fold_left cstrl ~init:env ~f:begin fun env (ty1, ck, ty2) ->
      Typing_generic_constraint.check_constraint env ck ty1 ~cstr_ty:ty2
    end in

  (* We only do this if the ft_tparam lengths match. Currently we don't even
   * report this as an error, indeed different names for type parameters.
   * TODO: make it an error to override with wrong number of type parameters
  *)
  let env =
    if List.length (fst ft_sub.ft_tparams) <> List.length (fst ft_super.ft_tparams)
    then env
    else check_tparams_constraints env (fst ft_sub_no_tvars.ft_tparams) in
  let env =
    check_where_constraints env ft_sub_no_tvars.ft_where_constraints in

  Env.env_with_tpenv env old_tpenv

let decompose_subtype_add_bound
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  : Env.env =
  let env, ty_super = Env.expand_type env ty_super in
  let env, ty_sub = Env.expand_type env ty_sub in
  match ty_sub, ty_super with
  (* name_sub <: ty_super so add an upper bound on name_sub *)
  | (_, Tabstract (AKgeneric name_sub, _)), _ when not (phys_equal ty_sub ty_super) ->
    log_subtype ~this_ty:None ~function_name:"decompose_subtype_add_bound" env ty_sub ty_super;
    let tys = Env.get_upper_bounds env name_sub in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_super tys then env
    else Env.add_upper_bound ~intersect:(try_intersect env) env name_sub ty_super

  (* ty_sub <: name_super so add a lower bound on name_super *)
  | _, (_, Tabstract (AKgeneric name_super, _)) when not (phys_equal ty_sub ty_super) ->
    log_subtype ~this_ty:None ~function_name:"decompose_subtype_add_bound" env ty_sub ty_super;
    let tys = Env.get_lower_bounds env name_super in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_sub tys then env
    else Env.add_lower_bound ~union:(try_union env) env name_super ty_sub

  | _, _ ->
    env

(* Given two types that we know are in a subtype relationship
 *   ty_sub <: ty_super
 * add to env.tpenv any bounds on generic type parameters that must
 * hold for ty_sub <: ty_super to be valid.
 *
 * For example, suppose we know Cov<T> <: Cov<D> for a covariant class Cov.
 * Then it must be the case that T <: D so we add an upper bound D to the
 * bounds for T.
 *
 * Although some of this code is similar to that for sub_type_inner, its
 * purpose is different. sub_type_inner takes two types t and u and makes
 * updates to the substitution of type variables (through unification) to
 * make t <: u true.
 *
 * decompose_subtype takes two types t and u for which t <: u is *assumed* to
 * hold, and makes updates to bounds on generic parameters that *necessarily*
 * hold in order for t <: u.
 *)
let rec decompose_subtype
  p
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  : Env.env =
  log_subtype ~this_ty:None ~function_name:"decompose_subtype" env ty_sub ty_super;
  let env, prop =
    simplify_subtype ~seen_generic_params:None
      ~deep:true ~no_top_bottom:false ~this_ty:None ty_sub ty_super env in
  decompose_subtype_add_prop p env prop

and decompose_subtype_add_prop p env prop =
  match prop with
  | TL.Conj props ->
    List.fold_left ~f:(decompose_subtype_add_prop p) ~init:env props
  | TL.Disj _props ->
    Typing_log.log_prop 2 env.Env.pos "decompose_subtype_add_prop" env prop;
    env
  | TL.Unsat _ ->
    env
  | TL.IsSubtype (ty1, ty2) ->
    decompose_subtype_add_bound env ty1 ty2
  | TL.IsEqual (ty1, ty2) ->
    let env = decompose_subtype_add_bound env ty1 ty2 in
    decompose_subtype_add_bound env ty2 ty1

(* Decompose a general constraint *)
and decompose_constraint
  p
  (env : Env.env)
  (ck : Ast.constraint_kind)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  : Env.env =
  match ck with
  | Ast.Constraint_as ->
    decompose_subtype p env ty_sub ty_super
  | Ast.Constraint_super ->
    decompose_subtype p env ty_super ty_sub
  | Ast.Constraint_eq ->
    let env' = decompose_subtype p env ty_sub ty_super in
    decompose_subtype p env' ty_super ty_sub
  | Ast.Constraint_pu_from -> failwith "TODO(T36532263): Pocket Universes"

(* Given a constraint ty1 ck ty2 where ck is AS, SUPER or =,
 * add bounds to type parameters in the environment that necessarily
 * must hold in order for ty1 ck ty2.
 *
 * First, we invoke decompose_constraint to add initial bounds to
 * the environment. Then we iterate, decomposing constraints that
 * arise through transitivity across bounds.
 *
 * For example, suppose that env already contains
 *   C<T1> <: T2
 * for some covariant class C. Now suppose we add the
 * constraint "T2 as C<T3>" i.e. we end up with
 *   C<T1> <: T2 <: C<T3>
 * Then by transitivity we know that T1 <: T3 so we add this to the
 * environment too.
 *
 * We repeat this process until no further bounds are added to the
 * environment, or some limit is reached. (It's possible to construct
 * types that expand forever under inheritance.)
*)
let constraint_iteration_limit = 20
let add_constraint
  p
  (env : Env.env)
  (ck : Ast.constraint_kind)
  (ty_sub : locl ty)
  (ty_super : locl ty): Env.env =
  log_subtype ~this_ty:None ~function_name:"add_constraint" env ty_sub ty_super;
  let oldsize = Env.get_tpenv_size env in
  let env' = decompose_constraint p env ck ty_sub ty_super in
  if Env.get_tpenv_size env' = oldsize
  then env'
  else
  let rec iter n env =
    if n > constraint_iteration_limit then env
    else
      let oldsize = Env.get_tpenv_size env in
      let env' =
        List.fold_left (Env.get_generic_parameters env) ~init:env
          ~f:(fun env x ->
            List.fold_left (Typing_set.elements (Env.get_lower_bounds env x)) ~init:env
              ~f:(fun env ty_sub' ->
                List.fold_left (Typing_set.elements (Env.get_upper_bounds env x)) ~init:env
                  ~f:(fun env ty_super' ->
                    decompose_subtype p env ty_sub' ty_super'))) in
      if Env.get_tpenv_size env' = oldsize
      then env'
      else iter (n+1) env'
  in
    iter 0 env'

(* Given a type ty, replace any covariant or contravariant components of the type
 * with fresh type variables. Components replaced include
 *   covariant key and element types for tuples, arrays, and shapes
 *   covariant return type and contravariant parameter types for function types
 *   co- and contra-variant parameters to classish types and newtypes
 * Assert that the component is a subtype of the fresh variable (covariant) or
 * a supertype of the fresh variable (contravariant).
 *
 * Note that the variance of type variables is set explicitly to be invariant
 * because we only use this function on the lower bounds of an invariant
 * type variable.
 *
 * Also note that freshening lifts through unions and nullables.
 *
 * Example 1: the type
 *   ?dict<t1,t2>
 * will be transformed to
 *   ?dict<tvar1,tvar2> with t1 <: tvar1 and t2 <: tvar2
 *
 * Example 2: the type
 *   Contra<t>
 * will be transformed to
 *   Contra<tvar1> with tvar1 <: t
 *)
let rec freshen_inside_ty_wrt_variance env ((r, ty_) as ty) =
  let default () = env, ty in
  match ty_ with
  | Tany | Tnonnull | Terr | Tdynamic | Tobject | Tprim _ | Tanon _ | Tabstract(_, None) ->
    default ()
    (* Nullable is covariant *)
  | Toption ty ->
    let env, ty = freshen_inside_ty_wrt_variance env ty in
    env, (r, Toption ty)
  | Tunresolved tyl ->
    let env, tyl = List.map_env env tyl freshen_inside_ty_wrt_variance in
    env, (r, Tunresolved tyl)
    (* Tuples are covariant *)
  | Ttuple tyl ->
    let env, tyl = List.map_env env tyl (freshen_ty ~variance:Ast.Covariant) in
    env, (r, Ttuple tyl)
    (* Shape data is covariant *)
  | Tshape (known, fdm) ->
    let env, fdm = ShapeFieldMap.map_env (freshen_ty ~variance:Ast.Covariant) env fdm in
    env, (r, Tshape (known, fdm))
    (* Functions are covariant in return type, contravariant in parameter types *)
  | Tfun ft ->
    let env, ft_ret = freshen_ty ~variance:Ast.Covariant env ft.ft_ret in
    let env, ft_params = List.map_env env ft.ft_params
      (fun env p ->
       let env, fp_type = freshen_ty ~variance:Ast.Contravariant env p.fp_type in
       env, { p with fp_type = fp_type }) in
    env, (r, Tfun { ft with ft_ret = ft_ret; ft_params = ft_params })
    (* Newtype definitions carry their own variance declarations *)
  | Tabstract (AKnewtype (name, tyl), tyopt) ->
    begin match Env.get_typedef env name with
    | None ->
      default ()
    | Some td ->
      let variancel = List.map td.td_tparams (fun t -> t.tp_variance) in
      let env, tyl = freshen_tparams_wrt_variance env variancel tyl in
      env, (r, Tabstract (AKnewtype (name, tyl), tyopt))
    end
  | Tabstract _ ->
    default ()
    (* Classes carry their own variance declarations *)
  | Tclass ((p, cid), e, tyl) ->
    begin match Env.get_class env cid with
    | None ->
      default ()
    | Some cls ->
      let variancel = List.map (Cls.tparams cls) (fun t -> t.tp_variance) in
      let env, tyl = freshen_tparams_wrt_variance env variancel tyl in
      env, (r, Tclass((p, cid), e, tyl))
    end
    (* Arrays are covariant in key and data types *)
  | Tarraykind ak ->
    begin match ak with
    | AKany | AKempty -> default ()
    | AKvarray ty ->
      let env, ty = freshen_ty ~variance:Ast.Covariant env ty in
      env, (r, Tarraykind (AKvarray ty))
    | AKvec ty ->
      let env, ty = freshen_ty ~variance:Ast.Covariant env ty in
      env, (r, Tarraykind (AKvec ty))
    | AKvarray_or_darray ty ->
      let env, ty = freshen_ty ~variance:Ast.Covariant env ty in
      env, (r, Tarraykind (AKvarray_or_darray ty))
    | AKdarray (ty1, ty2) ->
      let env, ty1 = freshen_ty ~variance:Ast.Covariant env ty1 in
      let env, ty2 = freshen_ty ~variance:Ast.Covariant env ty2 in
      env, (r, Tarraykind (AKdarray (ty1, ty2)))
    | AKmap (ty1, ty2) ->
      let env, ty1 = freshen_ty ~variance:Ast.Covariant env ty1 in
      let env, ty2 = freshen_ty ~variance:Ast.Covariant env ty2 in
      env, (r, Tarraykind (AKmap (ty1, ty2)))
    end
  | Tvar _ ->
    default ()

and freshen_ty ~variance env ty =
  match variance with
  | Ast.Invariant -> env, ty
  | Ast.Covariant | Ast.Contravariant ->
    let v = Env.fresh () in
    let env = Env.add_current_tyvar env (Reason.to_pos (fst ty)) v in
    let env = Env.set_tyvar_appears_covariantly env v in
    let env = Env.set_tyvar_appears_contravariantly env v in
    let freshty = (fst ty, Tvar v) in
    let env =
      if variance = Ast.Covariant
      then sub_type env ty freshty
      else sub_type env freshty ty in
    env, freshty

and freshen_tparams_wrt_variance env variancel tyl =
   match variancel, tyl with
   | [], [] ->
     env, []
   | variance::variancel, ty::tyl ->
     let env, tyl = freshen_tparams_wrt_variance env variancel tyl in
     let env, ty = freshen_ty ~variance env ty in
     env, ty::tyl
   | _ ->
     env, tyl

let bind env var ty =
  Env.log_env_change "bind" env @@

  (* If there has been a use of this type variable that led to an "unknown type"
   * error (e.g. method invocation), then record this in the reason info. We
   * can make use of this for linters and code mods that suggest annotations *)
  let ty =
    if Env.get_tyvar_eager_solve_fail env var
    then (Reason.Rsolve_fail (Reason.to_pos (fst ty)), snd ty)
    else ty in

  (* Update the variance *)
  let env = Env.update_variance_after_bind env var ty in

  (* Unify the variable *)
  let env = Typing_unify_recursive.add env var ty in
  (* Remove the variable from the environment *)
  let env = Env.remove_tyvar env var in
  env

let var_as_ty var = (Reason.Rnone, Tvar var)

(* For the types that are lower bounds on a variable `var`, compute
 * a union type. Remove `var` itself, as this is redundant. *)
let lower_bounds_as_union env r var lower_bounds =
  let env, ty = TUtils.union_list env r (TySet.elements lower_bounds) in
  env, TUtils.diff ty (var_as_ty var)

(* Solve type variable var by assigning it to the union of its lower bounds.
 * If freshen=true, first freshen the covariant and contravariant components of
 * the bounds.
 *)
let bind_to_lower_bound ~freshen env r var lower_bounds =
  Env.log_env_change "bind_to_lower_bound" env @@
  let env, ty = lower_bounds_as_union env r var lower_bounds in
  (* Freshen components of the types in the union wrt their variance.
   * For example, if we have
   *   Cov<C>, Contra<D> <: v
   * then we actually construct the union
   *   Cov<#1> | Contra<#2> with C <: #1 and #2 <: D
   *)
  let env, ty =
    if freshen
    then freshen_inside_ty_wrt_variance env ty
    else env, ty in
  (* If any of the components of the union are type variables, then remove
  * var from their upper bounds. Why? Because if we construct
  *   v1 , ... , vn , t <: var
  * for type variables v1, ..., vn and non-type variable t
  * then necessarily we must have var as an upper bound on each of vi
  * so after binding var we end up with redundant bounds
  *   vi <: v1 | ... | vn | t
  *)
  let env =
    TySet.fold (fun ty env ->
      match Env.expand_type env ty with
      | env, (_, Tvar v) ->
        Env.remove_tyvar_upper_bound env v var
      | env, _ -> env) lower_bounds env in
  (* Now actually make the assignment var := ty, and remove var from tvenv *)
  bind env var ty

let bind_to_upper_bound env r var upper_bounds =
  Env.log_env_change "bind_to_upper_bound" env @@
  (* Remove bounds which are the union of var and something else *)
  let upper_bounds = TySet.filter (fun bound ->
    is_sub_type_alt env (var_as_ty var) bound ~no_top_bottom:true <> Some true)
    upper_bounds in
  match Typing_set.elements upper_bounds with
  | [] -> bind env var (MakeType.mixed r)
  | [ty] ->
    (* If ty is a variable (in future, if any of the types in the list are variables),
      * then remove var from their lower bounds. Why? Because if we construct
      *   var <: v1 , ... , vn , t
      * for type variables v1 , ... , vn and non-type variable t
      * then necessarily we must have var as a lower bound on each of vi
      * so after binding var we end up with redundant bounds
      *   v1 & ... & vn & t <: vi
      *)
    let env =
      (match Env.expand_type env ty with
      | env, (_, Tvar v) ->
        Env.remove_tyvar_lower_bound env v var
      | env, _ -> env) in
    bind env var ty
  (* For now, if there are multiple bounds, then don't solve. *)
  | _ -> env

let bind_to_equal_bound env var =
  let expand_all tyset = Typing_set.map
    (fun ty -> let _, ty = Env.expand_type env ty in ty) tyset in
  let tyvar_info = Env.get_tyvar_info env var in
  let lower_bounds = expand_all tyvar_info.Env.lower_bounds in
  let upper_bounds = expand_all tyvar_info.Env.upper_bounds in
  let equal_bounds = Typing_set.inter lower_bounds upper_bounds in
  let equal_bounds = Typing_set.remove (var_as_ty var) equal_bounds in
  match Typing_set.choose_opt equal_bounds with
  | Some ty -> bind env var ty
  | None -> env

let tyvar_is_solved env var =
  match snd @@ snd @@ Env.expand_type env (var_as_ty var) with
  | Tvar var' when var' = var -> false
  | _ -> true

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
let solve_tyvar ~freshen ~force_solve env r var =
  Typing_log.(log_with_level env "prop" 2 (fun () ->
    log_types (Reason.to_pos r) env
    [Log_head (Printf.sprintf "Typing_subtype.solve_tyvar force_solve=%b #%d"
      force_solve var, [])]));

  (* Don't try and solve twice *)
  if tyvar_is_solved env var
  then env
  else
  let tyvar_info = Env.get_tyvar_info env var in
  let r = if r = Reason.Rnone then Reason.Rwitness tyvar_info.Env.tyvar_pos else r in
  let lower_bounds = tyvar_info.Env.lower_bounds and upper_bounds = tyvar_info.Env.upper_bounds in
  match tyvar_info.Env.appears_covariantly, tyvar_info.Env.appears_contravariantly with
  | true, false
  | false, false ->
    (* As in Local Type Inference by Pierce & Turner, if type variable does
     * not appear at all, or only appears covariantly, force to lower bound
     *)
    bind_to_lower_bound ~freshen:false env r var lower_bounds
  | false, true ->
    (* As in Local Type Inference by Pierce & Turner, if type variable
     * appears only contravariantly, force to upper bound
     *)
    bind_to_upper_bound env r var upper_bounds
  | true, true ->
    (* As in Local Type Inference by Pierce & Turner, if type variable
     * appears both covariantly and contravariantly and there is a type that
     * is both a lower and upper bound, force to that type
     *)
    let env = bind_to_equal_bound env var in
    if not (tyvar_is_solved env var) && force_solve
    then bind_to_lower_bound ~freshen env r var lower_bounds
    else env

let solve_tyvar ~freshen ~force_solve env r var =
  let rec solve_until_concrete_ty env v =
    let env = solve_tyvar ~force_solve ~freshen env r v in
    let env, ety = Env.expand_var env r v in
    match ety with
    | _, Tvar v' when v <> v' -> solve_until_concrete_ty env v'
    | _ -> env in
  solve_until_concrete_ty env var

(* Force solve all type variables in the environment *)
let solve_all_unsolved_tyvars env =
  if TypecheckerOptions.new_inference (Env.get_tcopt env)
  then
    Env.log_env_change "solve_all_unsolved_tyvars" env @@
    IMap.fold
    (fun tyvar _ env ->
      solve_tyvar ~freshen:false ~force_solve:true
        env Reason.Rnone tyvar) env.Env.tvenv env
  else env

(* Expand an already-solved type variable, and solve an unsolved type variable
 * by binding it to the union of its lower bounds, with covariant and contravariant
 * components of the type suitably "freshened". For example,
 *    vec<C> <: #1
 * will be solved by
 *    #1 := vec<#2>  where C <: #2
 *)
let expand_type_and_solve env ~description_of_expected p ty =
  let new_inference = TypecheckerOptions.new_inference (Env.get_tcopt env) in
  if new_inference then
    let env', ety = Typing_utils.simplify_unions env ty
      ~on_tyvar:(fun env r v ->
        let env = solve_tyvar ~force_solve:true ~freshen:true env r v in
        Env.expand_var env r v) in
    match ty, ety with
    | (r, Tvar v), (_, Tunresolved []) when Env.get_tyvar_appears_invariantly env v ->
      Errors.unknown_type description_of_expected p (Reason.to_string "It is unknown" r);
      let env = Env.set_tyvar_eager_solve_fail env v in
      env, (Reason.Rsolve_fail p, TUtils.terr env)
    | _ -> env', ety
  else Env.expand_type env ty

(* When applied to concrete types (typically classes), the `widen_concrete_type`
 * function should produce the largest supertype that is valid for an operation.
 * For example, if we have an expression $x->f for $x:v and exact C <: v, then
 * we widen `exact C` to `B`, if `B` is the base class from which `C` inherits
 * field f.
 *
 * The `widen` function extends this to nullables and abstract types.
 * General unions have been dealt with already.
 *)
let rec widen env widen_concrete_type ty =
  let env, ty = Env.expand_type env ty in
  match ty with
   | r, Toption ty ->
    begin match widen env widen_concrete_type ty with
    | env, Some ty -> env, Some (r, Toption ty)
    | env, None -> env, None
    end
  (* Don't widen the `this` type, because the field type changes up the hierarchy
   * so we lose precision
   *)
  | _, Tabstract (AKdependent (`this, _), _) ->
    env, Some ty
  (* For other abstract types, just widen to the bound, if possible *)
  | _, Tabstract (_, Some ty) ->
    widen env widen_concrete_type ty
  | _ ->
    widen_concrete_type env ty

(* Deconstruct a type into its union elements, if it's a union or nullable.
 * If any elements are type variables, take their lower bounds.
 * Return (has_var, nullable_reason, elements) where
 *   has_var = true if any union elements are type variables
 *   nullable_reason = Some r if there is a nullable/null element with reason r
 *   elements are the de-duplicated elements of the union
 *)
let get_union_elements env ty =
  let rec aux env tyl (has_var, nullable_reason, res) =
  match tyl with
  | [] -> env, has_var,
      begin match nullable_reason with
      | None -> res
      | Some r -> MakeType.null r::res
      end
  | ty::tyl ->
    let env, ety = Env.expand_type env ty in
    match ety with
    | _, Tunresolved tyl' ->
      aux env (tyl' @ tyl) (has_var, nullable_reason, res)
    | r, Toption ty ->
      aux env (ty::tyl) (has_var, Some r, res)
    | _, Tvar var ->
      let tyvar_info = Env.get_tyvar_info env var in
      (* Lower bounds of type variable, excluding itself *)
      let lower_bounds = Typing_set.remove ety tyvar_info.Env.lower_bounds in
      aux env tyl (true, nullable_reason, Typing_set.elements lower_bounds @ res)
    | _ ->
      aux env tyl (has_var, nullable_reason, ety::res)
  in
    aux env [ty] (false, None, [])

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
 *)
let expand_type_and_narrow env ~description_of_expected widen_concrete_type p ty =
  if not (TypecheckerOptions.new_inference (Env.get_tcopt env))
  then Env.expand_type env ty
  else
  (* Deconstruct the type into union elements (if it's a union). For variables,
   * take the lower bounds. If there are no variables, then we have a concrete
   * type so just return expanded type
   *)
  let env, has_var, tys = get_union_elements env ty in
  if not has_var
  then Typing_utils.simplify_unions env ty
  else
  (* Now for each union element, use widen_concrete_type to suggest a concrete
   * upper bound. *)
    let rec widen_tys env tyl res =
      match tyl with
      | [] ->
        env, res

      | ty :: tyl ->
        let env, opt_upper = widen env widen_concrete_type ty in
        let res = match opt_upper with None -> res | Some ty -> ty :: res in
        widen_tys env tyl res in

    let env, widened_tys = widen_tys env tys [] in
    (* We really don't want to just guess `nothing` if none of the types can be widened *)
    if List.is_empty widened_tys
    (* Default behaviour is currently to force solve *)
    then expand_type_and_solve env ~description_of_expected p ty
    else
    let env, widened_ty = TUtils.union_list env (fst ty) widened_tys in
      Errors.try_
        (fun () ->
          let env = sub_type env ty widened_ty in
          env, widened_ty)
        (fun _ ->
          expand_type_and_solve env ~description_of_expected p ty)

(* Solve type variables on top of stack, without losing completeness, and pop
 * variables off the stack
 *)
let close_tyvars_and_solve env =
  let tyvars = Env.get_current_tyvars env in
  let env = Env.close_tyvars env in
  List.fold_left tyvars ~init:env
    ~f:(fun env tyvar -> solve_tyvar ~freshen:false ~force_solve:false
      env Reason.Rnone tyvar)

let log_prop env =
  let filename = Pos.filename (Pos.to_absolute env.Env.pos) in
  if Str.string_match (Str.regexp {|.*\.hhi|}) filename 0 then () else
  let prop = env_to_prop env in
  if TypecheckerOptions.log_inference_constraints (Env.get_tcopt env) then (
    let p_as_string = Typing_print.subtype_prop env prop in
    let pos = Pos.string (Pos.to_absolute env.Env.pos) in
    let size = TL.size prop in
    let n_disj = TL.n_disj prop in
    let n_conj = TL.n_conj prop in
    TypingLogger.InferenceCnstr.log p_as_string ~pos ~size ~n_disj ~n_conj);
  if TypecheckerOptions.new_inference (Env.get_tcopt env) &&
    not (Errors.currently_has_errors ()) &&
    not (TL.is_valid prop)
  then Typing_log.log_prop ~do_normalize:true 1 env.Env.pos
    "There are remaining unsolved constraints!" env prop

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.sub_type_ref := sub_type
let () = Typing_utils.add_constraint_ref := add_constraint
let () = Typing_utils.is_sub_type_ref := is_sub_type
let () = Typing_utils.is_sub_type_alt_ref := is_sub_type_alt
let () = Typing_utils.expand_type_and_solve_ref := expand_type_and_solve
