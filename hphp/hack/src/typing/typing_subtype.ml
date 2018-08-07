(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Utils
open Typing_defs

module Reason = Typing_reason
module Unify = Typing_unify
module Env = Typing_env
module Subst = Decl_subst
module TUtils = Typing_utils
module SN = Naming_special_names
module Phase = Typing_phase

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
      let m = if is_static then cls.tc_smethods else cls.tc_methods in
      SMap.get method_name m
    | None -> None


  let localize_condition_type (env: Env.env) (ty: decl ty): locl ty =
    (* if condition type is generic - we cannot specify type argument in attribute.
       For cases when we check if containing type is a subtype of condition type
       if condition type is generic instantiate it with TAny's *)
    let ty =
      match try_get_class_for_condition_type env ty with
      | None -> ty
      | Some (_, cls) when cls.tc_tparams = [] -> ty
      | Some (((p, _) as sid), cls) ->
      let params =
        Core_list.map cls.tc_tparams
          ~f:(fun (_, (p, x), _, _) -> Reason.Rwitness p, Tgeneric x) in
      let subst =
        Decl_instantiate.make_subst cls.tc_tparams [] in
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
type simplification_result = {
  constraints: (locl ty * Ast.constraint_kind * locl ty) list;
  failed: (unit -> unit) option;
}

(* Initial input to simplification *)
let initial_result = { constraints = []; failed = None }

(* Subtype assertion is unsatisfiable: extend the error function with `f` *)
let with_error f (env, { constraints; failed }) =
  match failed with
  | None -> (env, { constraints; failed = Some f })
  | Some f' -> (env, { constraints; failed = Some (fun () -> f' (); f () )})

(* If `b` is false then fail with error function `f` *)
let maybe_with_error b f r = if b then with_error f r else r

let check_mutability
  ~(is_receiver: bool)
  (p_sub : Pos.t)
  (mut_sub: param_mutability option)
  (p_super : Pos.t)
  (mut_super: param_mutability option)
  res =
  let str m =
    match m with
    | None -> "immutable"
    | Some Param_mutable -> "mutable"
    | Some Param_maybe_mutable -> "maybe-mutable" in
  match mut_sub, mut_super with
  (* immutable is not compatible with mutable *)
  | None, Some Param_mutable
  (* mutable is not compatible with immutable  *)
  | Some Param_mutable, None
  (* maybe-mutable is not compatible with immutable *)
  | Some Param_maybe_mutable, None
  (* maybe mutable is not compatible with immutable *)
  | Some Param_maybe_mutable, Some Param_mutable ->
    with_error (fun () -> Errors.mutability_mismatch
      ~is_receiver p_sub (str mut_sub) p_super (str mut_super)) res
  | _ ->
    res

let rec process_simplify_subtype_result ~this_ty ~unwrappedToption_super env { constraints; failed } =
  match failed with
  | Some f ->
    f ();
    env
  | None ->
    List.fold_right ~f:(fun (ty1,ck,ty2) env ->
      match ck with
      | Ast.Constraint_eq -> fst (Unify.unify env ty2 ty1)
      | Ast.Constraint_as ->
        sub_type_unwrapped_helper env ~this_ty ~unwrappedToption_super ty1 ty2
      | Ast.Constraint_super ->
        failwith "subtype simplification should not produce super constraints"
      )
      ~init:env constraints

