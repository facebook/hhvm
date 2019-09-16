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
open Utils
open Typing_defs
open Typing_env_types
module Reason = Typing_reason
module Unify = Typing_unify
module Env = Typing_env
module Inter = Typing_intersection
module Subst = Decl_subst
module TUtils = Typing_utils
module SN = Naming_special_names
module Phase = Typing_phase
module TL = Typing_logic
module Cls = Decl_provider.Class
module TySet = Typing_set
module MakeType = Typing_make_type
module Partial = Partial_provider
module ShapeMap = Nast.ShapeMap
module ShapeSet = Ast_defs.ShapeSet
module Nast = Aast

type reactivity_extra_info = {
  method_info: (* method_name *) (string * (* is_static *) bool) option;
  class_ty: phase_ty option;
  parent_class_ty: phase_ty option;
}

let empty_extra_info =
  { method_info = None; class_ty = None; parent_class_ty = None }

module ConditionTypes = struct
  let try_get_class_for_condition_type (env : env) (ty : decl_ty) =
    match TUtils.try_unwrap_class_type ty with
    | None -> None
    | Some (_, ((_, x) as sid), _) ->
      begin
        match Env.get_class env x with
        | None -> None
        | Some cls -> Some (sid, cls)
      end

  let try_get_method_from_condition_type
      (env : env) (ty : decl_ty) (is_static : bool) (method_name : string) =
    match try_get_class_for_condition_type env ty with
    | Some (_, cls) ->
      let get =
        if is_static then
          Cls.get_smethod
        else
          Cls.get_method
      in
      get cls method_name
    | None -> None

  let localize_condition_type (env : env) (ty : decl_ty) : locl_ty =
    (* if condition type is generic - we cannot specify type argument in attribute.
       For cases when we check if containing type is a subtype of condition type
       if condition type is generic instantiate it with TAny's *)
    let do_localize ty =
      let ty =
        match try_get_class_for_condition_type env ty with
        | None -> ty
        | Some (_, cls) when Cls.tparams cls = [] -> ty
        | Some (((p, _) as sid), cls) ->
          let params =
            List.map (Cls.tparams cls) ~f:(fun { tp_name = (p, x); _ } ->
                (Reason.Rwitness p, Tgeneric x))
          in
          let subst = Decl_instantiate.make_subst (Cls.tparams cls) [] in
          let ty = (Reason.Rwitness p, Tapply (sid, params)) in
          Decl_instantiate.instantiate subst ty
      in
      let ety_env = Phase.env_with_self env in
      let (_, t) = Phase.localize ~ety_env env ty in
      t
    in
    match ty with
    | (r, Toption ty) -> (r, Toption (do_localize ty))
    | ty -> do_localize ty
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
 * Elide singleton unions, treat invariant generics as both-ways
 * subtypes, and actually chase hierarchy for extends and implements.
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
let with_error (f : unit -> unit) ((env, p) : env * TL.subtype_prop) :
    env * TL.subtype_prop =
  (env, TL.conj p (TL.invalid ~fail:f))

(* If `b` is false then fail with error function `f` *)
let check_with b f r =
  if not b then
    with_error f r
  else
    r

let valid env : env * TL.subtype_prop = (env, TL.valid)

let ( &&& ) (env, p1) (f : env -> env * TL.subtype_prop) =
  if TL.is_unsat p1 then
    (env, p1)
  else
    let (env, p2) = f env in
    (env, TL.conj p1 p2)

let if_unsat (f : unit -> env * TL.subtype_prop) (env, p) =
  if TL.is_unsat p then
    f ()
  else
    (env, p)

let ignore_hh_fixmes f =
  let is_hh_fixme = !Errors.is_hh_fixme in
  (Errors.is_hh_fixme := (fun _ _ -> false));
  let result = f () in
  Errors.is_hh_fixme := is_hh_fixme;
  result

(** Check that a mutability type is a subtype of another mutability type *)
let check_mutability
    ~(is_receiver : bool)
    (p_sub : Pos.t)
    (mut_sub : param_mutability option)
    (p_super : Pos.t)
    (mut_super : param_mutability option)
    env =
  let str m =
    match m with
    | None -> "immutable"
    | Some Param_owned_mutable -> "owned mutable"
    | Some Param_borrowed_mutable -> "mutable"
    | Some Param_maybe_mutable -> "maybe-mutable"
  in
  (* maybe-mutable <------immutable
                      |
                       <--mutable <-- owned mutable  *)
  match (mut_sub, mut_super) with
  (* immutable is not compatible with mutable *)
  | (None, Some (Param_borrowed_mutable | Param_owned_mutable))
  (* mutable is not compatible with immutable  *)
  
  | (Some (Param_borrowed_mutable | Param_owned_mutable), None)
  (* borrowed mutable is not compatible with owned mutable *)
  
  | (Some Param_borrowed_mutable, Some Param_owned_mutable)
  (* maybe-mutable is not compatible with immutable/mutable *)
  
  | ( Some Param_maybe_mutable,
      (None | Some (Param_borrowed_mutable | Param_owned_mutable)) ) ->
    ( env,
      TL.invalid ~fail:(fun () ->
          Errors.mutability_mismatch
            ~is_receiver
            p_sub
            (str mut_sub)
            p_super
            (str mut_super)) )
  | _ -> valid env

let empty_seen = Some SSet.empty

let log_subtype ~level ~this_ty ~function_name env ty_sub ty_super =
  Typing_log.(
    log_with_level env "sub" level (fun () ->
        let types =
          [Log_type ("ty_sub", ty_sub); Log_type ("ty_super", ty_super)]
        in
        let types =
          Option.value_map this_ty ~default:types ~f:(fun ty ->
              Log_type ("this_ty", ty) :: types)
        in
        log_types
          (Reason.to_pos (fst ty_sub))
          env
          [Log_head (function_name, types)]))

let is_final_and_not_contravariant env id =
  let class_def = Env.get_class env id in
  match class_def with
  | Some class_ty -> TUtils.class_is_final_and_not_contravariant class_ty
  | None -> false

(** Make all types appearing in the given type a Tany, e.g.
- for A<B> return A<_>
- for function(A): B return function (_): _
*)
let anyfy env r ty =
  let anyfyer =
    object
      inherit Type_mapper.deep_type_mapper as super

      method! on_type env _ty = (env, (r, Typing_defs.make_tany ()))

      method go ty =
        let (_, ty) = super#on_type env ty in
        ty
    end
  in
  anyfyer#go ty

let find_type_with_exact_negation env tyl =
  let rec find env tyl acc_tyl =
    match tyl with
    | [] -> (env, None, acc_tyl)
    | ty :: tyl' ->
      let (env, non_ty) = TUtils.non env (fst ty) ty TUtils.ApproxDown in
      let nothing = MakeType.nothing Reason.none in
      if ty_equal non_ty nothing then
        find env tyl' (ty :: acc_tyl)
      else
        (env, Some non_ty, tyl' @ acc_tyl)
  in
  find env tyl []

(* Process the constraint proposition *)
let rec process_simplify_subtype_result prop =
  match prop with
  | TL.IsSubtype (_ty1, _ty2) ->
    (* All subtypes should have been resolved *)
    assert false
  | TL.Conj props ->
    (* Evaluates list from left-to-right so preserves order of conjuncts *)
    List.iter ~f:process_simplify_subtype_result props
  | TL.Disj (f, props) ->
    let rec try_disj props =
      match props with
      | [] -> f ()
      | prop :: props ->
        Errors.try_
          (fun () ->
            ignore_hh_fixmes (fun () -> process_simplify_subtype_result prop))
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
    ~(no_top_bottom : bool)
    ?(this_ty : locl_ty option = None)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    ~(on_error : Errors.typing_error_callback)
    env : env * TL.subtype_prop =
  log_subtype
    ~level:2
    ~this_ty
    ~function_name:"simplify_subtype"
    env
    ty_sub
    ty_super;
  let (env, ety_super) = Env.expand_type env ty_super in
  let (env, ety_sub) = Env.expand_type env ty_sub in
  let fail () =
    let (r1, ty1) = ety_super in
    let (r2, ty2) = ety_sub in
    let ty1 =
      Typing_print.with_blank_tyvars (fun () ->
          Typing_print.full_strip_ns env (r1, ty1))
    in
    let ty2 =
      Typing_print.with_blank_tyvars (fun () ->
          Typing_print.full_strip_ns env (r2, ty2))
    in
    let (ty1, ty2) =
      if String.equal ty1 ty2 then
        ("exactly the type " ^ ty1, "the nonexact type " ^ ty2)
      else
        (ty1, ty2)
    in
    let left = Reason.to_string ("Expected " ^ ty1) r1 in
    let right = Reason.to_string ("But got " ^ ty2) r2 in
    match (r1, r2) with
    | (Reason.Rcstr_on_generics (p, tparam), _)
    | (_, Reason.Rcstr_on_generics (p, tparam)) ->
      Errors.violated_constraint p tparam left right
    | _ -> on_error left right
  in
  let ( ||| ) (env, p1) (f : env -> env * TL.subtype_prop) =
    if TL.is_valid p1 then
      (env, p1)
    else
      let (env, p2) = f env in
      (env, TL.disj ~fail p1 p2)
  in
  (* We *know* that the assertion is unsatisfiable *)
  let invalid_with f = (env, TL.invalid ~fail:f) in
  let invalid () = invalid_with fail in
  let invalid_env_with env f = (env, TL.invalid ~fail:f) in
  let invalid_env env = invalid_env_with env fail in
  (* We *know* that the assertion is valid *)
  let valid () = (env, TL.valid) in
  (* We don't know whether the assertion is valid or not *)
  let default () = (env, TL.IsSubtype (ety_sub, ety_super)) in
  let simplify_subtype = simplify_subtype ~no_top_bottom ~on_error in
  let simplify_subtype_funs = simplify_subtype_funs ~no_top_bottom in
  let simplify_subtype_variance =
    simplify_subtype_variance ~no_top_bottom ~on_error
  in
  let simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super env =
    match seen_generic_params with
    | None -> default ()
    | Some seen ->
      (* If we've seen this type parameter before then we must have gone
       * round a cycle so we fail
       *)
      if SSet.mem name_sub seen then
        invalid ()
      else
        (* If the generic is actually an expression dependent type,
      we need to update this_ty
    *)
        let this_ty =
          if AbstractKind.is_generic_dep_ty name_sub && Option.is_none this_ty
          then
            Some ety_sub
          else
            this_ty
        in
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
            let r =
              Reason.Rimplicit_upper_bound
                (Reason.to_pos (fst ety_sub), "?nonnull")
            in
            let tmixed = MakeType.mixed r in
            env
            |> simplify_subtype ~seen_generic_params ~this_ty tmixed ty_super
          | [ty] ->
            env |> simplify_subtype ~seen_generic_params ~this_ty ty ty_super
          | ty :: tyl ->
            env
            |> try_bounds tyl
            ||| simplify_subtype ~seen_generic_params ~this_ty ty ty_super
        in
        env
        |> try_bounds
             ( Option.to_list opt_sub_cstr
             @ Typing_set.elements (Env.get_upper_bounds env name_sub) )
        |> (* Turn error into a generic error about the type parameter *)
           if_unsat invalid
  in
  let simplify_subtype_generic_super ty_sub name_super env =
    match seen_generic_params with
    | None -> default ()
    | Some seen ->
      (* If we've seen this type parameter before then we must have gone
       * round a cycle so we fail
       *)
      if SSet.mem name_super seen then
        invalid ()
      else
        let seen_generic_params = Some (SSet.add name_super seen) in
        (* Collect all the lower bounds ("super" constraints) on the
         * generic parameter, and check ty_sub against each of them in turn
         * until one of them succeeds *)
        let rec try_bounds tyl env =
          match tyl with
          | [] -> invalid ()
          | ty :: tyl ->
            env
            |> simplify_subtype ~seen_generic_params ~this_ty ty_sub ty
            ||| try_bounds tyl
        in
        (* Turn error into a generic error about the type parameter *)
        env
        |> try_bounds
             (Typing_set.elements (Env.get_lower_bounds env name_super))
        |> if_unsat invalid
  in
  let has_lower_bounds id =
    let class_def = Env.get_class env id in
    match class_def with
    | Some class_ty ->
      not (Sequence.is_empty (Cls.lower_bounds_on_this class_ty))
    | None -> false
  in
  match (snd ety_sub, snd ety_super) with
  | (Tvar var_sub, Tvar var_super) when var_sub = var_super -> valid ()
  (* Internally, newtypes and dependent types are always equipped with an upper bound.
   * In the case when no upper bound is specified in source code,
   * an implicit upper bound mixed = ?nonnull is added.
   *)
  | (Tabstract ((AKnewtype _ | AKdependent _), None), _)
  | (_, Tabstract ((AKnewtype _ | AKdependent _), None)) ->
    assert false
  | ( ( Tprim
          Nast.(
            ( Tint | Tbool | Tfloat | Tstring | Tresource | Tnum | Tarraykey
            | Tnoreturn | Tatom _ ))
      | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject | Tclass _
      | Tarraykind _ | Tpu _ ),
      Tnonnull ) ->
    valid ()
  | ((Tdynamic | Toption _ | Tprim Nast.(Tnull | Tvoid)), Tnonnull) ->
    invalid ()
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tnonnull) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (Tdynamic, Tdynamic) -> valid ()
  | ( ( Tnonnull | Toption _ | Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tanon _
      | Tobject | Tclass _ | Tarraykind _ ),
      Tdynamic ) ->
    invalid ()
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tdynamic) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  (* everything subtypes mixed *)
  | (_, Toption (_, Tnonnull)) -> valid ()
  (* null is the type of null and is a subtype of any option type. *)
  | (Tprim Nast.Tnull, Toption _) -> valid ()
  (* void behaves with respect to subtyping as an abstract type with
   * an implicit upper bound ?nonnull
   *)
  | (Tprim Nast.Tvoid, Toption ty_super') ->
    let r =
      Reason.Rimplicit_upper_bound (Reason.to_pos (fst ety_sub), "?nonnull")
    in
    let tmixed = MakeType.mixed r in
    env
    |> simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super'
    ||| simplify_subtype ~seen_generic_params ~this_ty tmixed ty_super
  | (Tdynamic, Toption ty_super) ->
    simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super env
  (* ?ty_sub' <: ?ty_super' iff ty_sub' <: ?ty_super'. Reasoning:
   * If ?ty_sub' <: ?ty_super', then from ty_sub' <: ?ty_sub' (widening) and transitivity
   * of <: it follows that ty_sub' <: ?ty_super'.  Conversely, if ty_sub' <: ?ty_super', then
   * by covariance and idempotence of ?, we have ?ty_sub' <: ??ty_sub' <: ?ty_super'.
   * Therefore, this step preserves the set of solutions.
   *)
  | (Toption ty_sub', Toption _) ->
    simplify_subtype ~seen_generic_params ~this_ty ty_sub' ty_super env
  (* If ty_sub <: ?ty_super' and ty_sub does not contain null then we
   * must also have ty_sub <: ty_super'.  The converse follows by
   * widening and transitivity.  Therefore, this step preserves the set
   * of solutions.
   *)
  | ( ( Tprim
          Nast.(
            ( Tint | Tbool | Tfloat | Tstring | Tresource | Tnum | Tarraykey
            | Tnoreturn | Tatom _ ))
      | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject | Tclass _
      | Tarraykind _ | Tany _ | Tpu _ ),
      Toption ty_super' ) ->
    simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super' env
  | ( Tabstract (AKnewtype (name_sub, _), _),
      Toption ((_, Tabstract (AKnewtype (name_super, _), _)) as ty_super') )
    when name_super = name_sub ->
    simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super' env
  | ( Tabstract (AKdependent d_sub, Some bound_sub),
      Tabstract (AKdependent d_super, Some bound_super) ) ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    (* Dependent types are identical but bound might be different *)
    if d_sub = d_super then
      simplify_subtype ~seen_generic_params ~this_ty bound_sub bound_super env
    else
      simplify_subtype ~seen_generic_params ~this_ty bound_sub ty_super env
  | (Tabstract (AKdependent (DTexpr _), Some ty), Toption arg_ty_super) ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    env
    |> simplify_subtype ~seen_generic_params ~this_ty ty ty_super
    ||| simplify_subtype ~seen_generic_params ~this_ty ty_sub arg_ty_super
  (* If t1 <: ?t2 and t1 is an abstract type constrained as t1',
   * then t1 <: t2 or t1' <: ?t2.  The converse is obviously
   * true as well.  We can fold the case where t1 is unconstrained
   * into the case analysis below.
   *)
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Toption arg_ty_super)
    ->
    env
    |> simplify_subtype ~seen_generic_params ~this_ty ty_sub arg_ty_super
    ||| simplify_subtype ~seen_generic_params ~this_ty ty ty_super
  (* If t1 <: ?t2, where t1 is guaranteed not to contain null, then
   * t1 <: t2, and the converse is obviously true as well.
   *)
  | (Tabstract (AKgeneric name_sub, opt_sub_cstr), Toption arg_ty_super)
    when Option.is_some seen_generic_params ->
    env
    |> simplify_subtype ~seen_generic_params ~this_ty ty_sub arg_ty_super
    ||| (* Look up *)
        simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super
  | (Tprim (Nast.Tint | Nast.Tfloat), Tprim Nast.Tnum) -> valid ()
  | (Tprim (Nast.Tint | Nast.Tstring), Tprim Nast.Tarraykey) -> valid ()
  | (Tprim p1, Tprim p2) ->
    if p1 = p2 then
      valid ()
    else
      invalid ()
  | ( ( Tnonnull | Tdynamic | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject
      | Tclass _ | Tarraykind _ ),
      Tprim _ ) ->
    invalid ()
  | ( Toption _,
      Tprim
        Nast.(
          ( Tvoid | Tint | Tbool | Tfloat | Tstring | Tresource | Tnum
          | Tarraykey | Tnoreturn | Tatom _ )) ) ->
    invalid ()
  | (Toption ty_sub', Tprim Nast.Tnull) ->
    simplify_subtype ~seen_generic_params ~this_ty ty_sub' ty_super env
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tprim _) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | ( ( Tnonnull | Tdynamic | Toption _ | Tprim _ | Ttuple _ | Tshape _
      | Tobject | Tclass _ | Tarraykind _ ),
      Tfun _ ) ->
    invalid ()
  | (Tfun ft_sub, Tfun ft_super) ->
    let (r_sub, r_super) = (fst ety_sub, fst ety_super) in
    simplify_subtype_funs
      ~seen_generic_params
      ~check_return:true
      r_sub
      ft_sub
      r_super
      ft_super
      on_error
      env
  | (Tanon (anon_arity, id), Tfun ft) ->
    let (r_sub, r_super) = (fst ety_sub, fst ety_super) in
    begin
      match Env.get_anonymous env id with
      | None ->
        invalid_with (fun () ->
            Errors.anonymous_recursive_call (Reason.to_pos r_sub))
      | Some
          {
            rx = reactivity;
            is_coroutine;
            counter = ftys;
            typecheck = anon;
            _;
          } ->
        let p_super = Reason.to_pos r_super in
        let p_sub = Reason.to_pos r_sub in
        (* Add function type to set of types seen so far *)
        ftys := TUtils.add_function_type env ety_super !ftys;
        (env, TL.valid)
        |> check_with
             ( subtype_reactivity env reactivity ft.ft_reactive
             || TypecheckerOptions.unsafe_rx (Env.get_tcopt env) )
             (fun () ->
               Errors.fun_reactivity_mismatch
                 p_super
                 (TUtils.reactivity_to_string env reactivity)
                 p_sub
                 (TUtils.reactivity_to_string env ft.ft_reactive))
        |> check_with (is_coroutine = ft.ft_is_coroutine) (fun () ->
               Errors.coroutinness_mismatch ft.ft_is_coroutine p_super p_sub)
        |> check_with
             (Unify.unify_arities
                ~ellipsis_is_variadic:true
                anon_arity
                ft.ft_arity)
             (fun () -> Errors.fun_arity_mismatch p_super p_sub)
        |> fun (env, prop) ->
        let (env, _, ret) = anon env ft.ft_params ft.ft_arity in
        (env, prop)
        &&& simplify_subtype
              ~seen_generic_params
              ~this_ty
              ret
              ft.ft_ret.et_type
    end
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tfun _) ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | ( ( Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Tshape _ | Tanon _
      | Tobject | Tclass _ | Tarraykind _ ),
      Ttuple _ ) ->
    invalid ()
  (* (t1,...,tn) <: (u1,...,un) iff t1<:u1, ... , tn <: un *)
  | (Ttuple tyl_sub, Ttuple tyl_super) ->
    if List.length tyl_super = List.length tyl_sub then
      wfold_left2
        (fun res ty_sub ty_super ->
          res &&& simplify_subtype ~seen_generic_params ty_sub ty_super)
        (env, TL.valid)
        tyl_sub
        tyl_super
    else
      invalid ()
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Ttuple _) ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | ( ( Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _ | Tanon _
      | Tobject | Tclass _ | Tarraykind _ ),
      Tshape _ ) ->
    invalid ()
  | (Tshape (shape_kind_sub, fdm_sub), Tshape (shape_kind_super, fdm_super)) ->
    let (r_sub, r_super) = (fst ety_sub, fst ety_super) in
    (*
     * shape_field_type A <: shape_field_type B iff:
     *   1. A is no more optional than B
     *   2. A's type <: B.type
     *)
    let simplify_subtype_shape_field name res sft_sub sft_super =
      match (sft_sub.sft_optional, sft_super.sft_optional) with
      | (_, true)
      | (false, false) ->
        res
        &&& simplify_subtype
              ~seen_generic_params
              ~this_ty
              sft_sub.sft_ty
              sft_super.sft_ty
      | (true, false) ->
        res
        |> with_error (fun () ->
               let printable_name =
                 TUtils.get_printable_shape_field_name name
               in
               match fst sft_sub.sft_ty with
               | Reason.Rmissing_required_field _ ->
                 Errors.missing_field
                   (Reason.to_pos r_sub)
                   (Reason.to_pos r_super)
                   printable_name
               | _ ->
                 Errors.required_field_is_optional
                   (Reason.to_pos r_sub)
                   (Reason.to_pos r_super)
                   printable_name)
    in
    let lookup_shape_field_type name r shape_kind fdm =
      match ShapeMap.get name fdm with
      | Some sft -> sft
      | None ->
        let printable_name = TUtils.get_printable_shape_field_name name in
        let sft_ty =
          match shape_kind with
          | Closed_shape ->
            MakeType.nothing
              (Reason.Rmissing_required_field (Reason.to_pos r, printable_name))
          | Open_shape ->
            MakeType.mixed
              (Reason.Rmissing_optional_field (Reason.to_pos r, printable_name))
        in
        { sft_ty; sft_optional = true }
    in
    begin
      match (shape_kind_sub, shape_kind_super) with
      | (Open_shape, Closed_shape) ->
        invalid_with (fun () ->
            Errors.shape_fields_unknown
              (Reason.to_pos r_sub)
              (Reason.to_pos r_super))
      | (_, _) ->
        ShapeSet.fold
          (fun name res ->
            simplify_subtype_shape_field
              name
              res
              (lookup_shape_field_type name r_sub shape_kind_sub fdm_sub)
              (lookup_shape_field_type name r_super shape_kind_super fdm_super))
          (ShapeSet.of_list (ShapeMap.keys fdm_sub @ ShapeMap.keys fdm_super))
          (env, TL.valid)
    end
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tshape _) ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (Tclass ((_, class_name), _, _), Tabstract (AKnewtype (enum_name, _), _))
    when Env.is_enum env enum_name && enum_name = class_name ->
    valid ()
  | ( ( Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _
      | Tshape _ | Tanon _ | Tobject | Tclass _ | Tarraykind _ ),
      Tabstract (AKnewtype _, _) ) ->
    invalid ()
  | (Tabstract (AKnewtype (e_sub, _), _), Tabstract (AKnewtype (e_super, _), _))
    when Env.is_enum env e_sub && Env.is_enum env e_super && e_sub = e_super ->
    valid ()
  | ( Tabstract (AKnewtype (name_sub, tyl_sub), _),
      Tabstract (AKnewtype (name_super, tyl_super), _) )
    when name_super = name_sub ->
    let td = Env.get_typedef env name_super in
    begin
      match td with
      | Some { td_tparams; _ } ->
        let variancel = List.map td_tparams (fun t -> t.tp_variance) in
        simplify_subtype_variance
          ~seen_generic_params
          name_sub
          variancel
          tyl_sub
          tyl_super
          env
      | None -> invalid ()
    end
  | ( Tabstract ((AKnewtype _ | AKdependent _), Some ty),
      Tabstract (AKnewtype _, _) ) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (_, Tabstract (AKdependent _, Some ((_, Tclass ((_, x), _, _)) as ty)))
    when is_final_and_not_contravariant env x ->
    (* For final class C, there is no difference between `this as X` and `X`,
     * and `expr<#n> as X` and `X`.
     * But we need to take care with contravariant classes, since we can't
     * statically guarantee their runtime type.
     *)
    simplify_subtype ~seen_generic_params ~this_ty ty_sub ty env
  | ( Tclass _,
      Tabstract (AKdependent DTthis, Some (_, Tclass ((_, x), _, tyl_super)))
    )
    when has_lower_bounds x ->
    let class_def = Env.get_class env x in
    begin
      match class_def with
      | Some class_ty ->
        let p_super = fst ety_super in
        let tyl_super =
          if
            List.is_empty tyl_super
            && not (Partial.should_check_error (Env.get_mode env) 4029)
          then
            List.map (Cls.tparams class_ty) (fun _ ->
                (p_super, Typing_defs.make_tany ()))
          else
            tyl_super
        in
        if List.length (Cls.tparams class_ty) <> List.length tyl_super then
          invalid_with (fun () ->
              Errors.expected_tparam
                ~definition_pos:(Cls.pos class_ty)
                ~use_pos:(Reason.to_pos p_super)
                (List.length (Cls.tparams class_ty)))
        else
          let ety_env =
            {
              type_expansions = [];
              substs = Subst.make (Cls.tparams class_ty) tyl_super;
              this_ty = Option.value this_ty ~default:ty_super;
              from_class = None;
            }
          in
          let lower_bounds_super = Cls.lower_bounds_on_this class_ty in
          let rec try_constraints lower_bounds_super env =
            match Sequence.next lower_bounds_super with
            | None -> invalid_env env
            | Some (ty_super, lower_bounds_super) ->
              let (env, ty_super) = Phase.localize ~ety_env env ty_super in
              env
              |> simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super
              ||| try_constraints lower_bounds_super
          in
          try_constraints lower_bounds_super env
      | None -> invalid ()
    end
  (* Primitives and other concrete types cannot be subtypes of dependent types *)
  | ( ( Tnonnull | Tdynamic | Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tanon _
      | Tclass _ | Tobject | Tarraykind _ ),
      Tabstract (AKdependent expr_dep, tyopt) ) ->
    (* If the bound is the same class try and show more explanation of the error *)
    begin
      match (snd ty_sub, tyopt) with
      | (Tclass ((_, y), _, _), Some (_, Tclass (((_, x) as id), _, _)))
        when y = x ->
        invalid_with (fun () ->
            Errors.try_ fail (fun error ->
                let p = Reason.to_pos (fst ety_sub) in
                if expr_dep = DTcls x then
                  Errors.exact_class_final id p error
                else
                  Errors.this_final id p error))
      | _ -> invalid ()
    end
  | (Tabstract (AKnewtype _, Some ty), Tabstract (AKdependent _, _)) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (Toption _, Tabstract (AKdependent _, Some _)) -> invalid ()
  | ( ( Tnonnull | Tdynamic | Toption _ | Tprim _ | Ttuple _ | Tshape _
      | Tobject | Tclass _ | Tarraykind _ ),
      Tanon _ ) ->
    invalid ()
  | (Tanon (_, id1), Tanon (_, id2)) ->
    if id1 = id2 then
      valid ()
    else
      invalid ()
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tanon _) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (Tfun _, Tanon _) -> invalid ()
  | (Tobject, Tobject) -> valid ()
  (* Any class type is a subtype of object *)
  | (Tclass _, Tobject) -> valid ()
  | ( ( Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _
      | Tshape _ | Tanon _ | Tarraykind _ ),
      Tobject ) ->
    invalid ()
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tobject) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | ( Tabstract (AKnewtype (cid, _), _),
      Tclass ((_, class_name), _, [ty_super']) )
    when Env.is_enum env cid && class_name = SN.Classes.cHH_BuiltinEnum ->
    env
    |> simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super'
    &&& simplify_subtype ~seen_generic_params ~this_ty ty_super' ty_sub
  | ( Tabstract (AKnewtype (enum_name, _), _),
      Tclass ((_, class_name), Nonexact, _) )
    when (Env.is_enum env enum_name && enum_name = class_name)
         || class_name = SN.Classes.cXHPChild ->
    valid ()
  | ( Tabstract (AKnewtype (enum_name, _), Some ty),
      Tclass ((_, class_name), exact, _) )
    when Env.is_enum env enum_name ->
    if enum_name = class_name && exact = Nonexact then
      valid ()
    else
      simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (Tprim Nast.Tstring, Tclass ((_, class_name), exact, _)) ->
    if
      (class_name = SN.Classes.cStringish || class_name = SN.Classes.cXHPChild)
      && exact = Nonexact
    then
      valid ()
    else
      invalid ()
  | ( Tprim Nast.(Tarraykey | Tint | Tfloat | Tnum),
      Tclass ((_, class_name), exact, _) ) ->
    if class_name = SN.Classes.cXHPChild && exact = Nonexact then
      valid ()
    else
      invalid ()
  | ( ( Tnonnull | Tdynamic
      | Tprim Nast.(Tnull | Tvoid | Tbool | Tresource | Tnoreturn | Tatom _)
      | Toption _ | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tpu _ ),
      Tclass _ ) ->
    invalid ()
  (* Match what's done in unify for non-strict code *)
  | (Tobject, Tclass _) ->
    if Partial.should_check_error (Env.get_mode env) 4110 then
      invalid ()
    else
      valid ()
  | ( Tclass (x_sub, exact_sub, tyl_sub),
      Tclass (x_super, exact_super, tyl_super) ) ->
    let exact_match =
      match (exact_sub, exact_super) with
      | (Nonexact, Exact) -> false
      | (_, _) -> true
    in
    let (p_sub, p_super) = (fst ety_sub, fst ety_super) in
    let (cid_super, cid_sub) = (snd x_super, snd x_sub) in
    (* This is side-effecting as it registers a dependency *)
    let class_def_sub = Env.get_class env cid_sub in
    if cid_super = cid_sub then
      (* If class is final then exactness is superfluous *)
      let is_final =
        match class_def_sub with
        | Some tc -> Cls.final tc
        | None -> false
      in
      if not (exact_match || is_final) then
        invalid ()
      else
        (* We handle the case where a generic A<T> is used as A *)
        let tyl_super =
          if
            List.is_empty tyl_super
            && not (Partial.should_check_error (Env.get_mode env) 4101)
          then
            List.map tyl_sub (fun _ -> (p_super, Typing_defs.make_tany ()))
          else
            tyl_super
        in
        let tyl_sub =
          if
            List.is_empty tyl_sub
            && not (Partial.should_check_error (Env.get_mode env) 4101)
          then
            List.map tyl_super (fun _ -> (p_super, Typing_defs.make_tany ()))
          else
            tyl_sub
        in
        if List.length tyl_sub <> List.length tyl_super then
          let n_sub = String_utils.soi (List.length tyl_sub) in
          let n_super = String_utils.soi (List.length tyl_super) in
          invalid_with (fun () ->
              Errors.type_arity_mismatch
                (fst x_super)
                n_super
                (fst x_sub)
                n_sub)
        else if List.is_empty tyl_sub && List.is_empty tyl_super then
          valid ()
        else
          let variancel =
            match class_def_sub with
            | None -> List.map tyl_sub (fun _ -> Ast_defs.Invariant)
            | Some class_sub ->
              List.map (Cls.tparams class_sub) (fun t -> t.tp_variance)
          in
          (* C<t1, .., tn> <: C<u1, .., un> iff
           *   t1 <:v1> u1 /\ ... /\ tn <:vn> un
           * where vi is the variance of the i'th generic parameter of C,
           * and <:v denotes the appropriate direction of subtyping for variance v
           *)
          simplify_subtype_variance
            ~seen_generic_params
            cid_sub
            variancel
            tyl_sub
            tyl_super
            env
    else if not exact_match then
      invalid ()
    else (
      match class_def_sub with
      | None ->
        (* This should have been caught already in the naming phase *)
        valid ()
      | Some class_sub ->
        (* We handle the case where a generic A<T> is used as A *)
        let tyl_sub =
          if
            List.is_empty tyl_sub
            && not (Partial.should_check_error (Env.get_mode env) 4029)
          then
            List.map (Cls.tparams class_sub) (fun _ ->
                (p_sub, Typing_defs.make_tany ()))
          else
            tyl_sub
        in
        if List.length (Cls.tparams class_sub) <> List.length tyl_sub then
          invalid_with (fun () ->
              Errors.expected_tparam
                ~definition_pos:(Cls.pos class_sub)
                ~use_pos:(Reason.to_pos p_sub)
                (List.length (Cls.tparams class_sub)))
        else
          let ety_env =
            {
              type_expansions = [];
              substs = Subst.make (Cls.tparams class_sub) tyl_sub;
              (* TODO: do we need this? *)
              this_ty = Option.value this_ty ~default:ty_sub;
              from_class = None;
            }
          in
          let up_obj = Cls.get_ancestor class_sub cid_super in
          (match up_obj with
          | Some up_obj ->
            let (env, up_obj) = Phase.localize ~ety_env env up_obj in
            simplify_subtype ~seen_generic_params ~this_ty up_obj ty_super env
          | None ->
            if
              Cls.kind class_sub = Ast_defs.Ctrait
              || Cls.kind class_sub = Ast_defs.Cinterface
            then
              let rec try_upper_bounds_on_this up_objs env =
                match Sequence.next up_objs with
                | None ->
                  (* It's crucial that we don't lose updates to global_tpenv in env that were
                   * introduced by PHase.localize. TODO: avoid this requirement *)
                  invalid_env env
                | Some (ub_obj_typ, up_objs) ->
                  (* a trait is never the runtime type, but it can be used
                   * as a constraint if it has requirements or where constraints
                   * for its using classes *)
                  let (env, ub_obj_typ) =
                    Phase.localize ~ety_env env ub_obj_typ
                  in
                  env
                  |> simplify_subtype
                       ~seen_generic_params
                       ~this_ty
                       (p_sub, snd ub_obj_typ)
                       ty_super
                  ||| try_upper_bounds_on_this up_objs
              in
              try_upper_bounds_on_this (Cls.upper_bounds_on_this class_sub) env
            else
              invalid ())
    )
  | (Tabstract (AKnewtype _, Some ty), Tclass _) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (Tarraykind _, Tclass ((_, class_name), Nonexact, _))
    when class_name = SN.Classes.cXHPChild ->
    valid ()
  | (Tarraykind akind, Tclass ((_, coll), Nonexact, [tv_super]))
    when coll = SN.Collections.cTraversable
         || coll = SN.Rx.cTraversable
         || coll = SN.Collections.cContainer ->
    (match akind with
    (* array <: Traversable<_> and emptyarray <: Traversable<t> for any t *)
    | AKany ->
      simplify_subtype
        ~seen_generic_params
        ~this_ty
        (fst ety_sub, Typing_defs.make_tany ())
        tv_super
        env
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
    | AKmap (_, tv) ->
      simplify_subtype ~seen_generic_params ~this_ty tv tv_super env)
  | (Tarraykind akind, Tclass ((_, coll), Nonexact, [tk_super; tv_super]))
    when coll = SN.Collections.cKeyedTraversable
         || coll = SN.Rx.cKeyedTraversable
         || coll = SN.Collections.cKeyedContainer ->
    let r = fst ety_sub in
    (match akind with
    | AKany ->
      env
      |> simplify_subtype
           ~seen_generic_params
           ~this_ty
           (fst ety_sub, Typing_defs.make_tany ())
           tk_super
      &&& simplify_subtype
            ~seen_generic_params
            ~this_ty
            (fst ety_sub, Typing_defs.make_tany ())
            tv_super
    | AKempty -> valid ()
    | AKvarray tv
    | AKvec tv ->
      env
      |> simplify_subtype
           ~seen_generic_params
           ~this_ty
           (MakeType.int r)
           tk_super
      &&& simplify_subtype ~seen_generic_params ~this_ty tv tv_super
    | AKvarray_or_darray tv ->
      let tk_sub =
        MakeType.arraykey (Reason.Rvarray_or_darray_key (Reason.to_pos r))
      in
      env
      |> simplify_subtype ~seen_generic_params ~this_ty tk_sub tk_super
      &&& simplify_subtype ~seen_generic_params ~this_ty tv tv_super
    | AKdarray (tk, tv)
    | AKmap (tk, tv) ->
      env
      |> simplify_subtype ~seen_generic_params ~this_ty tk tk_super
      &&& simplify_subtype ~seen_generic_params ~this_ty tv tv_super)
  | (Tarraykind _, Tclass ((_, coll), Nonexact, []))
    when coll = SN.Collections.cKeyedTraversable
         || coll = SN.Rx.cKeyedTraversable
         || coll = SN.Collections.cKeyedContainer ->
    (* All arrays are subtypes of the untyped KeyedContainer / Traversables *)
    valid ()
  | (Tarraykind _, Tclass _) -> invalid ()
  | (Tabstract (AKdependent _, Some ty), Tclass _) ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  (* Arrays *)
  | (Ttuple _, Tarraykind AKany) ->
    if TypecheckerOptions.disallow_array_as_tuple (Env.get_tcopt env) then
      invalid ()
    else
      valid ()
  | ( ( Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _
      | Tshape _ | Tanon _ | Tobject | Tclass _ ),
      Tarraykind _ ) ->
    invalid ()
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tarraykind _) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (Tarraykind ak_sub, Tarraykind ak_super) ->
    let r = fst ety_sub in
    begin
      match (ak_sub, ak_super) with
      (* An array of any kind is a subtype of an array of AKany *)
      | (_, AKany) -> valid ()
      (* An empty array is a subtype of any array type *)
      | (AKempty, _) -> valid ()
      (* array is a subtype of varray_or_darray<_> *)
      | (AKany, AKvarray_or_darray (_, Tany _)) -> valid ()
      | (AKany, _) ->
        let safe_array = TypecheckerOptions.safe_array (Env.get_tcopt env) in
        if safe_array then
          invalid ()
        else
          valid ()
      (* varray_or_darray<ty1> <: varray_or_darray<ty2> iff t1 <: ty2
       But, varray_or_darray<ty1> is never a subtype of a vect-like array *)
      | (AKvarray_or_darray ty_sub, AKvarray_or_darray ty_super) ->
        simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super env
      | ( (AKvarray ty_sub | AKvec ty_sub),
          (AKvarray ty_super | AKvec ty_super | AKvarray_or_darray ty_super) )
        ->
        simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super env
      | ( (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub)),
          (AKdarray (tk_super, tv_super) | AKmap (tk_super, tv_super)) ) ->
        env
        |> simplify_subtype ~seen_generic_params ~this_ty tk_sub tk_super
        &&& simplify_subtype ~seen_generic_params ~this_ty tv_sub tv_super
      | ( (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub)),
          AKvarray_or_darray tv_super ) ->
        let tk_super =
          MakeType.arraykey
            (Reason.Rvarray_or_darray_key (Reason.to_pos (fst ety_super)))
        in
        env
        |> simplify_subtype ~seen_generic_params ~this_ty tk_sub tk_super
        &&& simplify_subtype ~seen_generic_params ~this_ty tv_sub tv_super
      | ((AKvarray elt_ty | AKvec elt_ty), (AKdarray _ | AKmap _))
        when not (TypecheckerOptions.safe_vector_array (Env.get_tcopt env)) ->
        let int_reason = Reason.Ridx (Reason.to_pos r, Reason.Rnone) in
        let int_type = MakeType.int int_reason in
        simplify_subtype
          ~seen_generic_params
          ~this_ty
          (r, Tarraykind (AKmap (int_type, elt_ty)))
          ty_super
          env
      (* any other array subtyping is unsatisfiable *)
      | _ -> invalid ()
    end
  (* List destructuring *)
  | (Ttuple tyl, Tdestructure tyl_dest) ->
    if List.length tyl <> List.length tyl_dest then
      invalid ()
    else
      List.fold2_exn
        tyl
        tyl_dest
        ~init:(env, TL.valid)
        ~f:(fun res ty ty_dest ->
          res &&& simplify_subtype ~seen_generic_params ~this_ty ty ty_dest)
  | (Tclass ((_, x), _, [elt_type]), Tdestructure tyl_dest)
    when x = SN.Collections.cVector
         || x = SN.Collections.cImmVector
         || x = SN.Collections.cVec
         || x = SN.Collections.cConstVector ->
    List.fold tyl_dest ~init:(env, TL.valid) ~f:(fun res ty_dest ->
        res &&& simplify_subtype ~seen_generic_params ~this_ty elt_type ty_dest)
  | (Tclass ((_, x), _, tyl), Tdestructure tyl_dest)
    when x = SN.Collections.cPair ->
    if List.length tyl <> List.length tyl_dest then
      invalid ()
    else
      List.fold2_exn
        tyl
        tyl_dest
        ~init:(env, TL.valid)
        ~f:(fun res ty ty_dest ->
          res &&& simplify_subtype ~seen_generic_params ~this_ty ty ty_dest)
  | (Tarraykind (AKvec elt_type), Tdestructure tyl_dest)
  | (Tarraykind (AKvarray elt_type), Tdestructure tyl_dest) ->
    List.fold tyl_dest ~init:(env, TL.valid) ~f:(fun res ty_dest ->
        res &&& simplify_subtype ~seen_generic_params ~this_ty elt_type ty_dest)
  | (Tdynamic, Tdestructure tyl_dest) ->
    List.fold tyl_dest ~init:(env, TL.valid) ~f:(fun res ty_dest ->
        res &&& simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_dest)
  (* TODO: should remove these any cases *)
  | (Tarraykind (AKany | AKempty), Tdestructure tyl_dest)
  | (Tany _, Tdestructure tyl_dest) ->
    let any = (fst ty_super, Typing_defs.make_tany ()) in
    List.fold tyl_dest ~init:(env, TL.valid) ~f:(fun res ty_dest ->
        res &&& simplify_subtype ~seen_generic_params ~this_ty any ty_dest)
  (* ty_sub <: union{ty_super'} iff ty_sub <: ty_super' *)
  | (_, Tunion [ty_super']) ->
    simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super' env
  (* t1 | ... | tn <: t
   *   if and only if
   * t1 <: t /\ ... /\ tn <: t
   * We want this even if t is a type variable e.g. consider
   *   int | v <: v
   *)
  | (Tunion tyl, _) ->
    List.fold_left tyl ~init:(env, TL.valid) ~f:(fun res ty_sub ->
        res &&& simplify_subtype ~seen_generic_params ty_sub ty_super)
  (* t <: (t1 & ... & tn)
   *   if and only if
   * t <: t1 /\  ... /\ t <: tn
   *)
  | (_, Tintersection tyl) ->
    List.fold_left tyl ~init:(env, TL.valid) ~f:(fun res ty_super ->
        res &&& simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super)
  (* We want to treat nullable as a union with the same rule as above.
   * This is only needed for Tvar on right; other cases are dealt with specially as
   * derived rules.
   *)
  | (Toption t, Tvar _) ->
    env
    |> simplify_subtype ~seen_generic_params ~this_ty t ty_super
    &&& simplify_subtype
          ~seen_generic_params
          ~this_ty
          (MakeType.null (fst ety_sub))
          ty_super
  | (Terr, Terr) -> valid ()
  | (Terr, _)
  | (_, Terr) ->
    if no_top_bottom then
      default ()
    else
      valid ()
  | (Tvar _, _)
  | (_, Tvar _) ->
    default ()
  (* A & B <: C iif A <: C | !B *)
  | (Tintersection tyl, _)
    when let (_, non_ty_opt, _) = find_type_with_exact_negation env tyl in
         Option.is_some non_ty_opt ->
    let (env, non_ty_opt, tyl') = find_type_with_exact_negation env tyl in
    let non_ty = Option.value_exn non_ty_opt in
    let (env, ty_super') = TUtils.union env ty_super non_ty in
    let ty_sub' = MakeType.intersection (fst ety_sub) tyl' in
    simplify_subtype ~seen_generic_params ty_sub' ty_super' env
  (* A <: ?B iif A & nonnull <: B
  Only apply if B is a type variable or an intersection, to avoid oscillating
  forever between this case and the previous one.*)
  | (_, Toption ty_super')
    when let (_, (_, ety_super')) = Env.expand_type env ty_super' in
         match ety_super' with
         | Tintersection _
         | Tvar _ ->
           true
         | _ -> false ->
    let (env, ty_sub') =
      let r = fst ety_super in
      Inter.intersect env r ety_sub (MakeType.nonnull r)
    in
    simplify_subtype ~seen_generic_params ty_sub' ty_super' env
  (* If subtype and supertype are the same generic parameter, we're done *)
  | (Tabstract (AKgeneric name_sub, _), Tabstract (AKgeneric name_super, _))
    when name_sub = name_super ->
    valid ()
  (* When decomposing subtypes for the purpose of adding bounds on generic
   * parameters to the context, (so seen_generic_params = None), leave
   * subtype so that the bounds get added *)
  | (Tabstract (AKgeneric _, _), _)
  | (_, Tabstract (AKgeneric _, _))
    when Option.is_none seen_generic_params ->
    default ()
  (* Num is not atomic: it is equivalent to int|float. The rule below relies
   * on ty_sub not being a union e.g. consider num <: arraykey | float, so
   * we break out num first.
   *)
  | (Tprim Nast.Tnum, Tunion _) ->
    let r = fst ty_sub in
    env
    |> simplify_subtype
         ~seen_generic_params
         ~this_ty
         (MakeType.float r)
         ty_super
    &&& simplify_subtype
          ~seen_generic_params
          ~this_ty
          (MakeType.int r)
          ty_super
  (* Likewise, reduce nullable on left to a union *)
  | (Toption ty, Tunion _) ->
    let r = fst ty_sub in
    let (env, p1) =
      simplify_subtype
        ~seen_generic_params
        ~this_ty
        (MakeType.null r)
        ty_super
        env
    in
    if TL.is_unsat p1 then
      invalid ()
    else
      let (env, p2) =
        simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
      in
      (env, TL.conj p1 p2)
  | (Tabstract ((AKnewtype _ | AKdependent _), Some ty), Tunion []) ->
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
  | (Tabstract (AKgeneric name_sub, opt_sub_cstr), Tunion []) ->
    simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super env
  | ( ( Tnonnull | Tdynamic | Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tanon _
      | Tobject | Tclass _ | Tarraykind _ ),
      Tunion [] ) ->
    invalid ()
  | (_, Tunion (_ :: _ as tyl)) ->
    (* It's sound to reduce t <: t1 | t2 to (t <: t1) || (t <: t2). But
     * not complete e.g. consider (t1 | t3) <: (t1 | t2) | (t2 | t3).
     * But we deal with unions on the left first (see case above), so this
     * particular situation won't arise.
     * TODO: identify under what circumstances this reduction is complete.
     *)
    let rec try_each tys env =
      match tys with
      | [] ->
        (* If type on left might have an explicit upper bound (e.g. generic parameters, new type)
         * then we'd better check this too. e.g. consider foo<T as ~int> and T <: ~num
         *)
        begin
          match snd ty_sub with
          | Tabstract ((AKnewtype _ | AKdependent _), Some ty) ->
            simplify_subtype ~seen_generic_params ~this_ty ty ty_super env
          | Tabstract (AKgeneric name_sub, opt_sub_cstr) ->
            simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super env
          | _ -> invalid ()
        end
      | ty :: tys ->
        env
        |> simplify_subtype ~seen_generic_params ~this_ty ty_sub ty
        ||| try_each tys
    in
    try_each tyl env
  (* It's sound to reduce t1 & t2 <: t to (t1 <: t) || (t2 <: t), but
   * not complete.
   *)
  | (Tintersection tyl, _) ->
    List.fold_left
      tyl
      ~init:(env, TL.invalid ~fail)
      ~f:(fun res ty_sub ->
        res ||| simplify_subtype ~seen_generic_params ~this_ty ty_sub ty_super)
  | (Tany _, Tany _) -> valid ()
  (* If ty_sub contains other types, e.g. C<T>, make this a subtype assertion on
  those inner types and `any`. For example transform the assertion
    C<D> <: Tany
  into
    C<D> <: C<Tany>
  which might become
    D <: Tany
  if say C is covariant.
  *)
  | (_, Tany _) ->
    if no_top_bottom then
      default ()
    else
      let ety_super = anyfy env (fst ety_super) ety_sub in
      simplify_subtype ~seen_generic_params ~this_ty ety_sub ety_super env
  | (Tany _, _) ->
    if no_top_bottom then
      default ()
    else
      let ety_sub = anyfy env (fst ety_sub) ety_super in
      simplify_subtype ~seen_generic_params ~this_ty ety_sub ety_super env
  (* Supertype is generic parameter *and* subtype is a newtype with bound.
   * We need to make this a special case because there is a *choice*
   * of subtyping rule to apply. See details in the case of dependent type
   * against generic parameter which is similar
   *)
  | (Tabstract (AKnewtype (_, _), Some ty), Tabstract (AKgeneric name_super, _))
    when Option.is_some seen_generic_params ->
    env
    |> simplify_subtype ~seen_generic_params ~this_ty ty ty_super
    ||| simplify_subtype_generic_super ty_sub name_super
  (* void behaves with respect to subtyping as an abstract type with
   * an implicit upper bound ?nonnull
   *)
  | (Tprim Nast.Tvoid, Tabstract (AKgeneric name_super, _))
    when Option.is_some seen_generic_params ->
    let r =
      Reason.Rimplicit_upper_bound (Reason.to_pos (fst ety_sub), "?nonnull")
    in
    let tmixed = (r, Toption (r, Tnonnull)) in
    env
    |> simplify_subtype ~seen_generic_params ~this_ty tmixed ty_super
    ||| simplify_subtype_generic_super ty_sub name_super
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
  | (Tabstract (AKdependent _, Some ty), Tabstract (AKgeneric name_super, _))
    when Option.is_some seen_generic_params ->
    env
    |> simplify_subtype_generic_super ty_sub name_super
    |||
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~seen_generic_params ~this_ty ty ty_super
  (* Subtype or supertype is generic parameter
   * We delegate these cases to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
   *)
  | (Tabstract (AKgeneric name_sub, opt_sub_cstr), _) ->
    simplify_subtype_generic_sub name_sub opt_sub_cstr ty_super env
  | (_, Tabstract (AKgeneric name_super, _)) ->
    simplify_subtype_generic_super ty_sub name_super env
  | (Tdestructure _, _) -> invalid ()
  | (_, Tdestructure _) -> invalid ()
  | (Tpu _, _)
  | (_, Tpu _)
  | (Tpu_access _, _)
  | (_, Tpu_access _) ->
    (* TODO(T36532263) implement subtyping *)
    invalid ()

and simplify_subtype_variance
    ~(seen_generic_params : SSet.t option)
    ~(no_top_bottom : bool)
    (cid : string)
    (variancel : Ast_defs.variance list)
    (children_tyl : locl_ty list)
    (super_tyl : locl_ty list)
    ~(on_error : Errors.typing_error_callback) : env -> env * TL.subtype_prop =
 fun env ->
  let simplify_subtype =
    simplify_subtype
      ~no_top_bottom
      ~seen_generic_params
      ~on_error
      ~this_ty:None
  in
  let simplify_subtype_variance =
    simplify_subtype_variance ~no_top_bottom ~seen_generic_params ~on_error
  in
  match (variancel, children_tyl, super_tyl) with
  | ([], _, _)
  | (_, [], _)
  | (_, _, []) ->
    valid env
  | (variance :: variancel, child :: childrenl, super :: superl) ->
    begin
      match variance with
      | Ast_defs.Covariant -> simplify_subtype child super env
      | Ast_defs.Contravariant ->
        let super =
          ( Reason.Rcontravariant_generic (fst super, Utils.strip_ns cid),
            snd super )
        in
        simplify_subtype super child env
      | Ast_defs.Invariant ->
        let super' =
          (Reason.Rinvariant_generic (fst super, Utils.strip_ns cid), snd super)
        in
        env |> simplify_subtype child super' &&& simplify_subtype super' child
    end
    &&& simplify_subtype_variance cid variancel childrenl superl

and simplify_subtype_params
    ~(seen_generic_params : SSet.t option)
    ~(no_top_bottom : bool)
    ?(is_method : bool = false)
    ?(check_params_reactivity = false)
    ?(check_params_mutability = false)
    (subl : locl_fun_param list)
    (superl : locl_fun_param list)
    (variadic_sub_ty : locl_possibly_enforced_ty option)
    (variadic_super_ty : locl_possibly_enforced_ty option)
    ~(on_error : Errors.typing_error_callback)
    env =
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced
      ~seen_generic_params
      ~no_top_bottom
      ~on_error
  in
  let simplify_subtype_params =
    simplify_subtype_params ~seen_generic_params ~no_top_bottom ~on_error
  in
  let simplify_subtype_params_with_variadic =
    simplify_subtype_params_with_variadic ~seen_generic_params ~no_top_bottom
  in
  let simplify_supertype_params_with_variadic =
    simplify_supertype_params_with_variadic ~seen_generic_params ~no_top_bottom
  in
  match (subl, superl) with
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
  | ([], _) ->
    (match variadic_super_ty with
    | None -> valid env
    | Some ty -> simplify_supertype_params_with_variadic superl ty on_error env)
  | (_, []) ->
    (match variadic_sub_ty with
    | None -> valid env
    | Some ty -> simplify_subtype_params_with_variadic subl ty on_error env)
  | (sub :: subl, super :: superl) ->
    env
    |> begin
         if check_params_reactivity then
           subtype_fun_params_reactivity sub super
         else valid
       end
    &&& begin
          if check_params_mutability then
            check_mutability
              ~is_receiver:false
              sub.fp_pos
              sub.fp_mutability
              super.fp_pos
              super.fp_mutability
          else valid
        end
    &&& fun env ->
    begin
      let { fp_type = ty_sub; _ } = sub in
      let { fp_type = ty_super; _ } = super in
      (* Check that the calling conventions of the params are compatible.
       * We don't currently raise an error for reffiness because function
       * hints don't support '&' annotations (enforce_ctpbr = false). *)
      Unify.unify_param_modes ~enforce_ctpbr:is_method sub super;
      Unify.unify_accept_disposable sub super;
      match (sub.fp_kind, super.fp_kind) with
      | (FPinout, FPinout) ->
        (* Inout parameters are invariant wrt subtyping for function types. *)
        env
        |> simplify_subtype_possibly_enforced ty_super ty_sub
        &&& simplify_subtype_possibly_enforced ty_sub ty_super
      | _ -> env |> simplify_subtype_possibly_enforced ty_sub ty_super
    end
    &&& simplify_subtype_params
          ~is_method
          subl
          superl
          variadic_sub_ty
          variadic_super_ty

and simplify_subtype_params_with_variadic
    ~(seen_generic_params : SSet.t option)
    ~(no_top_bottom : bool)
    (subl : locl_fun_param list)
    (variadic_ty : locl_possibly_enforced_ty)
    (on_error : Errors.typing_error_callback)
    env =
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced
      ~seen_generic_params
      ~no_top_bottom
      ~on_error
  in
  let simplify_subtype_params_with_variadic =
    simplify_subtype_params_with_variadic ~seen_generic_params ~no_top_bottom
  in
  match subl with
  | [] -> valid env
  | { fp_type = sub; _ } :: subl ->
    env
    |> simplify_subtype_possibly_enforced sub variadic_ty
    &&& simplify_subtype_params_with_variadic subl variadic_ty on_error

and simplify_supertype_params_with_variadic
    ~(seen_generic_params : SSet.t option)
    ~(no_top_bottom : bool)
    (superl : locl_fun_param list)
    (variadic_ty : locl_possibly_enforced_ty)
    (on_error : Errors.typing_error_callback)
    env =
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced
      ~seen_generic_params
      ~no_top_bottom
      ~on_error
  in
  let simplify_supertype_params_with_variadic =
    simplify_supertype_params_with_variadic ~seen_generic_params ~no_top_bottom
  in
  match superl with
  | [] -> valid env
  | { fp_type = super; _ } :: superl ->
    env
    |> simplify_subtype_possibly_enforced variadic_ty super
    &&& simplify_supertype_params_with_variadic superl variadic_ty on_error

and subtype_reactivity
    ?(extra_info : reactivity_extra_info option)
    ?(is_call_site = false)
    (env : env)
    (r_sub : reactivity)
    (r_super : reactivity) : bool =
  let maybe_localize t =
    match t with
    | DeclTy t ->
      let ety_env = Phase.env_with_self env in
      let (_, t) = Phase.localize ~ety_env env t in
      t
    | LoclTy t -> t
  in
  let class_ty =
    Option.bind extra_info (fun { class_ty = cls; _ } ->
        Option.map cls ~f:maybe_localize)
  in
  (* for method declarations check if condition type for r_super includes
     reactive method with a matching name. If yes - then it will act as a guarantee
     that derived class will have to redefine the method with a shape required
     by condition type (reactivity of redefined method must be subtype of reactivity
     of method in interface) *)
  let condition_type_has_matching_reactive_method
      condition_type_super (method_name, is_static) =
    let m =
      ConditionTypes.try_get_method_from_condition_type
        env
        condition_type_super
        is_static
        method_name
    in
    match m with
    | Some { ce_type = (lazy (_, Tfun f)); _ } ->
      (* check that reactivity of interface method (effectively a promised
         reactivity of a method in derived class) is a subtype of r_super.
         NOTE: we check only for unconditional reactivity since conditional
         version does not seems to yield a lot and will requre implementing
         cycle detection for condition types *)
      begin
        match f.ft_reactive with
        | Reactive None
        | Shallow None
        | Local None ->
          let extra_info =
            {
              empty_extra_info with
              parent_class_ty = Some (DeclTy condition_type_super);
            }
          in
          subtype_reactivity ~extra_info env f.ft_reactive r_super
        | _ -> false
      end
    | _ -> false
  in
  match (r_sub, r_super, extra_info) with
  (* anything is a subtype of nonreactive functions *)
  | (_, Nonreactive, _) -> true
  (* to compare two maybe reactive values we need to unwrap them *)
  | (MaybeReactive sub, MaybeReactive super, _) ->
    subtype_reactivity ?extra_info ~is_call_site env sub super
  (* for explicit checks at callsites implicitly unwrap maybereactive value:
     function f(<<__AtMostRxAsFunc>> F $f)
     f(<<__RxLocal>> () ==> {... })
     here parameter will be maybereactive and argument - rxlocal
     *)
  | (sub, MaybeReactive super, _) when is_call_site ->
    subtype_reactivity ?extra_info ~is_call_site env sub super
  (* if is_call_site is falst ignore maybereactive flavors.
     This usually happens during subtype checks for arguments and when target
     function is conditionally reactive we'll do the proper check
     in typing_reactivity.check_call. *)
  | (_, MaybeReactive _, _) when not is_call_site -> true
  (* ok:
    class A { function f((function(): int) $f) {} }
    class B extends A {
      <<__Rx>>
      function f(<<__AtMostRxAsFunc>> (function(): int) $f);
    }
    reactivity for arguments is checked contravariantly *)
  | (_, RxVar None, _)
  (* ok:
     <<__Rx>>
     function f(<<__AtMostRxAsFunc>> (function(): int) $f) { return $f() }  *)
  
  | (RxVar None, RxVar _, _) ->
    true
  | (RxVar (Some sub), RxVar (Some super), _)
  | (sub, RxVar (Some super), _) ->
    subtype_reactivity ?extra_info ~is_call_site env sub super
  | (RxVar _, _, _) -> false
  | ( (Local cond_sub | Shallow cond_sub | Reactive cond_sub),
      Local cond_super,
      _ )
  | ((Shallow cond_sub | Reactive cond_sub), Shallow cond_super, _)
  | (Reactive cond_sub, Reactive cond_super, _)
    when subtype_param_rx_if_impl
           ~is_param:false
           env
           cond_sub
           class_ty
           cond_super ->
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
  | ( _,
      (Reactive (Some t) | Shallow (Some t) | Local (Some t)),
      Some { method_info = Some mi; _ } )
    when condition_type_has_matching_reactive_method t mi ->
    true
  (* call_site specific cases *)
  (* shallow can call into local *)
  | (Local cond_sub, Shallow cond_super, _)
    when is_call_site
         && subtype_param_rx_if_impl
              ~is_param:false
              env
              cond_sub
              class_ty
              cond_super ->
    true
  (* local can call into non-reactive *)
  | (Nonreactive, Local _, _) when is_call_site -> true
  | _ -> false

and should_check_fun_params_reactivity (ft_super : locl_fun_type) =
  ft_super.ft_reactive <> Nonreactive

(* checks condition described by OnlyRxIfImpl condition on parameter is met  *)
and subtype_param_rx_if_impl
    ~is_param
    (env : env)
    (cond_type_sub : decl_ty option)
    (declared_type_sub : locl_ty option)
    (cond_type_super : decl_ty option) =
  let cond_type_sub =
    Option.map cond_type_sub ~f:(ConditionTypes.localize_condition_type env)
  in
  let cond_type_super =
    Option.map cond_type_super ~f:(ConditionTypes.localize_condition_type env)
  in
  match (cond_type_sub, cond_type_super) with
  (* no condition types - do nothing *)
  | (None, None) -> true
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
  | (None, Some _) when not is_param -> true
  | (None, Some cond_type_super) ->
    Option.value_map
      declared_type_sub
      ~default:false
      ~f:(fun declared_type_sub ->
        is_sub_type_LEGACY_DEPRECATED env declared_type_sub cond_type_super)
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
  | (Some cond_type_sub, Some cond_type_super) ->
    is_sub_type_LEGACY_DEPRECATED env cond_type_super cond_type_sub
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
  | (Some cond_type_sub, None) ->
    Option.value_map
      declared_type_sub
      ~default:false
      ~f:(fun declared_type_sub ->
        is_sub_type_LEGACY_DEPRECATED env declared_type_sub cond_type_sub)

(* checks reactivity conditions for function parameters *)
and subtype_fun_params_reactivity
    (p_sub : locl_fun_param) (p_super : locl_fun_param) env =
  match (p_sub.fp_rx_annotation, p_super.fp_rx_annotation) with
  (* no conditions on parameters - do nothing *)
  | (None, None) -> valid env
  (* both parameters are conditioned to be rx function - no need to check anything *)
  | (Some Param_rx_var, Some Param_rx_var) -> valid env
  | (None, Some Param_rx_var) ->
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
    let (_, p_sub_type) = Env.expand_type env p_sub.fp_type.et_type in
    begin
      match p_sub_type with
      | (_, Tfun tfun) when tfun.ft_reactive <> Nonreactive -> valid env
      | (_, Tfun _) ->
        ( env,
          TL.invalid ~fail:(fun () ->
              Errors.rx_parameter_condition_mismatch
                SN.UserAttributes.uaAtMostRxAsFunc
                p_sub.fp_pos
                p_super.fp_pos) )
      (* parameter type is not function - error will be reported in different place *)
      | _ -> valid env
    end
  | (cond_sub, cond_super) ->
    let cond_type_sub =
      match cond_sub with
      | Some (Param_rx_if_impl t) -> Some t
      | _ -> None
    in
    let cond_type_super =
      match cond_super with
      | Some (Param_rx_if_impl t) -> Some t
      | _ -> None
    in
    let ok =
      subtype_param_rx_if_impl
        ~is_param:true
        env
        cond_type_sub
        (Some p_sub.fp_type.et_type)
        cond_type_super
    in
    check_with
      ok
      (fun () ->
        Errors.rx_parameter_condition_mismatch
          SN.UserAttributes.uaOnlyRxIfImpl
          p_sub.fp_pos
          p_super.fp_pos)
      (env, TL.valid)

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
    ?(extra_info : reactivity_extra_info option)
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    env =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  (env, TL.valid)
  |> check_with
       (subtype_reactivity
          ?extra_info
          env
          ft_sub.ft_reactive
          ft_super.ft_reactive)
       (fun () ->
         Errors.fun_reactivity_mismatch
           p_super
           (TUtils.reactivity_to_string env ft_super.ft_reactive)
           p_sub
           (TUtils.reactivity_to_string env ft_sub.ft_reactive))
  |> check_with (ft_sub.ft_is_coroutine = ft_super.ft_is_coroutine) (fun () ->
         Errors.coroutinness_mismatch ft_super.ft_is_coroutine p_super p_sub)
  |> check_with
       (ft_sub.ft_return_disposable = ft_super.ft_return_disposable)
       (fun () ->
         Errors.return_disposable_mismatch
           ft_super.ft_return_disposable
           p_super
           p_sub)
  |> (* it is ok for subclass to return mutably owned value and treat it as immutable -
  the fact that value is mutably owned guarantees it has only single reference so
  as a result this single reference will be immutable. However if super type
  returns mutable value and subtype yields immutable value - this is not safe.
  NOTE: error is not reported if child is non-reactive since it does not have
  immutability-by-default behavior *)
     check_with
       ( ft_sub.ft_returns_mutable = ft_super.ft_returns_mutable
       || (not ft_super.ft_returns_mutable)
       || ft_sub.ft_reactive = Nonreactive )
       (fun () ->
         Errors.mutable_return_result_mismatch
           ft_super.ft_returns_mutable
           p_super
           p_sub)
  |> check_with
       ( ft_super.ft_reactive = Nonreactive
       || ft_super.ft_returns_void_to_rx
       || not ft_sub.ft_returns_void_to_rx )
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
         Errors.return_void_to_rx_mismatch
           ~pos1_has_attribute:true
           p_sub
           p_super)
  |>
  (* check mutability only for reactive functions *)
  let check_params_mutability =
    ft_super.ft_reactive <> Nonreactive && ft_sub.ft_reactive <> Nonreactive
  in
  fun (env, prop) ->
    ( if check_params_mutability (* check mutability of receivers *) then
      (env, prop)
      &&& check_mutability
            ~is_receiver:true
            p_super
            ft_super.ft_mutability
            p_sub
            ft_sub.ft_mutability
    else
      (env, prop) )
    |> check_with
         (arity_min ft_sub.ft_arity <= arity_min ft_super.ft_arity)
         (fun () -> Errors.fun_too_many_args p_sub p_super)
    |> fun res ->
    match (ft_sub.ft_arity, ft_super.ft_arity) with
    | (Fellipsis _, Fvariadic _) ->
      (* The HHVM runtime ignores "..." entirely, but knows about
       * "...$args"; for contexts for which the runtime enforces method
       * compatibility (currently, inheritance from abstract/interface
       * methods), letting "..." override "...$args" would result in method
       * compatibility errors at runtime. *)
      with_error
        (fun () -> Errors.fun_variadicity_hh_vs_php56 p_sub p_super)
        res
    | (Fstandard (_, sub_max), Fstandard (_, super_max)) ->
      if sub_max < super_max then
        with_error (fun () -> Errors.fun_too_few_args p_sub p_super) res
      else
        res
    | (Fstandard _, _) ->
      with_error
        (fun () -> Errors.fun_unexpected_nonvariadic p_sub p_super)
        res
    | (_, _) -> res

and simplify_subtype_possibly_enforced
    ~(seen_generic_params : SSet.t option)
    ~(no_top_bottom : bool)
    et_sub
    et_super
    on_error =
  simplify_subtype
    ~seen_generic_params
    ~no_top_bottom
    et_sub.et_type
    et_super.et_type
    on_error

(* This implements basic subtyping on non-generic function types:
 *   (1) return type behaves covariantly
 *   (2) parameter types behave contravariantly
 *   (3) special casing for variadics, and various reactivity and mutability attributes
 *)
and simplify_subtype_funs
    ~(seen_generic_params : SSet.t option)
    ~(no_top_bottom : bool)
    ~(check_return : bool)
    ?(extra_info : reactivity_extra_info option)
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    (on_error : Errors.typing_error_callback)
    env : env * TL.subtype_prop =
  let variadic_subtype =
    match ft_sub.ft_arity with
    | Fvariadic (_, { fp_type = var_sub; _ }) -> Some var_sub
    | _ -> None
  in
  let variadic_supertype =
    match ft_super.ft_arity with
    | Fvariadic (_, { fp_type = var_super; _ }) -> Some var_super
    | _ -> None
  in
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced
      ~seen_generic_params
      ~no_top_bottom
      ~on_error
  in
  let simplify_subtype_params =
    simplify_subtype_params ~seen_generic_params ~no_top_bottom ~on_error
  in
  (* First apply checks on attributes, coroutine-ness and variadic arity *)
  env
  |> check_subtype_funs_attributes ?extra_info r_sub ft_sub r_super ft_super
  &&& (* Now do contravariant subtyping on parameters *)
      begin
        match (variadic_subtype, variadic_supertype) with
        | (Some var_sub, Some var_super) ->
          simplify_subtype_possibly_enforced var_super var_sub
        | _ -> valid
      end
  &&& begin
        let check_params_mutability =
          ft_super.ft_reactive <> Nonreactive
          && ft_sub.ft_reactive <> Nonreactive
        in
        let is_method =
          Option.map extra_info (fun i -> Option.is_some i.method_info)
          = Some true
        in
        simplify_subtype_params
          ~is_method
          ~check_params_reactivity:
            (should_check_fun_params_reactivity ft_super)
          ~check_params_mutability
          ft_super.ft_params
          ft_sub.ft_params
          variadic_subtype
          variadic_supertype
      end
  &&&
  (* Finally do covariant subtryping on return type *)
  if check_return then
    simplify_subtype_possibly_enforced ft_sub.ft_ret ft_super.ft_ret
  else
    valid

(* One of the main entry points to this module *)
and sub_type
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Errors.typing_error_callback) : env =
  Env.log_env_change "sub_type" env
  @@ sub_type_inner env ~this_ty:None ty_sub ty_super on_error

(* Add a new upper bound ty on var.  Apply transitivity of sutyping,
 * so if we already have tyl <: var, then check that for each ty_sub
 * in tyl we have ty_sub <: ty.
 *)
and add_tyvar_upper_bound_and_close env var ty on_error =
  Env.log_env_change "add_tyvar_upper_bound_and_close" env
  @@
  let upper_bounds_before = Env.get_tyvar_upper_bounds env var in
  let env =
    Env.add_tyvar_upper_bound ~intersect:(try_intersect env) env var ty
  in
  let upper_bounds_after = Env.get_tyvar_upper_bounds env var in
  let added_upper_bounds =
    Typing_set.diff upper_bounds_after upper_bounds_before
  in
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  let env =
    Typing_set.fold
      (fun upper_bound env ->
        let env =
          Typing_subtype_tconst.make_all_type_consts_equal
            env
            var
            upper_bound
            ~as_tyvar_with_cnstr:true
        in
        Typing_set.fold
          (fun lower_bound env ->
            sub_type env lower_bound upper_bound on_error)
          lower_bounds
          env)
      added_upper_bounds
      env
  in
  env

(* Add a new lower bound ty on var.  Apply transitivity of sutyping,
 * so if we already have var <: tyl, then check that for each ty_super
 * in tyl we have ty <: ty_super.
 *)
and add_tyvar_lower_bound_and_close env var ty on_error =
  Env.log_env_change "add_tyvar_lower_bound_and_close" env
  @@
  let lower_bounds_before = Env.get_tyvar_lower_bounds env var in
  let env = Env.add_tyvar_lower_bound ~union:(try_union env) env var ty in
  let lower_bounds_after = Env.get_tyvar_lower_bounds env var in
  let added_lower_bounds =
    Typing_set.diff lower_bounds_after lower_bounds_before
  in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let env =
    Typing_set.fold
      (fun lower_bound env ->
        let env =
          Typing_subtype_tconst.make_all_type_consts_equal
            env
            var
            lower_bound
            ~as_tyvar_with_cnstr:false
        in
        Typing_set.fold
          (fun upper_bound env ->
            sub_type env lower_bound upper_bound on_error)
          upper_bounds
          env)
      added_lower_bounds
      env
  in
  env

and props_to_env env remain props on_error =
  match props with
  | [] -> (env, List.rev remain)
  | TL.IsSubtype
      (((_, Tvar var_sub) as ty_sub), ((_, Tvar var_super) as ty_super))
    :: props ->
    let env = add_tyvar_upper_bound_and_close env var_sub ty_super on_error in
    let env = add_tyvar_lower_bound_and_close env var_super ty_sub on_error in
    props_to_env env remain props on_error
  | TL.IsSubtype ((_, Tvar var), ty) :: props ->
    let env = add_tyvar_upper_bound_and_close env var ty on_error in
    props_to_env env remain props on_error
  | TL.IsSubtype (ty, (_, Tvar var)) :: props ->
    let env = add_tyvar_lower_bound_and_close env var ty on_error in
    props_to_env env remain props on_error
  | TL.Conj props' :: props ->
    props_to_env env remain (props' @ props) on_error
  | TL.Disj (f, disj_props) :: conj_props ->
    (* For now, just find the first prop in the disjunction that works *)
    let rec try_disj disj_props =
      match disj_props with
      | [] ->
        (* For now let it fail later when calling
        process_simplify_subtype_result on the remaining constraints. *)
        props_to_env env (TL.invalid ~fail:f :: remain) conj_props on_error
      | prop :: disj_props' ->
        Errors.try_
          (fun () ->
            ignore_hh_fixmes (fun () ->
                props_to_env env remain (prop :: conj_props) on_error))
          (fun _ -> try_disj disj_props')
    in
    try_disj disj_props
  | prop :: props -> props_to_env env (prop :: remain) props on_error

(* Move any top-level conjuncts of the form Tvar v <: t or t <: Tvar v to
 * the type variable environment. To do: use intersection and union to
 * simplify bounds.
 *)
and prop_to_env env prop on_error =
  let (env, props') = props_to_env env [] [prop] on_error in
  (env, TL.conj_list props')

and env_to_prop env = TL.conj (tvenv_to_prop env.tvenv) env.subtype_prop

and tvenv_to_prop tvenv =
  let props_per_tvar =
    IMap.mapi
      (fun id tyvar_info ->
        match tyvar_info with
        | LocalTyvar { lower_bounds; upper_bounds; _ } ->
          let tyvar = (Reason.Rnone, Tvar id) in
          let lower_bounds = TySet.elements lower_bounds in
          let upper_bounds = TySet.elements upper_bounds in
          let lower_bounds_props =
            List.map ~f:(fun ty -> TL.IsSubtype (ty, tyvar)) lower_bounds
          in
          (* If an upper bound of variable n1 is a `Tvar n2`,
        then we have already added "Tvar n1 <: Tvar n2" when traversing
        lower bounds of n2, so we can filter out upper bounds that are Tvars. *)
          let can_be_removed = function
            | (_, Tvar n) ->
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
            List.map ~f:(fun ty -> TL.IsSubtype (tyvar, ty)) upper_bounds
          in
          TL.conj_list (lower_bounds_props @ upper_bounds_props)
        | GlobalTyvar -> TL.conj_list [])
      tvenv
  in
  let (_ids, props) = List.unzip (IMap.bindings props_per_tvar) in
  TL.conj_list props

and sub_type_inner
    (env : env)
    ~(this_ty : locl_ty option)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Errors.typing_error_callback) : env =
  log_subtype
    ~level:1
    ~this_ty
    ~function_name:"sub_type_inner"
    env
    ty_sub
    ty_super;
  let (env, prop) =
    simplify_subtype
      ~seen_generic_params:empty_seen
      ~no_top_bottom:false
      ~this_ty
      ty_sub
      ty_super
      ~on_error
      env
  in
  let (env, prop) = prop_to_env env prop on_error in
  let env = Env.add_subtype_prop env prop in
  process_simplify_subtype_result prop;
  env

(* BEWARE: hack upon hack here.
 * To implement a predicate that tests whether `ty_sub` is a subtype of
 * `ty_super`, we call sub_type but handle any unification errors and
 * turn them into `false` result. Unfortunately HH_FIXME might end up
 * hiding the "error", and so we need to disable the fixme mechanism
 * before calling sub_type and then re-enable it afterwards.
 *)
and is_sub_type_LEGACY_DEPRECATED
    (env : env) (ty_sub : locl_ty) (ty_super : locl_ty) : bool =
  (* quick short circuit to help perf *)
  ty_equal ty_sub ty_super
  || Errors.try_
       (fun () ->
         ignore_hh_fixmes (fun () ->
             ignore (sub_type env ty_sub ty_super Errors.unify_error);
             true))
       (fun _ -> false)

and is_sub_type_alt ~ignore_generic_params ~no_top_bottom env ty1 ty2 =
  let (_env, prop) =
    simplify_subtype
      ~seen_generic_params:
        ( if ignore_generic_params then
          None
        else
          empty_seen )
      ~no_top_bottom
      ~this_ty:(Some ty1)
      (* It is weird that this can cause errors, but I am wary to discard them.
       * Using the generic unify_error to maintain current behavior. *)
      ty1
      ty2
      ~on_error:Errors.unify_error
      env
  in
  if TL.is_valid prop then
    Some true
  else if TL.is_unsat prop then
    Some false
  else
    None

and is_sub_type env ty1 ty2 =
  is_sub_type_alt ~ignore_generic_params:false ~no_top_bottom:false env ty1 ty2
  = Some true

and is_sub_type_for_union env ty1 ty2 =
  is_sub_type_alt ~ignore_generic_params:false ~no_top_bottom:true env ty1 ty2
  = Some true

and can_sub_type env ty1 ty2 =
  is_sub_type_alt ~ignore_generic_params:false ~no_top_bottom:true env ty1 ty2
  <> Some false

and is_sub_type_ignore_generic_params env ty1 ty2 =
  is_sub_type_alt ~ignore_generic_params:true ~no_top_bottom:true env ty1 ty2
  = Some true

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
  match tyl with
  | [] -> [ty]
  | ty' :: tyl' ->
    if is_sub_type_ignore_generic_params env ty ty' then
      try_intersect env ty tyl'
    else if is_sub_type_ignore_generic_params env ty' ty then
      tyl
    else
      let nonnull_ty = (fst ty, Tnonnull) in
      (match (ty, ty') with
      | ((_, Toption t), _)
        when is_sub_type_ignore_generic_params env ty' nonnull_ty ->
        try_intersect env t (ty' :: tyl')
      | (_, (_, Toption t))
        when is_sub_type_ignore_generic_params env ty nonnull_ty ->
        try_intersect env t (ty :: tyl')
      | (_, _) -> ty' :: try_intersect env ty tyl')

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
 * 2. Do not use Tunion for a syntactic union - the caller can do that.
 * 3. It can be assumed that the original list contains no redundancy.
 * TODO: there are many more unions to implement yet.
 *)
and try_union env ty tyl =
  match tyl with
  | [] -> [ty]
  | ty' :: tyl' ->
    if is_sub_type_for_union env ty ty' then
      tyl
    else if is_sub_type_for_union env ty' ty then
      try_union env ty tyl'
    else (
      match (snd ty, snd ty') with
      | (Tprim Nast.Tfloat, Tprim Nast.Tint)
      | (Tprim Nast.Tint, Tprim Nast.Tfloat) ->
        let t = MakeType.num (fst ty) in
        try_union env t tyl'
      | (_, _) -> ty' :: try_union env ty tyl'
    )

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
    ~(extra_info : reactivity_extra_info)
    (env : env)
    (r_sub : Reason.t)
    (ft_sub : decl_fun_type)
    (r_super : Reason.t)
    (ft_super : decl_fun_type)
    (on_error : Errors.typing_error_callback) : env =
  if (not ft_super.ft_abstract) && ft_sub.ft_abstract then
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See override_abstract_concrete.php test case for example. *)
    Errors.abstract_concrete_override ft_sub.ft_pos ft_super.ft_pos `method_;
  let ety_env = Phase.env_with_self env in
  let (env, ft_super_no_tvars) = Phase.localize_ft ~ety_env env ft_super in
  let (env, ft_sub_no_tvars) = Phase.localize_ft ~ety_env env ft_sub in
  let old_tpenv = Env.get_tpenv env in
  (* We check constraint entailment and contravariant parameter/covariant result
   * subtyping in the context of the ft_super constraints. But we'd better
   * restore tpenv afterwards *)
  let add_tparams_constraints env (tparams : locl_tparam list) =
    let add_bound env { tp_name = (pos, name); tp_constraints = cstrl; _ } =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
          let tparam_ty =
            (Reason.Rwitness pos, Tabstract (AKgeneric name, None))
          in
          Typing_utils.add_constraint pos env ck tparam_ty ty)
    in
    List.fold_left tparams ~f:add_bound ~init:env
  in
  let p_sub = Reason.to_pos r_sub in
  let add_where_constraints env (cstrl : locl_where_constraint list) =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
        Typing_utils.add_constraint p_sub env ck ty1 ty2)
  in
  let env = add_tparams_constraints env (fst ft_super_no_tvars.ft_tparams) in
  let env = add_where_constraints env ft_super_no_tvars.ft_where_constraints in
  let (env, res) =
    simplify_subtype_funs
      ~seen_generic_params:empty_seen
      ~no_top_bottom:false
      ~check_return
      ~extra_info
      r_sub
      ft_sub_no_tvars
      r_super
      ft_super_no_tvars
      on_error
      env
  in
  let (env, res) = prop_to_env env res on_error in
  let env = Env.add_subtype_prop env res in
  process_simplify_subtype_result res;

  (* This is (3) above *)
  let check_tparams_constraints env tparams =
    let check_tparam_constraints
        env { tp_name = (p, name); tp_constraints = cstrl; _ } =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, cstr_ty) ->
          let tgeneric =
            (Reason.Rwitness p, Tabstract (AKgeneric name, None))
          in
          Typing_generic_constraint.check_constraint env ck tgeneric ~cstr_ty)
    in
    List.fold_left tparams ~init:env ~f:check_tparam_constraints
  in
  let check_where_constraints env cstrl =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
        Typing_generic_constraint.check_constraint env ck ty1 ~cstr_ty:ty2)
  in
  (* We only do this if the ft_tparam lengths match. Currently we don't even
   * report this as an error, indeed different names for type parameters.
   * TODO: make it an error to override with wrong number of type parameters
   *)
  let env =
    if
      List.length (fst ft_sub.ft_tparams)
      <> List.length (fst ft_super.ft_tparams)
    then
      env
    else
      check_tparams_constraints env (fst ft_sub_no_tvars.ft_tparams)
  in
  let env = check_where_constraints env ft_sub_no_tvars.ft_where_constraints in
  Env.env_with_tpenv env old_tpenv

let decompose_subtype_add_bound
    (env : env) (ty_sub : locl_ty) (ty_super : locl_ty) : env =
  let (env, ty_super) = Env.expand_type env ty_super in
  let (env, ty_sub) = Env.expand_type env ty_sub in
  match (ty_sub, ty_super) with
  | (_, (_, Tany _)) -> env
  (* name_sub <: ty_super so add an upper bound on name_sub *)
  | ((_, Tabstract (AKgeneric name_sub, _)), _)
    when not (phys_equal ty_sub ty_super) ->
    log_subtype
      ~level:2
      ~this_ty:None
      ~function_name:"decompose_subtype_add_bound"
      env
      ty_sub
      ty_super;
    let tys = Env.get_upper_bounds env name_sub in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_super tys then
      env
    else
      Env.add_upper_bound ~intersect:(try_intersect env) env name_sub ty_super
  (* ty_sub <: name_super so add a lower bound on name_super *)
  | (_, (_, Tabstract (AKgeneric name_super, _)))
    when not (phys_equal ty_sub ty_super) ->
    log_subtype
      ~level:2
      ~this_ty:None
      ~function_name:"decompose_subtype_add_bound"
      env
      ty_sub
      ty_super;
    let tys = Env.get_lower_bounds env name_super in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_sub tys then
      env
    else
      Env.add_lower_bound ~union:(try_union env) env name_super ty_sub
  | (_, _) -> env

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
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Errors.typing_error_callback) : env =
  log_subtype
    ~level:2
    ~this_ty:None
    ~function_name:"decompose_subtype"
    env
    ty_sub
    ty_super;
  let (env, prop) =
    simplify_subtype
      ~seen_generic_params:None
      ~no_top_bottom:false
      ~this_ty:None
      ty_sub
      ty_super
      ~on_error
      env
  in
  decompose_subtype_add_prop p env prop

and decompose_subtype_add_prop p env prop =
  match prop with
  | TL.Conj props ->
    List.fold_left ~f:(decompose_subtype_add_prop p) ~init:env props
  | TL.Disj (_, []) -> Env.mark_inconsistent env
  | TL.Disj (_, [prop']) -> decompose_subtype_add_prop p env prop'
  | TL.Disj _ ->
    Typing_log.log_prop
      2
      env.function_pos
      "decompose_subtype_add_prop"
      env
      prop;
    env
  | TL.IsSubtype (ty1, ty2) -> decompose_subtype_add_bound env ty1 ty2

(* Decompose a general constraint *)
and decompose_constraint
    p
    (env : env)
    (ck : Ast_defs.constraint_kind)
    (ty_sub : locl_ty)
    (ty_super : locl_ty) : env =
  (* constraints are caught based on reason, not error callback. Using unify_error *)
  match ck with
  | Ast_defs.Constraint_as ->
    decompose_subtype p env ty_sub ty_super Errors.unify_error
  | Ast_defs.Constraint_super ->
    decompose_subtype p env ty_super ty_sub Errors.unify_error
  | Ast_defs.Constraint_eq ->
    let env' = decompose_subtype p env ty_sub ty_super Errors.unify_error in
    decompose_subtype p env' ty_super ty_sub Errors.unify_error

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
    (env : env)
    (ck : Ast_defs.constraint_kind)
    (ty_sub : locl_ty)
    (ty_super : locl_ty) : env =
  log_subtype
    ~level:1
    ~this_ty:None
    ~function_name:"add_constraint"
    env
    ty_sub
    ty_super;
  let oldsize = Env.get_tpenv_size env in
  let env' = decompose_constraint p env ck ty_sub ty_super in
  if Env.get_tpenv_size env' = oldsize then
    env'
  else
    let rec iter n env =
      if n > constraint_iteration_limit then
        env
      else
        let oldsize = Env.get_tpenv_size env in
        let env' =
          List.fold_left
            (Env.get_generic_parameters env)
            ~init:env
            ~f:(fun env x ->
              List.fold_left
                (Typing_set.elements (Env.get_lower_bounds env x))
                ~init:env
                ~f:(fun env ty_sub' ->
                  List.fold_left
                    (Typing_set.elements (Env.get_upper_bounds env x))
                    ~init:env
                    ~f:(fun env ty_super' ->
                      decompose_subtype
                        p
                        env
                        ty_sub'
                        ty_super'
                        Errors.unify_error)))
        in
        if Env.get_tpenv_size env' = oldsize then
          env'
        else
          iter (n + 1) env'
    in
    iter 0 env'

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

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.sub_type_ref := sub_type

let () = Typing_utils.add_constraint_ref := add_constraint

let () = Typing_utils.is_sub_type_for_union_ref := is_sub_type_for_union
