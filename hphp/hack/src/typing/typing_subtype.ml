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

(* Given a pair of types `ty_sub` and `ty_super` attempt to apply simplifications
 * and add to the accumulated constraints in `res` any necessary and
 * sufficient [(t1,ck1,u1);...;(tn,ckn,un)] such that
 *   ty_sub <: ty_super iff t1 ck1 u1, ..., tn ckn un
 * where ck is `as` or `=`. Essentially we are making solution-preserving
 * simplifications to the subtype assertion, for now, also generating equalities
 * as well as subtype assertions, for backwards compatibility with use of
 * unification.
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
 *     we record this in the failed_subtype field of the result.
 *)
type simplification_result = {
  constraints: (locl ty * Ast.constraint_kind * locl ty) list;
  failed_subtype: (locl ty * locl ty) option;
}

let rec simplify_subtype
  ~(deep : bool)
  ~(this_ty : locl ty option)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  (res : Env.env * simplification_result) : Env.env * simplification_result =
  let env, { constraints = acc; failed_subtype } = res in
  let types = [
    Typing_log.Log_type ("ty_sub", ty_sub);
    Typing_log.Log_type ("ty_super", ty_super)] in
  let types = Option.value_map this_ty
    ~default:types ~f:(fun ty -> Typing_log.Log_type ("this_ty", ty)::types) in
  Typing_log.log_types 2 (Reason.to_pos (fst ty_sub)) env
    [Typing_log.Log_sub ("Typing_subtype.simplify_subtype", types)];
  let env, ety_super = Env.expand_type env ty_super in
  let env, ety_sub = Env.expand_type env ty_sub in
  let again env res ty_sub =
    simplify_subtype ~deep ~this_ty ty_sub ty_super (env, res) in
  (* We *know* that the assertion is unsatisfiable *)
  let invalid ()  =
    (env, { constraints = acc; failed_subtype = Some (ety_sub, ety_super) }) in
  (* We *know* that the assertion is valid *)
  let valid () =
    res in
  (* We don't know whether the assertion is valid or not *)
  let default () =
    env, { constraints = (ty_sub,Ast.Constraint_as,ty_super) :: acc;
      failed_subtype } in
  match ety_sub, ety_super with
  (* ?ty_sub' <: ?ty_super' iff ty_sub' <: ?ty_super'. Reasoning:
   * If ?ty_sub' <: ?ty_super', then from ty_sub' <: ?ty_super' (widening) and transitivity
   * of <: it follows that ty_sub' <: ?ty_super'.  Conversely, if ty_sub' <: ?ty_super', then
   * by covariance and idempotence of ?, we have ?ty_sub' <: ??ty_sub' <: ?ty_super'.
   * Therefore, this step preserves the set of solutions.
   *)
  | (_, Toption ty_sub'), (_, Toption _) ->
    simplify_subtype ~deep ~this_ty ty_sub' ty_super res

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

  (* C<ts> <: ?D<ts'> iff C<ts> <: D<ts'> *)
  | (_, Tclass _), (_, Toption ((_, Tclass _) as ty_super')) ->
    simplify_subtype ~deep ~this_ty ty_sub ty_super' res

  (* (t1,...,tn) <: (u1,...,un) iff t1<:u1, ... , tn <: un *)
  | (_, Ttuple tyl_sub), (_, Ttuple tyl_super)
    when List.length tyl_super = List.length tyl_sub ->
    wfold_left2 (fun res ty_sub ty_super ->
      simplify_subtype ~deep ~this_ty:None ty_sub ty_super res) res tyl_sub tyl_super

  | (_, Tabstract (AKnewtype (name_sub, tyl_sub), _)),
    (_, Tabstract (AKnewtype (name_super, tyl_super), _))
    when name_super = name_sub ->
      let td = Env.get_typedef env name_super in
      begin match td with
        | Some {td_tparams; _} ->
          simplify_subtype_variance ~deep name_sub td_tparams tyl_sub tyl_super res
        | None -> default ()
      end

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
            Errors.required_field_is_optional
              (Reason.to_pos r_sub)
              (Reason.to_pos r_super)
              (Env.get_shape_field_name name);
            invalid () in
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
        res
        (r_super, fields_known_super, fdm_super)
        (r_sub, fields_known_sub, fdm_sub)

  | (p_sub, (Tclass (x_sub, tyl_sub))), (_, (Tclass (x_super, tyl_super))) ->
    let cid_super, cid_sub = (snd x_super), (snd x_sub) in
    if cid_super = cid_sub
    then
      if tyl_super = [] || List.length tyl_super <> List.length tyl_sub
      then default ()
      else
        begin match Env.get_class env cid_sub with
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
      begin match Env.get_class env cid_sub with
      | None ->
        default ()

      | Some class_sub ->
        let ety_env =
          (* We handle the case where a generic A<T> is used as A *)
          let tyl_sub =
            if tyl_sub = [] && not (Env.is_strict env)
            then List.map class_sub.tc_tparams (fun _ -> (p_sub, Tany))
            else tyl_sub
          in
          if List.length class_sub.tc_tparams <> List.length tyl_sub
          then
            Errors.expected_tparam
              (Reason.to_pos p_sub) (List.length class_sub.tc_tparams);
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

  (* Subtype is known to be nullable, so never a subtype of nonnull *)
  | (_, (Tprim Nast.Tvoid | Tmixed | Tdynamic | Toption _
    | Tabstract ((AKnewtype _ | AKdependent _), None))), (_, Tnonnull) ->
    invalid ()

  | _, (_, Tmixed) -> valid ()
  | _, (_, Tdynamic) -> valid ()
  | _, (_, Toption (_, Tnonnull)) -> valid ()
  | (_, Tprim (Nast.Tint | Nast.Tfloat)), (_, Tprim Nast.Tnum) -> valid ()
  | (_, Tprim (Nast.Tint | Nast.Tstring)), (_, Tprim Nast.Tarraykey) -> valid ()
  | (_, Tabstract ((AKenum _), _)), (_, Tprim Nast.Tarraykey) -> valid ()
  | (_,
     (Tprim Nast.(Tint | Tbool | Tfloat | Tstring
                  | Tresource | Tnum | Tarraykey | Tnoreturn)
      | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _
      | Tobject | Tclass _ | Tarraykind _ | Tabstract (AKenum _, _))),
    (_, Tnonnull) -> valid ()
  | (_, Tprim Nast.Tstring), (_, Tclass ((_, stringish), _))
      when stringish = SN.Classes.cStringish -> valid ()
  | (_, Tarraykind _), (_, Tclass ((_, xhp_child), _))
  | (_, Tprim (Nast.Tarraykey | Nast.Tint | Nast.Tfloat | Nast.Tstring | Nast.Tnum)),
    (_, Tclass ((_, xhp_child), _))
      when xhp_child = SN.Classes.cXHPChild -> valid ()
  | (_, Tprim p1), (_, Tprim p2) ->
    if p1 = p2 then valid () else invalid ()
  | (_, Tabstract (AKenum e1, _)), (_, Tabstract (AKenum e2, _)) when e1 = e2 ->
    valid ()
  | (_, Tabstract (AKenum _, Some ty)), (_, Tprim _) ->
    simplify_subtype ~deep ~this_ty ty ty_super res
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
  | (variance,_,_) :: tparams, child :: childrenl, super :: superl ->
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
          let env, { constraints; failed_subtype } = res in
          let super = (Reason.Rinvariant_generic (fst super,
            Utils.strip_ns cid), snd super) in
          env, { constraints = (child, Ast.Constraint_eq, super) :: constraints;
                 failed_subtype }
      end in
    simplify_subtype_variance ~deep cid tparams childrenl superl res

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
    else Env.add_upper_bound env name_sub ty_super

  (* ty_sub <: name_super so add a lower bound on name_super *)
  | _, (_, Tabstract (AKgeneric name_super, _)) when ty_sub != ty_super ->
    Typing_log.log_types 2 p env
    [Typing_log.Log_sub ("Typing_subtype.decompose_subtype_add_bound",
     [Typing_log.Log_type ("ty_sub", ty_sub);
      Typing_log.Log_type ("ty_super", ty_super)])];
    let tys = Env.get_lower_bounds env name_super in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_sub tys then env
    else Env.add_lower_bound env name_super ty_sub

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
 * Although some of this code is similar to that for subtype_with_uenv, its
 * purpose is different. subtype_with_uenv takes two types t and u and makes
 * updates to the substitution of type variables (through unification) to
 * make t <: u true.
 *
 * decompose_subtype takes two types t and u for which t <: u is *assumed* to
 * hold, and makes updates to bounds on generic parameters that *necessarily*
 * hold in order for t <: u.
 *
 * If it turns out that there is no situation in which t <: u (for example, we
 * are given string and int, or Cov<Derived> <: Cov<Base>) then evaluate the
 * failure continuation `fail`
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
    simplify_subtype ~deep:true ~this_ty:None ty_sub ty_super
      (env, { constraints = []; failed_subtype = None} ) in
  List.fold_left ~f:(fun env (ty1,ck,ty2) ->
    match ck with
    | Ast.Constraint_as -> decompose_subtype_add_bound p env ty1 ty2
    | Ast.Constraint_super -> decompose_subtype_add_bound p env ty2 ty1
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
 *
 * If the constraint turns out to be unsatisfiable, invoke
 * the failure continuation fail.
*)
let constraint_iteration_limit = 20
let add_constraint_with_fail
  p
  (env : Env.env)
  (ck : Ast.constraint_kind)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  (_fail : Env.env -> Env.env): Env.env =
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

(* Default is to ignore unsatisfiable constraints; in future we might
 * want to produce an error. (For example, if after instantiation a
 * constraint becomes C<string> as C<int>)
 *)
let add_constraint
  (p : Pos.Map.key)
  (env : Env.env)
  (ck : Ast.constraint_kind)
  (ty_sub : locl ty)
  (ty_super : locl ty) : Env.env =
  Typing_log.log_types 2 p env
    [Typing_log.Log_sub ("Typing_subtype.add_constraint",
       [Typing_log.Log_type ("ty_sub", ty_sub);
        Typing_log.Log_type ("ty_super", ty_super)])];
  add_constraint_with_fail p env ck ty_sub ty_super (fun env -> env)

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

let rec subtype_params
  ?(is_method : bool = false)
  (env : Env.env)
  (subl : locl fun_param list)
  (superl : locl fun_param list)
  (variadic_sub_ty : locl ty option)
  (variadic_super_ty : locl ty option) : Env.env =

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
    | None -> env
    | Some ty -> supertype_params_with_variadic env superl ty)
  | _, [] -> (match variadic_sub_ty with
    | None -> env
    | Some ty -> subtype_params_with_variadic env subl ty)
  | sub :: subl, super :: superl ->
    let { fp_type = ty_sub; _ } = sub in
    let { fp_type = ty_super; _ } = super in
    (* Check that the calling conventions of the params are compatible.
     * We don't currently raise an error for reffiness because function
     * hints don't support '&' annotations (enforce_ctpbr = false). *)
    Unify.unify_param_modes ~enforce_ctpbr:is_method sub super;
    Unify.unify_accept_disposable sub super;
    let env = { env with Env.pos = Reason.to_pos (fst ty_sub) } in
    let env = match sub.fp_kind, super.fp_kind with
    | FPinout, FPinout ->
      (* Inout parameters are invariant wrt subtyping for function types. *)
      let env, _ = Unify.unify env ty_super ty_sub in
      env
    | _ ->
      sub_type env ty_sub ty_super in
    let env = subtype_params ~is_method env subl superl variadic_sub_ty variadic_super_ty in
    env

and subtype_params_with_variadic
  (env : Env.env)
  (subl : locl fun_param list)
  (variadic_ty : locl ty) : Env.env =
  match subl with
  | [] -> env
  | { fp_type = sub; _ } :: subl ->
    let env = { env with Env.pos = Reason.to_pos (fst sub) } in
    let env = sub_type env sub variadic_ty in
    subtype_params_with_variadic env subl variadic_ty

and supertype_params_with_variadic
  (env : Env.env)
  (superl : locl fun_param list)
  (variadic_ty : locl ty) : Env.env =
  match superl with
  | [] -> env
  | { fp_type = super; _ } :: superl ->
    let env = { env with Env.pos = Reason.to_pos (fst super) } in
    let env = sub_type env variadic_ty super in
    supertype_params_with_variadic env superl variadic_ty

and subtype_reactivity
  ?(extra_info: reactivity_extra_info option)
  ?(is_call_site = false)
  (env : Env.env)
  (r_sub : reactivity)
  (r_super : reactivity) : bool =
  let ety_env = Phase.env_with_self env in
  let localize ty =
    let _, t = Phase.localize ~ety_env env ty in
    t in
  let localize_condition_type ty =
    (* if condition type is generic - we cannot specify type argument in attribute.
       For cases when we check if containing type is a subtype of condition type
       if condition type is generic instantiate it with TAny's *)
    let ty =
      match TUtils.try_unwrap_class_type ty with
      | None -> ty
      | Some (_, ((p, x) as sid), _) ->
      begin match Env.get_class env x with
      | None -> ty
      | Some cls when cls.tc_tparams = [] -> ty
      | Some cls ->
      let params =
        Core_list.map cls.tc_tparams
          ~f:(fun (_, (p, x), _) -> Reason.Rwitness p, Tgeneric x) in
      let subst =
        Decl_instantiate.make_subst cls.tc_tparams [] in
      let ty = Reason.Rwitness p, (Tapply (sid, params)) in
      Decl_instantiate.instantiate subst ty
    end in
    localize ty in
  let maybe_localize ty =
    match ty with
    | LoclTy t -> t
    | DeclTy t -> localize t in
  (* for methods compute effective reactivity
     with respect to containing classes:
     interface Rx {
       <<__Rx>>
       public function f(): int;
     }
     class A {
       <<__Rx, __OnlyRxIfImpl(Rx::class)>>
       public function f(): int { return 1; }
     }
     class B extends A implements Rx {
     }
     When checking if B properly implements Rx we try to verify
     that A::f that B inherits from A is a subtype of Rx::f.
     A::f is conditionally reactive with condition type Rx and B states
     that it implements Rx so in context of B A::f will be unconditionally reactive
  *)
  let rec effective_reactivity r containing_ty =
    match r, containing_ty with
    | Local (Some t), Some containing_ty
      when is_sub_type env
        (maybe_localize containing_ty)
        (localize_condition_type t) -> Local None
    | Shallow (Some t), Some containing_ty
      when is_sub_type env
        (maybe_localize containing_ty)
        (localize_condition_type t) -> Shallow None
    | Reactive (Some t), Some containing_ty
      when is_sub_type env
        (maybe_localize containing_ty)
        (localize_condition_type t) -> Reactive None
    | MaybeReactive r, _ -> MaybeReactive (effective_reactivity r containing_ty)
    | _ -> r in
  let r_sub =
    Option.bind extra_info (fun { class_ty = c; _ } -> c)
    |> effective_reactivity r_sub in
  let r_super =
    Option.bind extra_info (fun { parent_class_ty = c; _ } -> c)
    |> effective_reactivity r_super in
  (* for method declarations check if condition type for r_super includes
     reactive method with a matching name. If yes - then it will act as a guarantee
     that derived class will have to redefine the method with a shape required
     by condition type (reactivity of redefined method must be subtype of reactivity
     of method in interface) *)
  let condition_type_has_matching_reactive_method condition_type_super (method_name, is_static) =
    let condition_type_opt =
      Option.bind
        (TUtils.try_unwrap_class_type condition_type_super)
        (fun (_, (_, x), _) -> Env.get_class env x) in
    begin match condition_type_opt with
    | None -> false
    | Some cls ->
      let m = if is_static then cls.tc_smethods else cls.tc_methods in
      begin match SMap.get method_name m with
      | Some { ce_type = lazy (_, Typing_defs.Tfun f); _  } ->
        (* check that reactivity of interface method (effectively a promised
           reactivity of a method in derived class) is a subtype of r_super.
           NOTE: we check only for unconditional reactivity since conditional
           version does not seems to yield a lot and will requre implementing
           cycle detection for condition types *)
        begin match f.ft_reactive with
        | Reactive None | Shallow None | Local None ->
          (* adjust r_super since now we are doing the check assuming
             that dervied class implements the condition type interface *)
          let r_super = effective_reactivity r_super (Some (DeclTy condition_type_super)) in
          subtype_reactivity env f.ft_reactive r_super
        | _ -> false
        end
      | _ -> false
      end
    end in
  match r_sub, r_super, extra_info with
  | MaybeReactive sub, MaybeReactive super, _ ->
    subtype_reactivity ?extra_info env sub super
  | _, MaybeReactive _, _ -> true
  (* anything is a subtype of nonreactive functions *)
  | _, Nonreactive, _ -> true
  (* unconditional local/shallow/reactive functions are subtypes of Local *.
      reason: if condition does not hold Local becomes non-reactive which is
      still ok *)
  | (Local None | Shallow None | Reactive None), Local _, _ -> true
  (* unconditional shallow / reactive functions are subtypes of Shallow *.
     Reason: same as above *)
  | (Shallow None | Reactive None), Shallow _, _ -> true
  (* unconditional reactive functions are subtype of reactive *.
     Reason: same as above *)
  | Reactive None, Reactive _, _ -> true
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
  (* conditionally reactive function A is a subtype of conditionally reactive
     function B only if condition type of B is a subtype of condition type of A *)
  | Reactive (Some t), Reactive (Some t1), _
  | (Shallow (Some t) | Reactive (Some t)), Shallow (Some t1), _
  | (Local (Some t) | Shallow (Some t) | Reactive (Some t)), Local (Some t1), _ ->
    is_sub_type env (localize_condition_type t1) (localize_condition_type t)

  (* call_site specific cases *)
  (* shallow can call into local *)
  | Local None, Shallow None, _ when is_call_site -> true
  | Local (Some t), Shallow (Some t1), _ when is_call_site ->
    is_sub_type env (localize_condition_type t1) (localize_condition_type t)
  (* local can call into non-reactive *)
  | Nonreactive, Local _, _ when is_call_site -> true
  | _ -> false


(* This function checks that the method ft_sub can be used to replace
 * (is a subtype of) ft_super.
 *
 * It's used for two purposes:
 * (1) checking that one method can validly override another
 * (2) checking that one function type is a subtype of another
 * For (1) there are a number of features (generics, bounds, Fvariadic)
 * that don't apply in (2)
 * For (2) we apply contravariance on function parameter types, for (1)
 * we treat them invariantly (because of runtime restrictions)
 *
 * The rules must take account of arity,
 * generic parameters and their constraints, parameter types, and return type.
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
 * Note in particular the generali form of the 'where' constraint.
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
and subtype_funs_generic
  ~(check_return : bool)
  ?(extra_info: reactivity_extra_info option)
  (env : Env.env)
  (r_sub : Reason.t)
  (ft_sub : locl fun_type)
  (r_super : Reason.t)
  (ft_super : locl fun_type) : Env.env =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  if not (subtype_reactivity ?extra_info env ft_sub.ft_reactive ft_super.ft_reactive) then
    Errors.fun_reactivity_mismatch
      p_super (TUtils.reactivity_to_string env ft_super.ft_reactive)
      p_sub (TUtils.reactivity_to_string env ft_sub.ft_reactive);
  if ft_sub.ft_is_coroutine <> ft_super.ft_is_coroutine
  then Errors.coroutinness_mismatch ft_super.ft_is_coroutine p_super p_sub;
  if ft_sub.ft_return_disposable <> ft_super.ft_return_disposable
  then Errors.return_disposable_mismatch ft_super.ft_return_disposable p_super p_sub;
  (* it is ok for subclass to return mutably owned value and treat it as immutable -
  the fact that value is mutably owned guarantees it has only single reference so
  as a result this single reference will be immutable. However if super type
  returns mutable value and subtype yields immutable value - this is not safe.
  NOTE: error is not reported if child is non-reactive since it does not have
  immutability-by-default behavior *)
  if (ft_sub.ft_returns_mutable <> ft_super.ft_returns_mutable)
    && ft_super.ft_returns_mutable
    && ft_sub.ft_reactive <> Nonreactive
  then Errors.mutable_return_result_mismatch ft_super.ft_returns_mutable p_super p_sub;
  if ft_super.ft_reactive <> Nonreactive
    && not ft_super.ft_returns_void_to_rx
    && ft_sub.ft_returns_void_to_rx
  then begin
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
      Errors.return_void_to_rx_mismatch ~pos1_has_attribute:true p_sub p_super
  end;
  if (arity_min ft_sub.ft_arity) > (arity_min ft_super.ft_arity)
  then Errors.fun_too_many_args p_sub p_super;
  (match ft_sub.ft_arity, ft_super.ft_arity with
    | Fellipsis _, Fvariadic _ ->
      (* The HHVM runtime ignores "..." entirely, but knows about
       * "...$args"; for contexts for which the runtime enforces method
       * compatibility (currently, inheritance from abstract/interface
       * methods), letting "..." override "...$args" would result in method
       * compatibility errors at runtime. *)
      Errors.fun_variadicity_hh_vs_php56 p_sub p_super;
    | Fstandard (_, sub_max), Fstandard (_, super_max) ->
      if sub_max < super_max
      then Errors.fun_too_few_args p_sub p_super;
    | Fstandard _, _ -> Errors.fun_unexpected_nonvariadic p_sub p_super;
    | _, _ -> ()
  );

  let env  = match ft_sub.ft_arity, ft_super.ft_arity with
    | Fvariadic (_, fp_super), Fvariadic (_, { fp_type = var_sub; _ }) ->
      let {fp_type = var_super; _ } = fp_super in
      sub_type env var_sub var_super
    | _ -> env
  in
  (* This is (1) above *)
  let env =
    let variadic_subtype = match ft_sub.ft_arity with
      | Fvariadic (_, {fp_type = var_sub; _ }) -> Some var_sub
      | _ -> None in
    let variadic_supertype =  match ft_super.ft_arity with
      | Fvariadic (_, {fp_type = var_super; _ }) -> Some var_super
      | _ -> None in
    let is_method =
      (Option.map extra_info (fun i -> i.method_info <> None)) = Some true in
    subtype_params
      ~is_method
      env ft_super.ft_params ft_sub.ft_params variadic_subtype variadic_supertype
  in

  (* We check constraint entailment and invariant parameter/covariant result
   * subtyping in the context of the ft_super constraints. But we'd better
   * restore tpenv afterwards *)
  let old_tpenv = env.Env.lenv.Env.tpenv in
  let add_tparams_constraints env (tparams: locl tparam list) =
    let add_bound env (_, (pos, name), cstrl) =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
        let tparam_ty = (Reason.Rwitness pos,
          Tabstract(AKgeneric name, None)) in
        add_constraint pos env ck tparam_ty ty) in
    List.fold_left tparams ~f:add_bound ~init: env in

  let add_where_constraints env (cstrl: locl where_constraint list) =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
      add_constraint p_sub env ck ty1 ty2) in

  let env =
    add_tparams_constraints env ft_super.ft_tparams in
  let env =
    add_where_constraints env ft_super.ft_where_constraints in

  (* Checking that if the return type was defined in the parent class, it
   * is defined in the subclass too (requested by Gabe Levi).
   *)
  (* We agreed this was too painful for now, breaks too many things *)
  (*  (match ft_super.ft_ret, ft_sub.ft_ret with
      | (_, Tany), _ -> ()
      | (r_super, ty), (r_sub, Tany) ->
      let p_super = Reason.to_pos r_super in
      let p_sub = Reason.to_pos r_sub in
      error_l [p_sub, "Please add a return type";
      p_super, "Because we want to be consistent with this annotation"]
      | _ -> ()
      );
  *)
  (* This is (2) above *)
  let env =
    if check_return
    then sub_type env ft_sub.ft_ret ft_super.ft_ret
    else env in

  (* This is (3) above *)
  let check_tparams_constraints env tparams =
  let check_tparam_constraints env (_var, (p, name), cstrl) =
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
    else check_tparams_constraints env ft_sub.ft_tparams in
  let env =
    check_where_constraints env ft_sub.ft_where_constraints in

  Env.env_with_tpenv env old_tpenv

(* Checking subtyping for methods is different than normal functions. Since
 * methods are declarations we do not want to instantiate their function type
 * parameters as unresolved, instead it should stay as a Tgeneric.
 *)
and subtype_method
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
  subtype_funs_generic
    ~check_return env
    ~extra_info
    r_sub ft_sub_no_tvars
    r_super ft_super_no_tvars

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
  let env, { constraints; failed_subtype } =
    simplify_subtype ~deep:false ~this_ty ty_sub ty_super
      (env, { constraints = []; failed_subtype = None }) in
    match failed_subtype with
    | Some (ty_sub, ty_super) ->
      TUtils.uerror (fst ty_super) (snd ty_super) (fst ty_sub) (snd ty_sub);
      env
    | None ->
      List.fold_right ~f:(fun (ty1,ck,ty2) env ->
        match ck with
        | Ast.Constraint_eq -> fst (Unify.unify env ty2 ty1)
        | Ast.Constraint_as ->
          sub_type_unwrapped_helper env ~this_ty
            ~unwrappedToption_super ty1 ty2
        | Ast.Constraint_super ->
          failwith "subtype simplification should not produce super constraints"
        )
        ~init:env constraints

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
      fst (Unify.unify env ty_super ty_sub)
  | (_, Tunresolved _), (_, Tany) ->
      (* This branch is necessary in the following case:
       * function foo<T as I>(T $x)
       * if I call foo with an intersection type, T is a Tvar,
       * it's expanded version (ety_super in this case) is Tany and what
       * we end up doing is unifying all the elements of the intersection
       * together ...
       * Thanks to this branch, the type variable unifies with the intersection
       * type.
       *)
    fst (Unify.unify env ty_super ty_sub)
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
  (* This case is for when Tany comes from expanding an empty Tvar - it will
   * result in binding the type variable to the other type. *)
  | _, (_, Tany) -> fst (Unify.unify env ty_super ty_sub)
  | (_,   Tabstract (AKdependent d_sub, Some ty_sub)),
    (_, Tabstract (AKdependent d_super, Some ty_super))
        when d_sub = d_super ->
      let this_ty = Option.first_some this_ty (Some ety_sub) in
      sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty_sub ty_super

  (* This is sort of a hack because our handling of Toption is highly
   * dependent on how the type is structured. When we see a bare
   * dependent type we strip it off at this point since it shouldn't be
   * relevant to subtyping any more.
   *)
  | (_, Tabstract (AKdependent (`expr _, []), Some ty_sub)), _ ->
      let this_ty = Option.first_some this_ty (Some ety_sub) in
      sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty_sub ty_super

  | (_, Tabstract (AKdependent d_sub, Some sub)),
    (_,     Tabstract (AKdependent d_super, _)) when d_sub <> d_super ->
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
            if tyl_sub = [] && not (Env.is_strict env)
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

  (* If ?t1 <: ?t2, then from t1 <: ?t1 (widening) and transitivity
   * of <: it follows that t1 <: ?t2.  Conversely, if t1 <: ?t2, then
   * by covariance and idempotence of ?, we have ?t1 <: ??t2 <: ?t2.
   * Therefore, this step preserves the set of solutions.
   *)
  | (_, Toption ty_sub), (_, Toption _) ->
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty_sub ty_super

  (* If the nonnull type is not enabled, mixed <: ?t is equivalent
   * to mixed <: t.  Otherwise, we should not encounter mixed
   * because by this time it should have been desugared into ?nonnull.
   *)
  | (_, (Tmixed | Tdynamic)), (_, Toption ty_super) ->
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super:true ty_sub ty_super

  (* If t1 <: ?t2, where t1 is guaranteed not to contain null, then
   * t1 <: t2, and the converse is obviously true as well.
   *)
  | (_, (Tprim _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ |
         Tobject | Tclass _ | Tarraykind _ |
         Tabstract ((AKdependent _ | AKnewtype _| AKenum _), None))),
    (_, Toption ty_super) ->
    sub_type_unwrapped env ~this_ty ~unwrappedToption_super:true ty_sub ty_super

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

  | (r_sub, Tfun ft_sub), (r_super, Tfun ft_super) ->
    subtype_funs_generic ~check_return:true env r_sub ft_sub r_super ft_super

  | (r_sub, Tanon (anon_arity, id)), (r_super, Tfun ft)  ->
      (match Env.get_anonymous env id with
      | None ->
          Errors.anonymous_recursive_call (Reason.to_pos r_sub);
          env
      | Some (reactivity, is_coroutine, counter, _, anon) ->
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
          counter := !counter + 1;
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
       (fun () -> fst (Unify.unify env ty_super ty_sub))
       (fun _ ->
          Errors.try_
           (fun () ->
             sub_type_unwrapped env ~this_ty ~unwrappedToption_super ty ty_super)
           (fun _ ->
              sub_generic_params SSet.empty env ~this_ty
                ~unwrappedToption_super ty_sub ty_super)
                )

  | (_, Tabstract ((AKnewtype (_, _) | AKenum _), Some ty)), _ ->
    Errors.try_
      (fun () ->
        fst @@ Unify.unify env ty_super ty_sub
      )
      (fun _ ->  sub_type_unwrapped env ~this_ty
        ~unwrappedToption_super ty ty_super)

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
      (fun () -> fst (Unify.unify env ty_super ty_sub))
      (fun _ ->
         Errors.try_
          (fun () ->
            let this_ty = Option.first_some this_ty (Some ety_sub) in
            sub_type_unwrapped env ~this_ty
              ~unwrappedToption_super ty ty_super)
          (fun _ ->
             sub_generic_params SSet.empty env ~this_ty
               ~unwrappedToption_super ty_sub ty_super))

  | (_, Tabstract (AKdependent _, Some ty)), _ ->
      Errors.try_
        (fun () -> fst (Unify.unify env ty_super ty_sub))
        (fun _ ->
          let this_ty = Option.first_some this_ty (Some ety_sub) in
          sub_type_unwrapped env ~this_ty
            ~unwrappedToption_super ty ty_super)

  (* Subtype is generic parameter
   * We delegate this case to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
   *)
  | (_, Tabstract (AKgeneric _, _)), _ ->
    sub_generic_params SSet.empty env ~this_ty
      ~unwrappedToption_super ty_sub ty_super

  (* Supertype is generic parameter
   * We delegate this case to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
  *)
  | _, (_, Tabstract (AKgeneric _, _)) ->
    sub_generic_params SSet.empty env ~this_ty
      ~unwrappedToption_super ty_sub ty_super

  | _, _ ->
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
    then fst (Unify.unify env ty_super ty_sub)
    else
      let seen = SSet.add name_sub seen in
      (* Otherwise, we collect all the upper bounds ("as" constraints) on
         the generic parameter, and check each of these in turn against
         ty_super until one of them succeeds
       *)
      let rec try_bounds tyl =
        match tyl with
        | [] ->
          (* There are no bounds so force an error *)
           fst (Unify.unify env ty_super ty_sub)

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
    then fst (Unify.unify env ty_super ty_sub)
    else
      let seen = SSet.add name_super seen in
      (* Collect all the lower bounds ("super" constraints) on the
       * generic parameter, and check ty_sub against each of them in turn
       * until one of them succeeds *)
      let rec try_bounds tyl =
        match tyl with
        | [] ->
          (* There are no bounds so force an error *)
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
  let f = !Errors.is_hh_fixme in
  Errors.is_hh_fixme := (fun _ _ -> false);
  let result =
    Errors.try_
      (fun () -> ignore(sub_type env ty_sub ty_super); true)
      (fun _ -> false) in
  Errors.is_hh_fixme := f;
  result

and sub_string
  (p : Pos.Map.key)
  (env : Env.env)
  (ty2 : locl ty) : Env.env =
  let env, ety2 = Env.expand_type env ty2 in
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
      | env, [] ->
        fst (Unify.unify env (Reason.Rwitness p, Tprim Nast.Tstring) ty2)
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
      fst (Unify.unify env (Reason.Rwitness p, Tprim Nast.Tstring) ty2)

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.sub_type_ref := sub_type
let () = Typing_utils.add_constraint_ref := add_constraint