and simplify_subtype
  ?(deep : bool = true)
  ?(this_ty : locl ty option = None)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  (res : Env.env * simplification_result) : Env.env * simplification_result =
  let env, { constraints = acc; failed } = res in
  let types = [
    Typing_log.Log_type ("ty_sub", ty_sub);
    Typing_log.Log_type ("ty_super", ty_super)] in
  let types = Option.value_map this_ty
    ~default:types ~f:(fun ty -> Typing_log.Log_type ("this_ty", ty)::types) in
  Typing_log.log_types 2 (Reason.to_pos (fst ty_sub)) env
    [Typing_log.Log_sub (Printf.sprintf "Typing_subtype.simplify_subtype deep=%b" deep, types)];
  let env, ety_super = Env.expand_type env ty_super in
  let env, ety_sub = Env.expand_type env ty_sub in
  let again env res ty_sub =
    simplify_subtype ~deep ~this_ty ty_sub ty_super (env, res) in
  (* We *know* that the assertion is unsatisfiable *)
  let invalid_with f = with_error f res in
  let invalid () =
    invalid_with (fun () ->
      TUtils.uerror (fst ety_super) (snd ety_super) (fst ety_sub) (snd ety_sub)) in
    (* We *know* that the assertion is valid *)
  let valid () =
    res in
  (* We don't know whether the assertion is valid or not *)
  let default () =
    env, { constraints = (ty_sub,Ast.Constraint_as,ty_super) :: acc;
      failed } in
  match ety_sub, ety_super with
  (* ?ty_sub' <: ?ty_super' iff ty_sub' <: ?ty_super'. Reasoning:
   * If ?ty_sub' <: ?ty_super', then from ty_sub' <: ?ty_super' (widening) and transitivity
   * of <: it follows that ty_sub' <: ?ty_super'.  Conversely, if ty_sub' <: ?ty_super', then
   * by covariance and idempotence of ?, we have ?ty_sub' <: ??ty_sub' <: ?ty_super'.
   * Therefore, this step preserves the set of solutions.
   *)
  | (_, Toption ty_sub'), (_, Toption _) ->
    simplify_subtype ~deep ~this_ty ty_sub' ty_super res

  (* If ty_sub <: ?ty_super' and ty_sub does not contain null then we
   * must also have ty_sub <: ty_super'.  The converse follows by
   * widening and transitivity.  Therefore, this step preserves the set
   * of solutions.
   *)
  | (_,
     (Tprim Nast.(Tint | Tbool | Tfloat | Tstring
                  | Tresource | Tnum | Tarraykey | Tnoreturn)
      | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _
      | Tobject | Tclass _ | Tarraykind _ | Tabstract (AKenum _, _))),
    (_, Toption ty_super') ->
    simplify_subtype ~deep ~this_ty ty_sub ty_super' res

  | (_, Tabstract (AKnewtype (name_sub, _), _)),
    (_, Toption (_, Tabstract (AKnewtype (name_super, _), _) as ty_super'))
    when name_super = name_sub ->
    simplify_subtype ~deep ~this_ty ty_sub ty_super' res

  (* Arrays *)
  | (r, Tarraykind ak_sub), (_, Tarraykind ak_super) ->
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
      let safe_array = TypecheckerOptions.safe_array (Env.get_options env) in
      if safe_array then invalid () else valid ()

    | AKshape fdm, (
        AKvarray _
      | AKvec _
      | AKdarray _
      | AKvarray_or_darray _
      | AKmap _) ->
      Typing_arrays.fold_akshape_as_akmap_with_acc
       begin fun env acc ty ->
         simplify_subtype ~deep ~this_ty ty ty_super (env, acc)
       end env (snd res) r fdm

    | AKtuple fields,
      (
        AKvarray _
      | AKvec _
      | AKdarray _
      | AKvarray_or_darray _
      | AKmap _
      ) ->
      Typing_arrays.fold_aktuple_as_akvec_with_acc
       begin fun env acc ty ->
         simplify_subtype ~deep ~this_ty ty ty_super (env, acc)
       end env (snd res) r fields

    (* varray_or_darray<ty1> <: varray_or_darray<ty2> iff t1 <: ty2
       But, varray_or_darray<ty1> is never a subtype of a vect-like array *)
    | AKvarray_or_darray ty_sub, AKvarray_or_darray ty_super ->
      simplify_subtype ~deep ~this_ty ty_sub ty_super res

    | (AKvarray ty_sub | AKvec ty_sub),
      (AKvarray ty_super | AKvec ty_super | AKvarray_or_darray ty_super) ->
      simplify_subtype ~deep ~this_ty ty_sub ty_super res

    | (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub)),
      (AKdarray (tk_super, tv_super) | AKmap (tk_super, tv_super)) ->
      res |>
      simplify_subtype ~deep ~this_ty tk_sub tk_super |>
      simplify_subtype ~deep ~this_ty tv_sub tv_super

    | (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub)),
      (AKvarray_or_darray tv_super) ->
      let tk_super =
        Reason.Rvarray_or_darray_key (Reason.to_pos (fst ety_super)),
        Tprim Nast.Tarraykey in
      res |>
      simplify_subtype ~deep ~this_ty tk_sub tk_super |>
      simplify_subtype ~deep ~this_ty tv_sub tv_super

    | (AKvarray elt_ty | AKvec elt_ty), (AKdarray _ | AKmap _)
        when not (TypecheckerOptions.safe_vector_array (Env.get_options env)) ->
          let int_reason = Reason.Ridx (Reason.to_pos r, Reason.Rnone) in
          let int_type = int_reason, Tprim Nast.Tint in
          simplify_subtype ~deep ~this_ty
            (r, Tarraykind (AKmap (int_type, elt_ty))) ty_super res

    (* any other array subtyping is unsatisfiable *)
    | _ ->
      invalid ()
    end

  (* ty_sub <: union{ty_super'} iff ty_sub <: ty_super' *)
  | _, (_, Tunresolved [ty_super']) when deep ->
    simplify_subtype ~deep ~this_ty ty_sub ty_super' res

  | (r, Tarraykind akind), (_, Tclass ((_, coll), [tv_super]))
    when (coll = SN.Collections.cTraversable ||
          coll = SN.Rx.cTraversable ||
          coll = SN.Collections.cContainer) ->
      (match akind with
        (* array <: Traversable<t> and emptyarray <: Traversable<t> for any t *)
      | AKany -> valid ()
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
      | AKmap (_, tv) -> simplify_subtype ~deep ~this_ty tv tv_super res
      | AKshape fdm ->
        Typing_arrays.fold_akshape_as_akmap_with_acc again env (snd res) r fdm
      | AKtuple fields ->
        Typing_arrays.fold_aktuple_as_akvec_with_acc again env (snd res) r fields
    )

  | (r, Tarraykind akind), (_, Tclass ((_, coll), [tk_super; tv_super]))
    when (coll = SN.Collections.cKeyedTraversable
         || coll = SN.Rx.cKeyedTraversable
         || coll = SN.Collections.cKeyedContainer
         || coll = SN.Collections.cIndexish) ->
      (match akind with
      | AKany -> valid ()
      | AKempty -> valid ()
      | AKvarray tv
      | AKvec tv ->
        res |>
        simplify_subtype ~deep ~this_ty (r, Tprim Nast.Tint) tk_super |>
        simplify_subtype ~deep ~this_ty tv tv_super
      | AKvarray_or_darray tv ->
        let tk_sub =
          Reason.Rvarray_or_darray_key (Reason.to_pos r),
          Tprim Nast.Tarraykey in
        res |>
        simplify_subtype ~deep ~this_ty tk_sub tk_super |>
        simplify_subtype ~deep ~this_ty tv tv_super
      | AKdarray (tk, tv)
      | AKmap (tk, tv) ->
        res |>
        simplify_subtype ~deep ~this_ty tk tk_super |>
        simplify_subtype ~deep ~this_ty tv tv_super
      | AKshape fdm ->
        Typing_arrays.fold_akshape_as_akmap_with_acc again env (snd res) r fdm
      | AKtuple fields ->
        Typing_arrays.fold_aktuple_as_akvec_with_acc again env (snd res) r fields
      )

  (* (t1,...,tn) <: (u1,...,un) iff t1<:u1, ... , tn <: un *)
  | (_, Ttuple tyl_sub), (_, Ttuple tyl_super)
    when List.length tyl_super = List.length tyl_sub ->
    wfold_left2 (fun res ty_sub ty_super ->
      simplify_subtype ~deep ty_sub ty_super res) res tyl_sub tyl_super

  | (_, Ttuple _), (_, Tarraykind AKany) ->
    if TypecheckerOptions.disallow_array_as_tuple (Env.get_options env)
    then invalid ()
    else valid ()

  | (_, Ttuple _),
    (_, (Tprim _ | Tfun _ | Ttuple _ | Tanon _ | Tobject | Tclass _ |
         Tarraykind _ | Tabstract ((AKnewtype _ | AKenum _), _))) ->
    invalid ()

  | (r_sub, Tfun ft_sub), (r_super, Tfun ft_super) ->
    simplify_subtype_funs ~deep ~check_return:true r_sub ft_sub r_super ft_super res

  | (_, Tabstract (AKnewtype (name_sub, tyl_sub), _)),
    (_, Tabstract (AKnewtype (name_super, tyl_super), _))
    when name_super = name_sub ->
      let td = Env.get_typedef env name_super in
      begin match td with
        | Some {td_tparams; _} ->
          simplify_subtype_variance ~deep name_sub td_tparams tyl_sub tyl_super res
        | None -> default ()
      end

  | (_, Tabstract (AKdependent d_sub, Some ty_sub)),
    (_, Tabstract (AKdependent d_super, Some ty_super))
    when d_sub = d_super ->
    (* Dependent types are identical but bound might be different *)
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    simplify_subtype ~deep ~this_ty ty_sub ty_super res

  (* Abstract type with no bound is only compatible with itself (previous case),
   * with one exception: lower bounds on generic types (hence no AKgeneric here)
   *)
  | (_, Tabstract (AKnewtype _, None)),
    (_, (Tprim _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject |
         Tclass _ | Tarraykind _ | Tabstract ((AKnewtype _ | AKenum _ | AKdependent _), _))) ->
    invalid ()

  (* For abstract type with a bound, use transitivity on bound *)
  | (_, Tabstract (AKnewtype _, Some ty)),
    (_, (Tprim _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject |
         Tclass _ | Tarraykind _ | Tabstract ((AKnewtype _ | AKenum _ | AKdependent _), _))) ->
    simplify_subtype ~deep ~this_ty ty ty_super res

  | (_, Tabstract (AKenum e_sub, _)), (_, Tabstract (AKenum e_super, _))
    when e_sub = e_super -> valid ()

  | (_, Tabstract (AKenum enum_name, _)), (_, Tclass ((_, class_name), _))
  | (_, Tclass ((_, class_name), _)), (_, Tabstract (AKenum enum_name, _))
    when enum_name = class_name -> valid ()

  | (_, Tabstract ((AKenum _), _)), (_, (Tnonnull | Tprim Nast.Tarraykey)) ->
    valid ()

  (* Similar to newtype above *)
  | (_, Tabstract (AKenum _, None)),
    (_, (Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject |
         Tclass _ | Tarraykind _ | Tabstract ((AKnewtype _ | AKenum _ | AKdependent _), _))) ->
    invalid ()

  | (_, Tabstract (AKenum _, Some ty)),
    (_, (Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject |
         Tclass _ | Tarraykind _ | Tabstract ((AKnewtype _ | AKenum _ | AKdependent _), _))) ->
    simplify_subtype ~deep ~this_ty ty ty_super res

  | (r_sub,   Tshape (fields_known_sub, fdm_sub)),
    (r_super, Tshape (fields_known_super, fdm_super)) ->
      (**
       * shape_field_type A <: shape_field_type B iff:
       *   1. A is no more optional than B
       *   2. A's type <: B.type
       *)
      let on_common_field
          res name
          { sft_optional = optional_super; sft_ty = ty_super }
          { sft_optional = optional_sub; sft_ty = ty_sub } =
        match optional_super, optional_sub with
          | true, _ | false, false ->
            simplify_subtype ~deep ~this_ty ty_sub ty_super res
          | false, true ->
            invalid_with (fun () -> Errors.required_field_is_optional
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
        simplify_subtype ~deep ~this_ty
          (r, TUtils.desugar_mixed r) ty_super res in
      TUtils.apply_shape
        ~on_common_field
        ~on_missing_omittable_optional_field
        ~on_missing_non_omittable_optional_field
        ~on_error:(fun _ f -> invalid_with f)
        res
        (r_super, fields_known_super, fdm_super)
        (r_sub, fields_known_sub, fdm_sub)

  | (_, Tshape _),
    (_, (Tprim _ | Tfun _ | Ttuple _ | Tanon _ | Tobject | Tclass _ |
         Tarraykind _ | Tabstract ((AKnewtype _ | AKenum _), _))) ->
    invalid ()

  | (p_sub, (Tclass (x_sub, tyl_sub))), (p_super, (Tclass (x_super, tyl_super))) ->
    let cid_super, cid_sub = (snd x_super), (snd x_sub) in
    (* This is side-effecting as it registers a dependency *)
    let class_def_sub = Env.get_class env cid_sub in
    if cid_super = cid_sub
    then
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
      else if List.is_empty tyl_sub && List.is_empty tyl_super
      then valid ()
      else
        begin match class_def_sub with
        | None ->
          default ()

        | Some class_sub ->
          (* C<t1, .., tn> <: C<u1, .., un> iff
           *   t1 <:v1> u1 /\ ... /\ tn <:vn> un
           * where vi is the variance of the i'th generic parameter of C,
           * and <:v denotes the appropriate direction of subtyping for variance v
           *)
          simplify_subtype_variance ~deep cid_sub
            class_sub.tc_tparams tyl_sub tyl_super res
        end
    else
      begin match class_def_sub with
      | None ->
        default ()

      | Some class_sub ->
        (* We handle the case where a generic A<T> is used as A *)
        let tyl_sub =
          if List.is_empty tyl_sub && not (Env.is_strict env)
          then List.map class_sub.tc_tparams (fun _ -> (p_sub, Tany))
          else tyl_sub in
        if List.length class_sub.tc_tparams <> List.length tyl_sub
        then
          invalid_with (fun () ->
          Errors.expected_tparam
            (Reason.to_pos p_sub) (List.length class_sub.tc_tparams))
        else
          let ety_env =
          (* NOTE: We rely on the fact that we fold all ancestors of
           * ty_sub in its class_type so we will never hit this case
           * again. If this ever changes then we would need to store
           * ty_sub as the 'this_ty' in the uenv and be careful to
           * thread it through.
           *
           * This is covered by test/typecheck/this_tparam2.php
          *)
          {
            type_expansions = [];
            substs = Subst.make class_sub.tc_tparams tyl_sub;
            (* TODO: do we need this? *)
            this_ty = Option.value this_ty ~default:ty_sub;
            from_class = None;
            validate_dty = None;
          } in
            let up_obj = SMap.get cid_super class_sub.tc_ancestors in
            match up_obj with
              | Some up_obj ->
                let env, up_obj = Phase.localize ~ety_env env up_obj in
                simplify_subtype ~deep ~this_ty up_obj ty_super (env, snd res)
              | None ->
                default ()
      end

  (* void is the type of null and is a subtype of any option type. *)
  | (_, Tprim Nast.Tvoid), (_, Toption _)
    when TUtils.is_void_type_of_null env -> valid ()

  | (_, Toption ty_sub'), (_, Tprim Nast.Tvoid)
    when TUtils.is_void_type_of_null env ->
    simplify_subtype ~deep ~this_ty ty_sub' ty_super res

  (* Subtype is known to be nullable, so never a subtype of nonnull *)
  | (_, (Tprim Nast.Tvoid | Tmixed | Tdynamic | Toption _
    | Tabstract (AKdependent _, None))), (_, Tnonnull) ->
    invalid ()

  (* Tvoid is not allowed to subtype Tdynamic *)
  | (_, Tprim Nast.Tvoid), (_, Tdynamic) ->
    if TUtils.is_void_type_of_null env then valid () else invalid ()

  (* everything subtypes mixed *)
  | _, (_, Tmixed) -> valid ()
  | _, (_, Toption (_, Tnonnull)) -> valid ()

  (* mixed doesn't subtype dynamic *)
  | (_, Tmixed), (_, Tdynamic) -> invalid ()
  | (_, Toption (_, Tnonnull)), (_, Tdynamic) -> invalid ()

  (* everything else non-mixed subtypes dynamic *)
  | _, (_, Tdynamic) -> valid ()

  | (_, Tprim (Nast.Tint | Nast.Tfloat)), (_, Tprim Nast.Tnum) -> valid ()
  | (_, Tprim (Nast.Tint | Nast.Tstring)), (_, Tprim Nast.Tarraykey) -> valid ()
  | (_,
     (Tprim Nast.(Tint | Tbool | Tfloat | Tstring
                  | Tresource | Tnum | Tarraykey | Tnoreturn)
      | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _
      | Tobject | Tclass _ | Tarraykind _)),
    (_, Tnonnull) -> valid ()

  (* Any class type is a subtype of object *)
  | (_, Tclass _), (_, Tobject) -> valid ()

  (* Match what's done in unify for non-strict code *)
  | (_, Tobject), (_, Tclass _) when not (Env.is_strict env) -> valid ()

  | (_, Tprim Nast.Tstring), (_, Tclass ((_, stringish), _))
      when stringish = SN.Classes.cStringish -> valid ()
  | (_, Tarraykind _), (_, Tclass ((_, xhp_child), _))
  | (_, Tprim (Nast.Tarraykey | Nast.Tint | Nast.Tfloat | Nast.Tstring | Nast.Tnum)),
    (_, Tclass ((_, xhp_child), _))
      when xhp_child = SN.Classes.cXHPChild -> valid ()
  | (_, Tprim p1), (_, Tprim p2) ->
    if p1 = p2 then valid () else invalid ()
  | (_, Tprim _),
    (_, (Tfun _ | Ttuple _ | Tshape _ | Tanon _  | Tobject | Tclass _ |
         Tarraykind _ | Tabstract ((AKnewtype _ | AKenum _), _))) ->
    invalid ()
  | _, _ ->
    default ()


and simplify_subtype_variance
  ~(deep : bool)
  (cid : string)
  (tparams : decl tparam list)
  (children_tyl : locl ty list)
  (super_tyl : locl ty list)
  : Env.env * simplification_result -> Env.env * simplification_result
  = fun res ->
  match tparams, children_tyl, super_tyl with
  | [], _, _
  | _, [], _
  | _, _, [] -> res
  | (variance,_,_,_) :: tparams, child :: childrenl, super :: superl ->
    let res =
      begin match variance with
      | Ast.Covariant ->
        simplify_subtype ~deep ~this_ty:None child super res
      | Ast.Contravariant ->
        let super = (Reason.Rcontravariant_generic (fst super,
          Utils.strip_ns cid), snd super) in
        simplify_subtype ~deep ~this_ty:None super child res
      | Ast.Invariant ->
        if deep
        then
          res |>
          simplify_subtype ~deep ~this_ty:None child super |>
          simplify_subtype ~deep ~this_ty:None super child
        else
          let env, { constraints; failed } = res in
          let super = (Reason.Rinvariant_generic (fst super,
            Utils.strip_ns cid), snd super) in
          env, { constraints = (child, Ast.Constraint_eq, super) :: constraints;
                 failed }
      end in
    simplify_subtype_variance ~deep cid tparams childrenl superl res

and simplify_subtype_params
  ~deep
  ?(is_method : bool = false)
  ?(check_params_reactivity = false)
  ?(check_params_mutability = false)
  (subl : locl fun_param list)
  (superl : locl fun_param list)
  (variadic_sub_ty : locl ty option)
  (variadic_super_ty : locl ty option)
  res =

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
    | None -> res
    | Some ty -> simplify_supertype_params_with_variadic ~deep superl ty res)
  | _, [] -> (match variadic_sub_ty with
    | None -> res
    | Some ty -> simplify_subtype_params_with_variadic ~deep subl ty res)
  | sub :: subl, super :: superl ->
    let res =
      if check_params_reactivity
      then subtype_fun_params_reactivity sub super res
      else res in
    let res =
      if check_params_mutability
      then check_mutability
        ~is_receiver:false
        sub.fp_pos sub.fp_mutability super.fp_pos super.fp_mutability res
      else res in

    let { fp_type = ty_sub; _ } = sub in
    let { fp_type = ty_super; _ } = super in
    (* Check that the calling conventions of the params are compatible.
     * We don't currently raise an error for reffiness because function
     * hints don't support '&' annotations (enforce_ctpbr = false). *)
    Unify.unify_param_modes ~enforce_ctpbr:is_method sub super;
    Unify.unify_accept_disposable sub super;
    let env, res = res in
    let env = { env with Env.pos = Reason.to_pos (fst ty_sub) } in
    let env, res = match sub.fp_kind, super.fp_kind with
    | FPinout, FPinout ->
      (* Inout parameters are invariant wrt subtyping for function types. *)
      (env, res) |>
      simplify_subtype ~deep ty_super ty_sub |>
      simplify_subtype ~deep ty_sub ty_super
    | _ ->
      simplify_subtype ~deep ty_sub ty_super (env, res) in
    simplify_subtype_params ~deep ~is_method subl superl
      variadic_sub_ty variadic_super_ty (env, res)

and simplify_subtype_params_with_variadic
  ~deep
  (subl : locl fun_param list)
  (variadic_ty : locl ty)
  (env, res) =
  match subl with
  | [] -> (env, res)
  | { fp_type = sub; _ } :: subl ->
    let env = { env with Env.pos = Reason.to_pos (fst sub) } in
    (env, res) |>
    simplify_subtype ~deep sub variadic_ty |>
    simplify_subtype_params_with_variadic ~deep subl variadic_ty

and simplify_supertype_params_with_variadic
  ~deep
  (superl : locl fun_param list)
  (variadic_ty : locl ty)
  (env, res) =
  match superl with
  | [] -> (env, res)
  | { fp_type = super; _ } :: superl ->
    let env = { env with Env.pos = Reason.to_pos (fst super) } in
    (env, res) |>
    simplify_subtype ~deep variadic_ty super |>
    simplify_supertype_params_with_variadic ~deep superl variadic_ty

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

  let parent_class_ty =
    Option.bind extra_info (fun { parent_class_ty = parent_cls; _ } ->
      Option.map parent_cls ~f:maybe_localize) in

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
  (* to compare two maybe reactive values we need to unwrap them *)
  | MaybeReactive sub, MaybeReactive super, _ ->
    subtype_reactivity ?extra_info ~is_call_site env sub super
  (* for explicit checks at callsites implicitly unwrap maybereactive value:
     function f(<<__OnlyRxIfRxFunc>> F $f)
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
  (* anything is a subtype of nonreactive functions *)
  | _, Nonreactive, _ -> true
  | (Local cond_sub | Shallow cond_sub | Reactive cond_sub), Local cond_super, _
  | (Shallow cond_sub | Reactive cond_sub), Shallow cond_super, _
  | Reactive cond_sub, Reactive cond_super, _
    when subtype_param_rx_if_impl env cond_super parent_class_ty cond_sub ->
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
    subtype_param_rx_if_impl env cond_super parent_class_ty cond_sub ->
    true
  (* local can call into non-reactive *)
  | Nonreactive, Local _, _ when is_call_site -> true
  | _ -> false

and should_check_fun_params_reactivity
  (ft_super: locl fun_type) = ft_super.ft_reactive <> Nonreactive

(* checks condition described by OnlyRxIfImpl condition on parameter is met  *)
and subtype_param_rx_if_impl
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
  (* condition type is specified only for subtype - ok
    Consider this case:
    Receivers here are treated as parameters and checked contravariantly
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
  *)
  | Some _, None -> true
  (* condition types are set for both sub and super types *)
  | Some cond_type_sub, Some cond_type_super ->
    is_sub_type env cond_type_sub cond_type_super
  (* condition type is set for super type, check if declared type of
     subtype is a subtype of condition type
     Receivers here are treated as parameters and checked contravariantly
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
  | None, Some cond_type_super ->
    Option.value_map declared_type_sub
      ~default:false
      ~f:(fun declared_type_sub -> is_sub_type env declared_type_sub cond_type_super)

(* checks reactivity conditions for function parameters *)
and subtype_fun_params_reactivity
  (p_sub: locl fun_param)
  (p_super: locl fun_param)
  res =
  match p_sub.fp_rx_condition, p_super.fp_rx_condition with
  (* no conditions on parameters - do nothing *)
  | None, None -> res
  (* both parameters are conditioned to be rx function - no need to check anything *)
  | Some Param_rxfunc, Some Param_rxfunc -> res
  | None, Some Param_rxfunc ->
    (* parameter is conditionally reactive in supertype and missing condition
      in subtype - this is ok only if parameter in subtype is reactive
      <<__Rx>>
      function super((function(): int) $f);
      <<__Rx>>
      function sub(<<__OnlyRxIfRxFunc>> (function(): int) $f);
      We check if sub <: super. parameters are checked contravariantly
      so we need to verify that
      (function(): int) $f <: <<__OnlyRxIfRxFunc>> (function(): int) $f

      Suppose this is legal, then this will be allowed (in pseudo-code)

      function sub(<<__OnlyRxIfRxFunc>> (function(): int) $f) {
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
    let _, p_sub_type = Env.expand_type (fst res) p_sub.fp_type in
    begin match p_sub_type with
    | _, Tfun tfun when tfun.ft_reactive <> Nonreactive -> res
    | _, Tfun _ ->
      with_error (fun () -> Errors.rx_parameter_condition_mismatch
        SN.UserAttributes.uaOnlyRxIfRxFunc p_sub.fp_pos p_super.fp_pos) res
    (* parameter type is not function - error will be reported in different place *)
    | _ -> res
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
      subtype_param_rx_if_impl (fst res) cond_type_sub (Some p_sub.fp_type) cond_type_super in
    maybe_with_error (not ok) (fun () ->
      Errors.rx_parameter_condition_mismatch
        SN.UserAttributes.uaOnlyRxIfImpl p_sub.fp_pos p_super.fp_pos) res

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
  (ft_super : locl fun_type) res =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  let env = fst res in
  res |>
  maybe_with_error
    (not (subtype_reactivity ?extra_info env ft_sub.ft_reactive ft_super.ft_reactive))
    (fun () -> Errors.fun_reactivity_mismatch
      p_super (TUtils.reactivity_to_string env ft_super.ft_reactive)
      p_sub (TUtils.reactivity_to_string env ft_sub.ft_reactive))
  |>
  maybe_with_error
    (ft_sub.ft_is_coroutine <> ft_super.ft_is_coroutine)
    (fun () -> Errors.coroutinness_mismatch ft_super.ft_is_coroutine p_super p_sub) |>
  maybe_with_error
    (ft_sub.ft_return_disposable <> ft_super.ft_return_disposable)
    (fun () -> Errors.return_disposable_mismatch ft_super.ft_return_disposable p_super p_sub) |>
  (* it is ok for subclass to return mutably owned value and treat it as immutable -
  the fact that value is mutably owned guarantees it has only single reference so
  as a result this single reference will be immutable. However if super type
  returns mutable value and subtype yields immutable value - this is not safe.
  NOTE: error is not reported if child is non-reactive since it does not have
  immutability-by-default behavior *)
  maybe_with_error
    (ft_sub.ft_returns_mutable <> ft_super.ft_returns_mutable
    && ft_super.ft_returns_mutable
    && ft_sub.ft_reactive <> Nonreactive)
    (fun () -> Errors.mutable_return_result_mismatch ft_super.ft_returns_mutable p_super p_sub) |>
  maybe_with_error
    (ft_super.ft_reactive <> Nonreactive
    && not ft_super.ft_returns_void_to_rx
    && ft_sub.ft_returns_void_to_rx)
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
  fun res -> (if check_params_mutability
  (* check mutability of receivers *)
  then check_mutability
    ~is_receiver:true
    p_super ft_super.ft_mutability p_sub ft_sub.ft_mutability res else res) |>
  maybe_with_error
    ((arity_min ft_sub.ft_arity) > (arity_min ft_super.ft_arity))
    (fun () -> Errors.fun_too_many_args p_sub p_super) |>
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
  ~deep
  ~(check_return : bool)
  ?(extra_info: reactivity_extra_info option)
  (r_sub : Reason.t)
  (ft_sub : locl fun_type)
  (r_super : Reason.t)
  (ft_super : locl fun_type)
  (acc : Env.env * simplification_result)
   : Env.env * simplification_result =
  (* First apply checks on attributes, coroutine-ness and variadic arity *)
  let acc =
    check_subtype_funs_attributes ?extra_info r_sub ft_sub r_super ft_super acc in

  let check_params_mutability =
    ft_super.ft_reactive <> Nonreactive &&
    ft_sub.ft_reactive <> Nonreactive in

  let variadic_subtype = match ft_sub.ft_arity with
    | Fvariadic (_, {fp_type = var_sub; _ }) -> Some var_sub
    | _ -> None in
  let variadic_supertype =  match ft_super.ft_arity with
    | Fvariadic (_, {fp_type = var_super; _ }) -> Some var_super
    | _ -> None in

  let is_method =
    (Option.map extra_info (fun i -> Option.is_some i.method_info)) = Some true in

  (* Now do contravariant subtyping on parameters *)
  let acc =
    match variadic_subtype, variadic_supertype with
    | Some var_sub, Some var_super -> simplify_subtype ~deep var_super var_sub acc
    | _ -> acc in
  let acc =
    simplify_subtype_params
      ~deep
      ~is_method
      ~check_params_reactivity:(should_check_fun_params_reactivity ft_super)
      ~check_params_mutability
      ft_super.ft_params ft_sub.ft_params variadic_subtype variadic_supertype
      acc in
  (* Finally do covariant subtryping on return type *)
  let acc =
    if check_return
    then simplify_subtype ~deep ft_sub.ft_ret ft_super.ft_ret acc
    else acc in
  acc

(* unwrappedToption_super is true if we have already stripped off a Toption
 * from the supertype. *)
and sub_type
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty) : Env.env =
    sub_type_unwrapped env ~this_ty:None ~unwrappedToption_super:false ty_sub ty_super

(**
 * Checks that ty_sub is a subtype of ty_super, and returns an env.
 *
 * E.g. sub_type env ?int int   => env
 *      sub_type env int alpha  => env where alpha==int
 *      sub_type env ?int alpha => env where alpha==?int
 *      sub_type env int string => error
*)
and sub_type_unwrapped
  (env : Env.env)
  ~(this_ty : locl ty option)
  ~(unwrappedToption_super: bool)
  (ty_sub: locl ty)
  (ty_super: locl ty) : Env.env =
  let types =
    [Typing_log.Log_type ("ty_sub", ty_sub);
     Typing_log.Log_type ("ty_super", ty_super)]  in
  let types = Option.value_map this_ty ~default:types
    ~f:(fun ty -> Typing_log.Log_type ("this_ty", ty) :: types) in
  Typing_log.log_types 2 (Reason.to_pos (fst ty_sub)) env
    [Typing_log.Log_sub (Printf.sprintf
      "Typing_subtype.sub_type_unwrapped unwrappedToption_super=%b"
        unwrappedToption_super,
      types)];
  let env, res =
    simplify_subtype ~deep:false ~this_ty ty_sub ty_super (env, initial_result) in
  process_simplify_subtype_result ~this_ty ~unwrappedToption_super env res

(* Deal with the cases not dealt with by simplify_subtype *)
and sub_type_unwrapped_helper env ~this_ty
  ~unwrappedToption_super ty_sub ty_super =
  let env, ety_super =
    Env.expand_type env ty_super in
  let env, ety_sub =
    Env.expand_type env ty_sub in
  (* Default error *)

  let fail () =
    TUtils.uerror (fst ety_super) (snd ety_super) (fst ety_sub) (snd ety_sub);
    env in

  match ety_sub, ety_super with
  | (_, Terr), _
  | _, (_, Terr) -> env

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
      sub_type_unwrapped_helper env ~this_ty
        ~unwrappedToption_super ty_sub ty_super
   (* This case is for when Tany comes from expanding an empty Tvar - it will
    * result in binding the type variable to the other type. *)
  | _, (_, Tany) -> fst (Unify.unify env ty_super ty_sub)
    (* If the subtype is a type variable bound to an empty unresolved, record
     * this in the todo list to be checked later. But make sure that we wrap
     * any error using the position and reason information that was supplied
     * at the entry point to subtyping.
     *)
  | (_, Tunresolved []), _ ->
    begin match ty_sub with
    | (_, Tvar _) ->
      if not
        (TypecheckerOptions.experimental_feature_enabled
        (Env.get_options env)
        TypecheckerOptions.experimental_unresolved_fix)
      then env
      else
        let outer_pos = env.Env.outer_pos in
        let outer_reason = env.Env.outer_reason in
        Typing_log.log_types 2 outer_pos env
        [Typing_log.Log_sub
          ("Typing_subtype.add_todo",
           [Typing_log.Log_type ("ty_sub", ty_sub);
           Typing_log.Log_type ("ty_super", ty_super)])];
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
        sub_type_unwrapped env ~this_ty
          ~unwrappedToption_super x ty_super
      end ~init:env in
    env

(****************************************************************************)
(* ### End Tunresolved madness ### *)
(****************************************************************************)
  | (_, Tany), _ -> env

  (* This is sort of a hack because our handling of Toption is highly
   * dependent on how the type is structured. When we see a bare
   * dependent type we strip it off at this point since it shouldn't be
   * relevant to subtyping any more.
   *)
  | (_, Tabstract (AKdependent (`expr _, []), Some ty_sub)), _ ->
      let this_ty = Option.first_some this_ty (Some ety_sub) in
      sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty_sub ty_super

  | (_, Tabstract (AKdependent d_sub, Some sub)),
    (_, Tabstract (AKdependent d_super, _)) when d_sub <> d_super ->
      let this_ty = Option.first_some this_ty (Some ety_sub) in
      (* If an error occurred while subtyping, we produce a unification error
       * so we get the full information on how the dependent type was
       * generated
       *)
      Errors.try_when
        (fun () ->
          sub_type_unwrapped env ~this_ty
            ~unwrappedToption_super sub ty_super)
        ~when_: begin fun () ->
          match sub, TUtils.get_base_type env ty_super with
          | (_, Tclass ((_, y), _)), (_, Tclass ((_, x), _)) when x = y -> false
          | _, _ -> true
        end
        ~do_: (fun _ -> TUtils.simplified_uerror env ty_super ty_sub)

  | (r_sub, _),
    (_, Tabstract (AKdependent (expr_dep, _),
      Some (_, Tclass ((_, x) as id, _) as ty_bound))) ->
    let class_ = Env.get_class env x in
    (* For final non-contravariant class C, there is no difference between
     * `this as C<t1,...,tn>` and `C<t1,...,tn>`.
     *)
    begin match class_ with
    | Some class_ty
      when TUtils.class_is_final_and_not_contravariant class_ty ->
      sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty_sub ty_bound
    | _ ->
      (Errors.try_when
         (fun () -> TUtils.simplified_uerror env ty_super ty_sub)
             ~when_: begin fun () ->
               match snd ty_sub with
               | Tclass ((_, y), _) -> y = x
               | Tany | Terr | Tmixed | Tnonnull | Tarraykind _ | Tprim _
               | Toption _ | Tvar _ | Tabstract (_, _) | Ttuple _
               | Tanon (_, _) | Tfun _ | Tunresolved _ | Tobject
               | Tshape _ | Tdynamic -> false
             end
             ~do_: begin fun error ->
               if expr_dep = `cls x then
                 Errors.exact_class_final id (Reason.to_pos r_sub) error
               else
                 Errors.this_final id (Reason.to_pos r_sub) error
             end
          );
          env
    end

  | (p_sub, (Tclass (x_sub, tyl_sub))),
    (_, (Tclass (x_super, _tyl_super))) ->
    let cid_super, cid_sub = (snd x_super), (snd x_sub) in
    if cid_super = cid_sub
    (* Already dealt with this case (variance) in simplify_subtype *)
    then fst (Unify.unify env ety_super ety_sub)
    else
      let class_ = Env.get_class env cid_sub in
      begin match class_ with
        | None -> env
        | Some class_ ->
          (* We handle the case where a generic A<T> is used as A *)
          let tyl_sub =
            if List.is_empty tyl_sub && not (Env.is_strict env)
            then List.map class_.tc_tparams (fun _ -> (p_sub, Tany))
            else tyl_sub
          in
            if List.length class_.tc_tparams <> List.length tyl_sub
            then
              Errors.expected_tparam
                (Reason.to_pos p_sub) (List.length class_.tc_tparams);
          if class_.tc_kind = Ast.Ctrait || class_.tc_kind = Ast.Cinterface then
          (* NOTE: We rely on the fact that we fold all ancestors of
           * ty_sub in its class_type so we will never hit this case
           * again. If this ever changes then we would need to store
           * ty_sub as the 'this_ty' in the uenv and be careful to
           * thread it through.
           *
           * This is covered by test/typecheck/this_tparam2.php
          *)
          let ety_env =
          {
            type_expansions = [];
            substs = Subst.make class_.tc_tparams tyl_sub;
            this_ty = Option.value this_ty ~default:ty_sub;
            from_class = None;
            validate_dty = None;
          } in
          let rec try_reqs reqs =
              match reqs with
              | [] ->
                fail ()

              | (_, req_type) :: reqs ->

              (* a trait is never the runtime type, but it can be used
               * as a constraint if it has requirements for its using
               * classes *)
                Errors.try_ begin fun () ->
                  let env, req_type =
                    Phase.localize ~ety_env env req_type in
                  Typing_log.log_types 2 (Reason.to_pos p_sub) env
                    [Typing_log.Log_sub ("try_reqs",
                     [Typing_log.Log_type ("req_type", req_type)])];
                  sub_type env (p_sub, snd req_type) ty_super
                end (fun _ -> try_reqs reqs)
            in
              try_reqs class_.tc_req_ancestors
          else fail ()
    end

  | _, (_, Toption ty)
    when unwrappedToption_super ->
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty_sub ty

  (* If the nonnull type is not enabled, mixed <: ?t is equivalent
   * to mixed <: t.  Otherwise, we should not encounter mixed
   * because by this time it should have been desugared into ?nonnull.
   *)
  | (_, (Tmixed | Tdynamic)), (_, Toption ty_super) ->
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super:true ty_sub ty_super

  (* If t1 <: ?t2, where t1 is guaranteed not to contain null, then
   * t1 <: t2, and the converse is obviously true as well.
   *)
  | (_, (Tprim Nast.Tvoid | Tabstract (AKdependent _, None))),
    (_, Toption ty_super) ->
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super:true ty_sub ty_super

  | (r_sub, Tabstract (AKnewtype _ as ak, None)),
    (_, Toption _) ->
    let r = Reason.Rimplicit_upper_bound (Reason.to_pos r_sub) in
    let ty = (r, Toption (r, Tnonnull)) in
    let ty_sub' = (r_sub, Tabstract (ak, Some ty)) in
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty_sub' ty_super

  (* If t1 <: ?t2 and t1 is an abstract type constrained as t1',
   * then t1 <: t2 or t1' <: ?t2.  The converse is obviously
   * true as well.  We can fold the case where t1 is unconstrained
   * into the case analysis below.
   *)
  | (_, Tabstract ((AKnewtype _ | AKenum _), Some ty)),
    (_, Toption arg_ty_super) ->
    Errors.try_
      (fun () ->
        sub_type_unwrapped env ~this_ty
          ~unwrappedToption_super:true ty_sub arg_ty_super)
      (fun _ ->
        sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty ty_super)

  | (_, Tabstract (AKdependent _, Some ty)), (_, Toption arg_ty_super) ->
    Errors.try_
      (fun () ->
        sub_type_unwrapped env ~this_ty
          ~unwrappedToption_super:true ty_sub arg_ty_super)
      (fun _ ->
        let this_ty = Option.first_some this_ty (Some ety_sub) in
         sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty ty_super)

  | (_, Tabstract (AKgeneric _, _)), (_, Toption arg_ty_super) ->
    Errors.try_
      (fun () ->
        sub_type_unwrapped env ~this_ty ~unwrappedToption_super:true ty_sub arg_ty_super)
      (fun _ ->
        sub_generic_params SSet.empty env ~this_ty ~unwrappedToption_super ty_sub ty_super)

  | (r_sub, Tvar v), (_, Toption _) ->
    let env, ty_sub = Env.get_type env r_sub v in
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty_sub ty_super

  | (r_sub, Tanon (anon_arity, id)), (r_super, Tfun ft)  ->
      (match Env.get_anonymous env id with
      | None ->
          Errors.anonymous_recursive_call (Reason.to_pos r_sub);
          env
      | Some (reactivity, is_coroutine, ftys, _, anon) ->
          let p_super = Reason.to_pos r_super in
          let p_sub = Reason.to_pos r_sub in
          if not (subtype_reactivity env reactivity ft.ft_reactive)
          then Errors.fun_reactivity_mismatch
            p_super (TUtils.reactivity_to_string env reactivity)
            p_sub (TUtils.reactivity_to_string env ft.ft_reactive);
          if is_coroutine <> ft.ft_is_coroutine
          then Errors.coroutinness_mismatch ft.ft_is_coroutine p_super p_sub;
          if not (Unify.unify_arities
                    ~ellipsis_is_variadic:true anon_arity ft.ft_arity)
          then Errors.fun_arity_mismatch p_super p_sub;
          (* Add function type to set of types seen so far *)
          ftys := TUtils.add_function_type env ety_super !ftys;
          let env, _, ret = anon env ft.ft_params ft.ft_arity in
          let env = sub_type env ret ft.ft_ret in
          env
      )

  (* Supertype is generic parameter *and* subtype is a newtype with bound.
   * We need to make this a special case because there is a *choice*
   * of subtyping rule to apply. See details in the case of dependent type
   * against generic parameter which is similar
   *)
  | (_, Tabstract (AKnewtype (_, _), Some ty)),
    (_, Tabstract (AKgeneric _, _)) ->
    Errors.try_
      (fun () ->
        sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty ty_super)
      (fun _ ->
        sub_generic_params SSet.empty env ~this_ty ~unwrappedToption_super ty_sub ty_super)

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
  | (_, Tabstract (AKdependent _, Some ty)), (_, Tabstract (AKgeneric _, _)) ->
    Errors.try_
      (fun () ->
        let this_ty = Option.first_some this_ty (Some ety_sub) in
        sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty ty_super)
      (fun _ ->
        sub_generic_params SSet.empty env ~this_ty ~unwrappedToption_super ty_sub ty_super)

  | (_, Tabstract (AKdependent _, Some ty)), _ ->
    let this_ty = Option.first_some this_ty (Some ety_sub) in
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty ty_super

  (* Subtype or supertype is generic parameter
   * We delegate these cases to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
   *)
  | (_, Tabstract (AKgeneric _, _)), _
  | _, (_, Tabstract (AKgeneric _, _)) ->
    sub_generic_params SSet.empty env ~this_ty
      ~unwrappedToption_super ty_sub ty_super

  | _, _ ->
    (* TODO: replace this by fail() once we support all subtyping rules that
     * are implemented in unification *)
    fst (Unify.unify env ty_super ty_sub)

and sub_generic_params
  (seen : SSet.t)
  (env : Env.env)
  ~(this_ty : locl ty option)
  ~(unwrappedToption_super : bool)
  (ty_sub: locl ty)
  (ty_super: locl ty) : Env.env =
  let env, ety_super = Env.expand_type env ty_super in
  let env, ety_sub = Env.expand_type env ty_sub in
  let fail _ =
    TUtils.uerror (fst ety_super) (snd ety_super) (fst ety_sub) (snd ety_sub);
    env in
  match ety_sub, ety_super with
  (* If subtype and supertype are the same generic parameter, we're done *)
  | (_, Tabstract (AKgeneric name_sub, _)),
    (_, Tabstract (AKgeneric name_super, _))
       when name_sub = name_super
    -> env

  (* Subtype is generic parameter *)
  | (r_sub, Tabstract (AKgeneric name_sub, opt_sub_cstr)), _ ->
    (* If the generic is actually an expression dependent type,
      we need to update the Unification environment's this_ty
    *)
    let this_ty = if AbstractKind.is_generic_dep_ty name_sub &&
      Option.is_none this_ty then Some ety_sub else this_ty in
    (* If we've seen this type parameter before then we must have gone
     * round a cycle so we fail
     *)
    if SSet.mem name_sub seen
    then fail ()
    else
      let seen = SSet.add name_sub seen in
      (* Otherwise, we collect all the upper bounds ("as" constraints) on
         the generic parameter, and check each of these in turn against
         ty_super until one of them succeeds
       *)
      let rec try_bounds tyl =
        match tyl with
        | [] ->
          (* Try an implicit mixed = ?nonnull bound before giving up.
             This can be useful when checking T <: t, where type t is
             equivalent to but syntactically different from ?nonnull.
             E.g., if t is a generic type parameter T with nonnull as
             a lower bound.
           *)
          let r = Reason.Rimplicit_upper_bound (Reason.to_pos r_sub) in
          let tmixed = (r, Toption (r, Tnonnull)) in
          Errors.try_
            (fun () ->
              sub_generic_params seen env ~this_ty ~unwrappedToption_super tmixed ty_super)
            fail

        | ty::tyl ->
          Errors.try_
            (fun () ->
              sub_generic_params seen env ~this_ty ~unwrappedToption_super ty ty_super)
            (fun l ->
              (* Right now we report constraint failure based on the last
               * error. This should change when we start supporting
               * multiple constraints *)
              if List.is_empty tyl
              then (Reason.explain_generic_constraint
                env.Env.pos r_sub name_sub l; env)
              else try_bounds tyl)
      in try_bounds (Option.to_list opt_sub_cstr @
           Typing_set.elements (Env.get_upper_bounds env name_sub))

  (* Supertype is generic parameter *)
  | _, (r_super, Tabstract (AKgeneric name_super, _)) ->
    (* If we've seen this type parameter before then we must have gone
     * round a cycle so we fail
     *)
    if SSet.mem name_super seen
    then fail ()
    else
      let seen = SSet.add name_super seen in
      (* Collect all the lower bounds ("super" constraints) on the
       * generic parameter, and check ty_sub against each of them in turn
       * until one of them succeeds *)
      let rec try_bounds tyl =
        match tyl with
        | [] ->
          (* Should just fail here. But unification sometimes succeeds.
           * TODO: investigate why *)
          fst (Unify.unify env ty_super ty_sub)

        | ty::tyl ->
          Errors.try_
            (fun () -> sub_generic_params seen ~this_ty env ~unwrappedToption_super ty_sub ty)
          (fun l ->
           (* Right now we report constraint failure based on the last
            * error. This should change when we start supporting
              multiple constraints *)
             if List.is_empty tyl
             then (Reason.explain_generic_constraint
                 env.Env.pos r_super name_super l; env)
             else try_bounds tyl)
      in try_bounds (Typing_set.elements (Env.get_lower_bounds env name_super))

  | _, _ ->
    sub_type_unwrapped env ~this_ty
      ~unwrappedToption_super ty_sub ty_super

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

(* Non-side-effecting test for subtypes, using simplify_subtype.
 * Result is
 *   result = Some true implies ty1 <: ty2
 *   result = Some false implies NOT ty1 <: ty2
 *   result = None, we don't know
 *)
and is_sub_type_alt env ty1 ty2 =
  match simplify_subtype ~deep:true ~this_ty:(Some ty1) ty1 ty2 (env, initial_result) with
  | _, { constraints = []; failed = None } -> Some true
  | _, { constraints = _; failed = Some _ }-> Some false
  | _ -> None

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
  | ty'::tyl' ->
    if is_sub_type_alt env ty ty' = Some true
    then try_intersect env ty tyl'
    else
    if is_sub_type_alt env ty' ty = Some true
    then try_intersect env ty' tyl'
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
let rec try_union env ty tyl =
  match tyl with
  | [] -> [ty]
  | ty'::tyl' ->
    if is_sub_type_alt env ty ty' = Some true
    then try_union env ty' tyl'
    else
    if is_sub_type_alt env ty' ty = Some true
    then try_union env ty tyl'
    else match snd ty, snd ty' with
    | Tprim Nast.Tfloat, Tprim Nast.Tint
    | Tprim Nast.Tint, Tprim Nast.Tfloat ->
      let t = (fst ty, Tprim Nast.Tnum) in
      try_union env t tyl'
    | _, _ -> ty' :: try_union env ty tyl'

let rec sub_string
  (p : Pos.Map.key)
  (env : Env.env)
  (ty2 : locl ty) : Env.env =
  let env, ety2 = Env.expand_type env ty2 in
  let fail () =
    TUtils.uerror (Reason.Rwitness p) (Tprim Nast.Tstring) (fst ety2) (snd ety2);
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
      | _, [] ->
        fail ()
      | env, tyl ->
        List.fold_left tyl ~f:(sub_string p) ~init:env
    end
  | (r2, Tclass (x, _)) ->
      let class_ = Env.get_class env (snd x) in
      (match class_ with
      | None -> env
      | Some tc
          (* A Stringish is a string or an object with a __toString method
           * that will be converted to a string *)
          when tc.tc_name = SN.Classes.cStringish
          || SMap.mem SN.Classes.cStringish tc.tc_ancestors ->
        env
      | Some _ ->
        Errors.object_string p (Reason.to_pos r2);
        env
      )
  | _, (Tany | Terr | Tdynamic) ->
    env (* Tany, Terr and Tdynamic are considered Stringish by default *)
  | _, Tobject -> env
  | _, (Tmixed | Tnonnull | Tarraykind _ | Tvar _
    | Ttuple _ | Tanon (_, _) | Tfun _ | Tshape _) ->
  fail ()

 (* Check that the method with signature ft_sub can be used to override
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
    let add_bound env (_, (pos, name), cstrl, _) =
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
    add_tparams_constraints env ft_super_no_tvars.ft_tparams in
  let env =
    add_where_constraints env ft_super_no_tvars.ft_where_constraints in

  let env, res =
  simplify_subtype_funs
    ~deep:true
    ~check_return
    ~extra_info
    r_sub ft_sub_no_tvars
    r_super ft_super_no_tvars
    (env, initial_result) in
  let env =
    process_simplify_subtype_result ~this_ty:None ~unwrappedToption_super:false env res in

  (* This is (3) above *)
  let check_tparams_constraints env tparams =
  let check_tparam_constraints env (_var, (p, name), cstrl, _) =
    List.fold_left cstrl ~init:env ~f:begin fun env (ck, cstr_ty) ->
      let tgeneric = (Reason.Rwitness p, Tabstract (AKgeneric name, None)) in
      Typing_generic_constraint.check_constraint env ck cstr_ty tgeneric
    end in
  List.fold_left tparams ~init:env ~f:check_tparam_constraints in

  let check_where_constraints env cstrl =
    List.fold_left cstrl ~init:env ~f:begin fun env (ty1, ck, ty2) ->
      Typing_generic_constraint.check_constraint env ck ty2 ty1
    end in

  (* We only do this if the ft_tparam lengths match. Currently we don't even
   * report this as an error, indeed different names for type parameters.
   * TODO: make it an error to override with wrong number of type parameters
  *)
  let env =
    if List.length ft_sub.ft_tparams <> List.length ft_super.ft_tparams
    then env
    else check_tparams_constraints env ft_sub_no_tvars.ft_tparams in
  let env =
    check_where_constraints env ft_sub_no_tvars.ft_where_constraints in

  Env.env_with_tpenv env old_tpenv

let decompose_subtype_add_bound
  p
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  : Env.env =
  let env, ty_super = Env.expand_type env ty_super in
  let env, ty_sub = Env.expand_type env ty_sub in
  match ty_sub, ty_super with
  (* name_sub <: ty_super so add an upper bound on name_sub *)
  | (_, Tabstract (AKgeneric name_sub, _)), _ when ty_sub != ty_super ->
    Typing_log.log_types 2 p env
      [Typing_log.Log_sub ("Typing_subtype.decompose_subtype_add_bound",
       [Typing_log.Log_type ("ty_sub", ty_sub);
        Typing_log.Log_type ("ty_super", ty_super)])];
    let tys = Env.get_upper_bounds env name_sub in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_super tys then env
    else Env.add_upper_bound ~intersect:(try_intersect env) env name_sub ty_super

  (* ty_sub <: name_super so add a lower bound on name_super *)
  | _, (_, Tabstract (AKgeneric name_super, _)) when ty_sub != ty_super ->
    Typing_log.log_types 2 p env
    [Typing_log.Log_sub ("Typing_subtype.decompose_subtype_add_bound",
     [Typing_log.Log_type ("ty_sub", ty_sub);
      Typing_log.Log_type ("ty_super", ty_super)])];
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
 * Although some of this code is similar to that for sub_type_unwrapped, its
 * purpose is different. sub_type_unwrapped takes two types t and u and makes
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
  Typing_log.log_types 2 p env
  [Typing_log.Log_sub ("Typing_subtype.decompose_subtype",
   [Typing_log.Log_type ("ty_sub", ty_sub);
    Typing_log.Log_type ("ty_super", ty_super)])];
  let env, { constraints; _ } =
    simplify_subtype ~deep:true ~this_ty:None ty_sub ty_super (env, initial_result) in
  List.fold_left ~f:(fun env (ty1,ck,ty2) ->
    match ck with
    | Ast.Constraint_as -> decompose_subtype_add_bound p env ty1 ty2
    | Ast.Constraint_super ->
      failwith "subtype simplification should not produce super constraints"
    | Ast.Constraint_eq ->
      let env = decompose_subtype_add_bound p env ty1 ty2 in
      decompose_subtype_add_bound p env ty2 ty1)
    ~init:env constraints

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
 * environment also.
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
  Typing_log.log_types 2 p env
    [Typing_log.Log_sub ("Typing_subtype.add_constraint",
       [Typing_log.Log_type ("ty_sub", ty_sub);
        Typing_log.Log_type ("ty_super", ty_super)])];
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

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.sub_type_ref := sub_type
let () = Typing_utils.add_constraint_ref := add_constraint
let () = Typing_utils.is_sub_type_ref := is_sub_type
